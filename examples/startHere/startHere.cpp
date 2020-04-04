//************************************************************
//
// this is a Application that uses the easyMesh library to create network and
// mobile device can connect with BLE
//
//************************************************************
#include <Arduino.h>

#define PROG_VERSION "0.2"  // Don't change this!

// -------------- OTA ----------------------------
#include <Preferences.h>
Preferences preferences;

#include <IotWebConf.h>

// -- Default SSID of the own Access Point before user update.
const char thingName[] = "ESPCon";

// -- Initial password to connect to the Thing, when it creates an own Access
// Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define CONFIG_PIN 35

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

// -------------- Mesh Network / BLE --------------
#include <painlessMesh.h>

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <deque>

#include "Button2.h"

#define VERSION "1.2.20"

// --------------- Display -------------------------
#include "TTGOTDisplay.h"
#define OLED 1
#define TTGOLED 2
#define NO_DISPLAY 0
#define DISPLAY_MODE TTGOLED

// ----------------- WIFI Mesh Setting -------------------//
// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define LED 2  // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define BLINK_PERIOD 3000   // milliseconds until cycle repeat
#define BLINK_DURATION 200  // milliseconds LED is on for

#define MESH_SSID "FreeHK"
#define MESH_PASSWORD "6q8DS9YQbr"
#define MESH_PORT 5555

Scheduler userScheduler;  // to control your personal task

#if DISPLAY_MODE == OLED
#define BUTTON_A_PIN 0
#define BUTTON_A_PIN -1
#elif DISPLAY_MODE == TTGOLED
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 35
#endif

Button2 buttonA = Button2(BUTTON_A_PIN);
Button2 buttonB = Button2(BUTTON_B_PIN);

#if DISPLAY_MODE == OLED
// Libraries for OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#elif DISPLAY_MODE == TTGOLED
int ledBacklight = 80;  // Initial TFT backlight intensity on a scale of 0 to
                        // 255. Initial value is 80.

TTGOTDisplay ttgo;
TFT_eSPI tft = ttgo.getTFT();

char buff[512];
int vref = 1100;

// Menu
#include "MeshMenu.h"
using namespace Menu;

// Setting PWM properties, do not change this!
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
char* constMEM hexDigit MEMMODE = "0123456789ABCDEF";
char* constMEM hexNr[] MEMMODE = {"0", "x", hexDigit, hexDigit};
char buf1[] = "0x11";

painlessMesh mesh;
// Prototypes
void decodeMessage(String message);
void sendGroupMessage(std::string group_id, std::string message,
                      std::string message_id);
void sendGroupMessage(String group_id, String message, String message_id);
void sendPrivateMessage(uint32_t receiver_id, String message,
                        String message_id);
void sendPrivateMessage(uint32_t receiver_id, std::string message,
                        std::string message_id);
void sendPingMessage(uint32_t receiver_id);
void sendPongMessage(uint32_t receiver_id);
void sendBroadcastMessage(String message, bool includeSelf);
void sendHeartbeat();
void receivedCallback(uint32_t from, String& msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);
void loopTTGOLEDDisplay();

// Handle OTA
void handleRoot();

Task taskSendHeartbeat(TASK_SECOND * 1, TASK_FOREVER,
                       &sendHeartbeat);  // start with a one second interval

int otaCtrl = LOW;  // Initial value for external connected led

result myOtaOn() {
  Serial.println("Setup OTA Enable in myOtaOn");
  otaCtrl = HIGH;

  preferences.begin("mesh-network", false);
  otaCtrl = preferences.putUInt("ota", HIGH);
  preferences.putInt("nodeId", mesh.getNodeId());
  preferences.end();
  delay(500);
  mesh.stop();
  delay(500);
  ESP.restart();

  return proceed;
}

result myOtaOff() {
  Serial.println("Setup OTA Disable in myOtaOff");
  otaCtrl = LOW;

  preferences.begin("mesh-network", false);
  otaCtrl = preferences.putUInt("ota", LOW);
  preferences.end();
  delay(500);
  mesh.stop();
  delay(500);
  ESP.restart();

  return proceed;
}

void toggleOTA() {
  if (otaCtrl == HIGH) {
    myOtaOn();
  } else {
    myOtaOff();
  }
}

TOGGLE(otaCtrl, setOta, "OTA Status: ", toggleOTA,
       (Menu::eventMask)(updateEvent | enterEvent),
       noStyle  //,doExit,enterEvent,noStyle
       ,
       VALUE("Enable", HIGH, doNothing, noEvent),
       VALUE("Disable", LOW, doNothing, noEvent));

MENU(subMenu, "OTA Update", doNothing, noEvent, noStyle, SUBMENU(setOta),
     OP("Enable", myOtaOn, enterEvent), OP("Disable", myOtaOff, enterEvent),
     EXIT("<Back"));

MENU(settingMenu, "Setting", doNothing, noEvent, noStyle, SUBMENU(subMenu),
     OP(VERSION, doNothing, noEvent), EXIT("<Back"));

MENU(mainMenu, "FreeHK - WiFi Mesh", doNothing, noEvent, wrapStyle,
     SUBMENU(setOta),
     FIELD(ledBacklight, "Backlight: ", "", 0, 255, 10, 5, doNothing, noEvent,
           wrapStyle)  // Menu option to set the intensity of the backlight of
                       // the screen.
     ,
     OP("Send Group Message", doNothing, noEvent),
     OP("Send PM Message", doNothing, noEvent),
     OP("Send Ping Message", doNothing, noEvent), SUBMENU(settingMenu),
     EXIT("<Home"));

// define menu colors --------------------------------------------------------
//  {{disabled normal,disabled selected},{enabled normal,enabled selected,
//  enabled editing}}
// monochromatic color table`'/.

const colorDef<uint16_t> colors[6] MEMMODE = {
    {{(uint16_t)Black, (uint16_t)Black},
     {(uint16_t)Black, (uint16_t)DarkerBlue, (uint16_t)DarkerBlue}},  // bgColor
    {{(uint16_t)Gray, (uint16_t)Gray},
     {(uint16_t)White, (uint16_t)White, (uint16_t)White}},  // fgColor
    {{(uint16_t)White, (uint16_t)Black},
     {(uint16_t)Yellow, (uint16_t)Yellow, (uint16_t)Red}},  // valColor
    {{(uint16_t)White, (uint16_t)Black},
     {(uint16_t)White, (uint16_t)Yellow, (uint16_t)Yellow}},  // unitColor
    {{(uint16_t)White, (uint16_t)Gray},
     {(uint16_t)Black, (uint16_t)Blue, (uint16_t)White}},  // cursorColor
    {{(uint16_t)White, (uint16_t)Yellow},
     {(uint16_t)DarkerRed, (uint16_t)White, (uint16_t)White}},  // titleColor
};
serialIn serial(Serial);

// MENU_INPUTS(in,&serial);its single, no need to `chainStream`

// define serial output device
idx_t serialTops[MAX_DEPTH] = {0};
serialOut outSerial(Serial, serialTops);

constMEM panel panels[] MEMMODE = {
    {0, 0, GFX_WIDTH / fontW, GFX_HEIGHT / fontH}};
navNode* navNodes[sizeof(panels) /
                  sizeof(panel)];       // navNodes to store navigation status
panelsList pList(panels, navNodes, 1);  // a list of panels and nodes
idx_t eSpiTops[MAX_DEPTH] = {0};
TFT_eSPIOut eSpiOut(tft, colors, eSpiTops, pList, fontW, fontH + 1);
menuOut* constMEM outputs[] MEMMODE = {&outSerial,
                                       &eSpiOut};  // list of output devices
outputsList out(outputs,
                sizeof(outputs) / sizeof(menuOut*));  // outputs list controller

NAVROOT(nav, mainMenu, MAX_DEPTH, serial, out);

unsigned long idleTimestamp = millis();

// when menu is suspended
result idle(menuOut& o, idleEvent e) {
  if (e == idling) {
    loopTTGOLEDDisplay();
  }
  return proceed;
}
#endif

BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;
BLECharacteristic* pAPINotifyCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
uint8_t apiUnreadValue = 0;

std::deque<String> read_message_queue = {};
std::deque<String> heartbeat_message_queue = {};
std::deque<String> api_respond_queue = {};

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendHeartbeat();  // Prototype

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

bool display_need_update = true;

#if DISPLAY_MODE == OLED

void initOLED();
void loopOLEDDisplay();

void initOLED() {
  // reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false,
                     false)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("");
  display.display();
}

void loopOLEDDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  String out = "Mesh Net - ";
  out += String(VERSION);
  display.println(out);
  display.setCursor(0, 15);
  display.setTextSize(1);

  String node_id = "Node ID - ";
  node_id += mesh.getNodeId();

  display.print(node_id);

  if (read_message_queue.size() > 0) {
    String message = read_message_queue.back();
    display.setCursor(0, 30);
    display.print(message);
  }

  display.display();
}

#elif DISPLAY_MODE == TTGOLED

void loopTTGOLEDDisplay() {
  if (display_need_update) {
    ttgo.loop(mesh.getNodeId(), mesh.getNodeList().size(), String(VERSION),
              read_message_queue.size());
    display_need_update = false;
  }
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot() {
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal()) {
    // -- Captive portal request were already served.
    return;
  }
  String s =
      "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" "
      "content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>HK Free Wifi</title></head><body>Thanks for support WiFi Mesh! "
       "<br/><br/>";
  // s += "Go to <a href='config'>configure page</a> to change
  // settings.<br/><br/>";
  s += "Go to http://192.168.4.1/firmware to apply latest firmware "
       "changes.<br/><br/>";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

#endif

#define SERVICE_UUID \
  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_NOTIFY "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_READ "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_API_NOTIFY "6E400005-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_API_WRITE "6E400006-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_API_RESPOND "6E400007-B5A3-F393-E0A9-E50E24DCCA9E"
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { deviceConnected = true; };

  void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};

void decodeMessage(String message) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message.c_str());
  if (error) {
    Serial.printf("JSON Format decode error, fall back to public message\n\n");
    sendGroupMessage(String("public"), message, String("error"));
    return;
  }

  const char* type = doc["type"];

  if (strcmp("gm", type) == 0) {
    const char* group_id = doc["group_id"];
    const char* to_message = doc["message"];
    const char* message_id = doc["message_id"];

    Serial.printf("Prepare Send Group message : msg= #### %s ####\n",
                  to_message);
    sendGroupMessage(String(group_id), String(to_message), String(message_id));
    Serial.println("Prepare Send Group message done");

  } else if (strcmp("pm", type) == 0) {
    uint32_t receiver_id = doc["receiver_id"];
    const char* to_message = doc["message"];
    const char* message_id = doc["message_id"];

    Serial.printf("Prepare send Private Message : [%u] msg= #### %s  ####\n",
                  receiver_id, to_message);
    sendPrivateMessage(receiver_id, String(to_message), String(message_id));
    Serial.printf("Prepare send Private Message done\n");

  } else if (strcmp("ping", type) == 0) {
    uint32_t receiver_id = doc["receiver_id"];
    Serial.printf("Prepare send Ping Message : [%u] \n", receiver_id);
    sendPingMessage(receiver_id);
    Serial.printf("Prepare send Private Message done\n");

  } else {
    sendGroupMessage(String("public"), message, String("Unknown"));
  }
}

class BLEReadCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {}

  void onRead(BLECharacteristic* pCharacteristic) {
    if (read_message_queue.size() > 0) {
      String message = read_message_queue.front();
      pCharacteristic->setValue(message.c_str());
      read_message_queue.pop_front();
      display_need_update = true;
    } else {
      // if no message in queue
      String message = "{}";
      pCharacteristic->setValue(message.c_str());
    }
  }
};

class BLEWriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      String messageStr = String(rxValue.c_str());
      Serial.println(messageStr);
      if (strcmp("{}", rxValue.c_str()) == 0) {
        Serial.println("empty message, ignore for now, need to fix later");
      } else {
        decodeMessage(messageStr);
      }
    }
  }

  void onRead(BLECharacteristic* pCharacteristic) {}
};

void decodeAPIMessage(String message) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message.c_str());
  if (error) {
    Serial.printf("JSON Format decode error, api call will ignore for now\n\n");
    return;
  }

  const char* type = doc["type"];

  if (strcmp("set_ota_mode", type) == 0) {
    const char* status = doc["status"];

    if (strcmp("true", status) == 0) {
      Serial.printf("Setup OTA mode on\n\n");
      myOtaOn();
    } else if (strcmp("false", status) == 0) {
      Serial.printf("Setup OTA mode off\n\n");
      myOtaOff();
    }
  } else if (strcmp("read_ota_mode", type) == 0) {
    Serial.printf("Read OTA mode\n\n");
    preferences.begin("mesh-network", false);
    int otaCtrlStatus = preferences.getUInt("ota", LOW);
    preferences.end();

    if (otaCtrlStatus == HIGH) {
      doc["result"] = "true";
    } else {
      doc["result"] = "false";
    }
    String str;
    serializeJson(doc, str);
    Serial.printf("Read OTA mode with respond");
    Serial.printf(str.c_str());
    Serial.printf("\n\n");
    api_respond_queue.push_back(str);
  }
}

class BLEAPIWriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      String messageStr = String(rxValue.c_str());
      Serial.println(messageStr);
      if (strcmp("{}", rxValue.c_str()) == 0) {
        Serial.println("empty message, ignore for now, need to fix later");
      }
      decodeAPIMessage(messageStr);
    }
  }

  void onRead(BLECharacteristic* pCharacteristic) {}
};

class BLEAPIRespondCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {}

  void onRead(BLECharacteristic* pCharacteristic) {
    Serial.print("-------BLEAPIRespondCallbacks onRead -------------\n");
    if (api_respond_queue.size() > 0) {
      Serial.print("-------api_respond_queue.size() > 0 -------------\n");
      String message = api_respond_queue.front();
      pCharacteristic->setValue(message.c_str());
      api_respond_queue.pop_front();
    } else {
      Serial.print("-------api_respond_queue.size() == 0 -------------\n");
      // if no message in queue
      String message = "{}";
      pCharacteristic->setValue(message.c_str());
    }
  }
};

void tapHandler(Button2& btn) {
  display_need_update = true;
  switch (btn.getClickType()) {
    case SINGLE_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button A single \n\n");
      //
      nav.doNav(upCmd);
      break;
    case DOUBLE_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button A double \n\n");
      decodeMessage(
          "{\"type\": \"gm\",\"group_id\": \"public\",  \"message\": "
          "\"abcd\", "
          " \"message_id\": \"test\"}");
      decodeMessage(
          "{\"type\": \"pm\",\"receiver_id\": 673516837,  \"message\": "
          "\"abcd\",  \"message_id\": \"test\"}");

      break;
    case TRIPLE_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button A triple \n\n");
      decodeMessage(
          "{\"type\": \"ping\",\"receiver_id\": 673516837,  \"message_id\": "
          "\"test\"}");

      break;
    case LONG_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button A long \n\n");
      unsigned int time = btn.wasPressedFor();
      if (time >= 1000) {
        nav.doNav(enterCmd);
      }
      break;
  }
  Serial.print("click");
  Serial.print(" (");
  Serial.print(btn.getNumberOfClicks());
  Serial.println(")");
}

void tapButtonBHandler(Button2& btn) {
  display_need_update = true;
  switch (btn.getClickType()) {
    case SINGLE_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button B single \n\n");
      nav.doNav(downCmd);
      break;
    case DOUBLE_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button B double \n\n");
      decodeMessage(
          "{\"type\": \"pm\",\"receiver_id\": 673516837,  \"message\": "
          "\"abcd\"}");

      break;
    case TRIPLE_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button B triple \n\n");
      decodeMessage("{\"type\": \"ping\",\"receiver_id\": 673516837}");

      break;
    case LONG_CLICK:
      Serial.print("--------------------\n");
      Serial.print("Button B long \n\n");
      // decodeMessage("a");
      unsigned int time = btn.wasPressedFor();
      if (time >= 1000) {
        nav.doNav(escCmd);
      }
      break;
  }
  Serial.print("click");
  Serial.print(" (");
  Serial.print(btn.getNumberOfClicks());
  Serial.println(")");
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.flush();

  preferences.begin("mesh-network", false);
  otaCtrl = preferences.getUInt("ota", LOW);
  preferences.end();

#if DISPLAY_MODE == OLED
  initOLED();
#elif DISPLAY_MODE == TTGOLED

  buttonB.setClickHandler(tapButtonBHandler);
  buttonB.setLongClickHandler(tapButtonBHandler);
  buttonB.setDoubleClickHandler(tapButtonBHandler);
  buttonB.setTripleClickHandler(tapButtonBHandler);

  Serial.print("Configuring PWM for TFT backlight... ");
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  Serial.println("DONE");

  Serial.print("Setting PWM for TFT backlight to default intensity... ");
  ledcWrite(pwmLedChannelTFT, ledBacklight);
  Serial.println("DONE");

  buttonA.setClickHandler(tapHandler);
  buttonA.setLongClickHandler(tapHandler);
  buttonA.setDoubleClickHandler(tapHandler);
  buttonA.setTripleClickHandler(tapHandler);

  nav.idleTask = idle;     // point a function to be used when menu is suspended
  nav.idleChanged = true;  // If you have a task that needs constant
                           // attention, like drawing a clock on idle screen
                           // you can signal that with idleChanged as true

#endif

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION | SYNC);
  // mesh.setDebugMsgTypes(ERROR | DEBUG);  // set before init() so that you
  // can see error messages

  String node_id = "UART Service - ";

  if (otaCtrl == LOW) {
    mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler,
              MESH_PORT);  //, WIFI_STA);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
    mesh.onNodeDelayReceived(&delayReceivedCallback);

    node_id += mesh.getNodeId();
    String node_id_display = node_id + "\nVersion = " + String(VERSION);
    Serial.println(node_id_display);

    userScheduler.addTask(taskSendHeartbeat);
    taskSendHeartbeat.enable();

    blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
      // If on, switch off, else switch on
      if (onFlag)
        onFlag = false;
      else
        onFlag = true;
      blinkNoNodes.delay(BLINK_DURATION);

      if (blinkNoNodes.isLastIteration()) {
        // Finished blinking. Reset task for next run
        // blink number of nodes (including this node) times
        blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
        // Calculate delay based on current mesh time and BLINK_PERIOD
        // This results in blinks between nodes being synced
        blinkNoNodes.enableDelayed(
            BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
      }
    });

    userScheduler.addTask(blinkNoNodes);
    blinkNoNodes.enable();

    randomSeed(analogRead(A0));

    // Create the BLE Server

    display_need_update = true;
    nav.idleOn();
  } else {
    preferences.begin("mesh-network", false);
    node_id += preferences.getInt("nodeId", 0);
    preferences.end();
    // node_id += mesh.getNodeId();
    Serial.println("Start OTA mode");

    // -- Initializing the configuration.
    iotWebConf.setConfigPin(CONFIG_PIN);
    iotWebConf.setupUpdateServer(&httpUpdater);
    iotWebConf.init();

    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    server.onNotFound([]() { iotWebConf.handleNotFound(); });
  }

  BLEDevice::init(node_id.c_str());

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_NOTIFY, BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic* pReadCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_READ, BLECharacteristic::PROPERTY_READ);

  pReadCharacteristic->setCallbacks(new BLEReadCallbacks());

  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new BLEWriteCallbacks());

  pAPINotifyCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_API_NOTIFY, BLECharacteristic::PROPERTY_NOTIFY);

  BLECharacteristic* pAPIWriteCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_API_WRITE, BLECharacteristic::PROPERTY_WRITE);

  pAPIWriteCharacteristic->setCallbacks(new BLEAPIWriteCallbacks());

  BLECharacteristic* pAPIRespondCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_API_RESPOND, BLECharacteristic::PROPERTY_READ);

  pAPIRespondCharacteristic->setCallbacks(new BLEAPIRespondCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->addServiceUUID(pService->getUUID());
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (otaCtrl == LOW) {
    mesh.update();
  } else {
    iotWebConf.doLoop();
  }

  if (deviceConnected) {
    txValue = read_message_queue.size();
    pTxCharacteristic->setValue(&txValue, 1);
    pTxCharacteristic->notify();

    apiUnreadValue = api_respond_queue.size();
    pAPINotifyCharacteristic->setValue(&apiUnreadValue, 1);
    pAPINotifyCharacteristic->notify();

    delay(10);  // bluetooth stack will go into congestion, if too many
                // packets are sent
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

#if DISPLAY_MODE == OLED
  loopOLEDDisplay();
  buttonA.loop();
#elif DISPLAY_MODE == TTGOLED
  // loopTTGOLEDDisplay();
  buttonA.loop();
  buttonB.loop();

  nav.poll();  // this device only draws when needed
  if (nav.sleepTask) {
    // Serial.println("sleep task");
    loopTTGOLEDDisplay();
  }

  ledcWrite(pwmLedChannelTFT, ledBacklight);
#endif
}

void sendGroupMessage(String group_id, String message, String message_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "gm";
  msg["group_id"] = group_id;
  msg["message_id"] = message_id;
  msg["message"] = message;
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);

  sendBroadcastMessage(str, false);
}

void sendGroupMessage(std::string group_id, std::string message,
                      std::string message_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "gm";
  msg["group_id"] = String(group_id.c_str());
  msg["message"] = String(message.c_str());
  msg["message_id"] = String(message_id.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);

  sendBroadcastMessage(str, false);
}

void sendPrivateMessage(uint32_t receiver_id, String message,
                        String message_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "pm";
  msg["receiver_id"] = receiver_id;
  msg["message"] = message;
  msg["message_id"] = message_id;
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);

  sendBroadcastMessage(str, false);
}

void sendPrivateMessage(uint32_t receiver_id, std::string message,
                        std::string message_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "pm";
  msg["receiver_id"] = receiver_id;
  msg["message"] = String(message.c_str());
  msg["message_id"] = String(message_id.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);

  Serial.printf("Send value %s\n", str.c_str());

  sendBroadcastMessage(str, false);
}

void sendPingMessage(uint32_t receiver_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "ping";
  msg["receiver_id"] = serialized(String(receiver_id));
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);

  sendBroadcastMessage(str, false);
}

void sendPongMessage(uint32_t receiver_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "pong";
  msg["receiver_id"] = receiver_id;
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);

  sendBroadcastMessage(str, false);
}

void sendBroadcastMessage(String body, bool includeSelf = false) {
  String node_id = "";
  node_id += mesh.getNodeId();
  Serial.printf("Sending: [%s] %s\n", node_id.c_str(), body.c_str());
  mesh.sendBroadcast(body);
}

void sendHeartbeat() {
  if (otaCtrl == HIGH) {
    return;
  }

  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();

  msg["type"] = "heartbeat";
  msg["free_memory"] = String(ESP.getFreeHeap());
  msg["nodeId"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  mesh.sendBroadcast(str);

  String node_id = "";
  node_id += mesh.getNodeId();

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  Serial.printf("Sending: [%s] heartbeat - Free Memory %s\n", node_id.c_str(),
                String(ESP.getFreeHeap()).c_str());

  taskSendHeartbeat.setInterval(
      random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
  // taskSendHeartbeat.setInterval( random(TASK_SECOND * 30, TASK_SECOND * 40)
  // );  // heartbeat send between 30 and 40 seconds
}

void receivedCallback(uint32_t from, String& msg) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg.c_str());
  const char* type = doc["type"];

  if (strcmp("heartbeat", type) == 0) {
    Serial.printf("heartbeat message from : %u\n", from);
  } else if (strcmp("gm", type) == 0) {
    read_message_queue.push_back(String(msg.c_str()));
    display_need_update = true;
    Serial.printf("Group message Receive: [%u] msg= #### %s  ####\n", from,
                  msg.c_str());
  } else if (strcmp("pm", type) == 0) {
    if (mesh.getNodeId() == doc["receiver_id"]) {
      read_message_queue.push_back(String(msg.c_str()));
      display_need_update = true;
      Serial.printf("Private Message Receive: [%u] msg= #### %s  ####\n", from,
                    msg.c_str());
    } else {
      Serial.printf(
          "Private Message Receive but not me: from : [%u] to : [%u] msg= "
          "#### "
          "%s  ####\n",
          from, doc["receiver_id"], msg.c_str());
    }
  } else if (strcmp("ping", type) == 0) {
    if (mesh.getNodeId() == doc["receiver_id"]) {
      read_message_queue.push_back(String(msg.c_str()));
      display_need_update = true;
      Serial.printf("Ping Message Receive: [%u] \n", from);
      sendPongMessage(from);
      Serial.printf("Pong Message Send: [%u] \n", from);
    }
  } else if (strcmp("pong", type) == 0) {
    Serial.println("Pong Message Receive - before filter");
    if (mesh.getNodeId() == doc["receiver_id"]) {
      read_message_queue.push_back(String(msg.c_str()));
      display_need_update = true;
      Serial.printf("Pong Message Receive: [%u] \n", from);
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(
      BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> startHere: New Connection, %s\n",
                mesh.subConnectionJson(true).c_str());

  display_need_update = true;
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(
      BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  nodes = mesh.getNodeList();

  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;

  display_need_update = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}
