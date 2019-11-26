//************************************************************
// this is a simple example that uses the easyMesh library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD
// 3. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 4. prints anything it receives to Serial.print
//
//
//************************************************************
#include <Arduino.h>

#define PROG_VERSION "0.2" // Don't change this!

#include <painlessMesh.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <deque>

#include "Button2.h"

#define   VERSION       "1.1.11"

// ----------------- WIFI Mesh Setting -------------------//
// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define   LED             2       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  200  // milliseconds LED is on for

#define   MESH_SSID       "FreeHK"
#define   MESH_PASSWORD   "6q8DS9YQbr"
#define   MESH_PORT       5555



//------------------ define display Mode -----------------//
#define OLED 1
#define TTGOLED 2
#define NO_DISPLAY 0
#define DISPLAY_MODE TTGOLED

//------------------ define display Mode end ------------//


#if DISPLAY_MODE == OLED
  //Libraries for OLED Display
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  #define OLED_SDA 4
  #define OLED_SCL 15 
  #define OLED_RST 16
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels

  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

  #define BUTTON_A_PIN  0
  #define BUTTON_A_PIN  -1

#elif DISPLAY_MODE == TTGOLED
  #include <TFT_eSPI.h>
  #include <SPI.h>
  #include "WiFi.h"
  #include <Wire.h>
  #include "esp_adc_cal.h"
  //#include "bmp.h"

  #ifndef TFT_DISPOFF
  #define TFT_DISPOFF 0x28
  #endif

  #ifndef TFT_SLPIN
  #define TFT_SLPIN   0x10
  #endif

  #define TFT_MOSI            19
  #define TFT_SCLK            18
  #define TFT_CS              5
  #define TFT_DC              16
  #define TFT_RST             23 

  #define TFT_BL          4  // Display backlight control pin
  #define ADC_EN          14
  #define ADC_PIN         34
  #define BUTTON_1        35
  #define BUTTON_2        0

  #define BUTTON_A_PIN  0
  #define BUTTON_B_PIN  35
  
  int ledBacklight = 80; // Initial TFT backlight intensity on a scale of 0 to 255. Initial value is 80.

  TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

  char buff[512];
  int vref = 1100;

  // Menu
  #include <menu.h>
  #include <menuIO/serialIO.h>
  #include <menuIO/TFT_eSPIOut.h>
  #include <menuIO/esp8266Out.h>//must include this even if not doing web output...
  using namespace Menu;

  // Setting PWM properties, do not change this!
  const int pwmFreq = 5000;
  const int pwmResolution = 8;
  const int pwmLedChannelTFT = 0;
  char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
  char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
  char buf1[]="0x11";

  painlessMesh  mesh;

  //customizing a prompt look!
  //by extending the prompt class
  // class altPrompt:public prompt {
  // public:
  //   altPrompt(constMEM promptShadow& p):prompt(p) {}
  //   Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t) override {
  //     return out.printRaw(F("special prompt!"),len);;
  //   }
  // };

  MENU(mainMenu,"FreeHK - WiFi Mesh",doNothing,noEvent,wrapStyle
    ,FIELD(ledBacklight,"Backlight: ","",0,255,10,5,doNothing,noEvent,wrapStyle) // Menu option to set the intensity of the backlight of the screen.
    ,OP("Send Group Message",doNothing,noEvent)
    ,OP("Send PM Message",doNothing,noEvent)
    ,OP("Send Ping Message",doNothing,noEvent)
    ,OP("Settings",doNothing,noEvent)
    ,EXIT("<Home")
  );

  // define menu colors --------------------------------------------------------
  //  {{disabled normal,disabled selected},{enabled normal,enabled selected, enabled editing}}
  //monochromatic color table`'/.


  #define Black RGB565(0,0,0)
  #define Red	RGB565(255,0,0)
  #define Green RGB565(0,255,0)
  #define Blue RGB565(0,0,255)
  #define Gray RGB565(128,128,128)
  #define LighterRed RGB565(255,150,150)
  #define LighterGreen RGB565(150,255,150)
  #define LighterBlue RGB565(150,150,255)
  #define DarkerRed RGB565(150,0,0)
  #define DarkerGreen RGB565(0,150,0)
  #define DarkerBlue RGB565(0,0,150)
  #define Cyan RGB565(0,255,255)
  #define Magenta RGB565(255,0,255)
  #define Yellow RGB565(255,255,0)
  #define White RGB565(255,255,255)

  const colorDef<uint16_t> colors[6] MEMMODE={
    {
      {
        (uint16_t)Black,
        (uint16_t)Black
      },
      {
        (uint16_t)Black,
        (uint16_t)DarkerBlue,
        (uint16_t)DarkerBlue
      }
    },//bgColor
    {
      {
        (uint16_t)Gray,
        (uint16_t)Gray
      },
      {
        (uint16_t)White,
        (uint16_t)White,
        (uint16_t)White
      }
    },//fgColor
    {
      {
        (uint16_t)White,
        (uint16_t)Black
      },
      {
        (uint16_t)Yellow,
        (uint16_t)Yellow,
        (uint16_t)Red
      }
    },//valColor
    {
      {
        (uint16_t)White,
        (uint16_t)Black
      },
      {
        (uint16_t)White,
        (uint16_t)Yellow,
        (uint16_t)Yellow
      }
    },//unitColor
    {
      {
        (uint16_t)White,
        (uint16_t)Gray
      },
      {
        (uint16_t)Black,
        (uint16_t)Blue,
        (uint16_t)White
      }
    },//cursorColor
    {
      {
        (uint16_t)White,
        (uint16_t)Yellow
      },
      {
        (uint16_t)DarkerRed,
        (uint16_t)White,
        (uint16_t)White
      }
    },//titleColor
  };

  #define MAX_DEPTH 5

  serialIn serial(Serial);

  // MENU_INPUTS(in,&serial);its single, no need to `chainStream`

  // define serial output device
  idx_t serialTops[MAX_DEPTH]={0};
  serialOut outSerial(Serial,serialTops);


  #define GFX_WIDTH 240
  #define GFX_HEIGHT 135
  #define fontW 12
  #define fontH 18

  constMEM panel panels[] MEMMODE = {{0, 0, GFX_WIDTH / fontW, GFX_HEIGHT / fontH}};
  navNode* navNodes[sizeof(panels) / sizeof(panel)]; //navNodes to store navigation status
  panelsList pList(panels, navNodes, 1); //a list of panels and nodes
  idx_t eSpiTops[MAX_DEPTH]={0};
  TFT_eSPIOut eSpiOut(tft,colors,eSpiTops,pList,fontW,fontH+1);
  menuOut* constMEM outputs[] MEMMODE={&outSerial,&eSpiOut};//list of output devices
  outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller

  NAVROOT(nav,mainMenu,MAX_DEPTH,serial,out);

  unsigned long idleTimestamp = millis();

  //when menu is suspended
  result idle(menuOut& o,idleEvent e) {
    if (e==idling) {
        // Show the idle message once
        int xpos = tft.width() / 2; // Half the screen width
        tft.fillScreen(Black);

        tft.setTextSize(5);
        tft.setTextColor(Yellow,Black);
        tft.setTextWrap(false);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("IDLE", xpos, 50);
        int getFontHeight = tft.fontHeight();

        tft.setTextSize(2);
        tft.setTextColor(White,Black);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Long press a button", xpos, 90);
        tft.drawString("to exit", xpos, 110);
    }
    return proceed;
  }

#endif

Button2 buttonA = Button2(BUTTON_A_PIN);


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

std::deque<String> read_message_queue = {};
std::deque<String> heartbeat_message_queue = {};

// https://gitlab.com/painlessMesh/painlessMesh




// Prototypes
void decodeMessage(String message);
void sendGroupMessage(std::string group_id, std::string message);
void sendGroupMessage(String group_id, String message);
void sendPrivateMessage(uint32_t receiver_id, String message);
void sendPrivateMessage(uint32_t receiver_id, std::string message);
void sendPingMessage(uint32_t receiver_id);
void sendPongMessage(uint32_t receiver_id);
void sendBroadcastMessage(String message, bool includeSelf);
void sendHeartbeat(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler     userScheduler; // to control your personal task


bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendHeartbeat() ; // Prototype
Task taskSendHeartbeat( TASK_SECOND * 1, TASK_FOREVER, &sendHeartbeat ); // start with a one second interval

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

bool display_need_update = true;

#if DISPLAY_MODE == OLED

void initOLED();
void loopOLEDDisplay();

void initOLED() {
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("");
  display.display();
}

void loopOLEDDisplay() {
  display.clearDisplay();
  display.setCursor(0,0);
  String out = "Mesh Net - ";
  out += String(VERSION);
  display.println(out);
  display.setCursor(0,15);
  display.setTextSize(1);

  String node_id = "Node ID - ";
  node_id += mesh.getNodeId();

  display.print(node_id);

  if (read_message_queue.size() > 0) {
    String message = read_message_queue.back();
    display.setCursor(0,30);
    display.print(message);

  }
  // display.print("Counter:");
  // display.setCursor(50,30);
  // display.print(counter);      
  display.display();
}


#elif DISPLAY_MODE == TTGOLED
void initTTGOLED();
void loopTTGOLEDDisplay();

void initTTGOLED() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
}

void loopTTGOLEDDisplay() {

  if (display_need_update) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    String out = "WiFi Mesh v";
    out += String(VERSION);
    tft.drawString(out,  tft.width() / 2, 10 );

    String node_id = "Node ID - ";
    node_id += mesh.getNodeId();
    node_id += " ( ";
    node_id += mesh.getNodeList().size();
    node_id += " connected )";

    tft.drawString(node_id,  tft.width() / 2, 25 );  
    display_need_update = false;

    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED);

    if (read_message_queue.size() > 0) {
      String unread_message = "";
      unread_message += String(read_message_queue.size());
      unread_message += " Unread Messages";

      tft.drawString(unread_message, tft.width() / 2, 100 );  
    }

  }
}

Button2 buttonB = Button2(BUTTON_B_PIN);

#endif

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_NOTIFY "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_READ "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void decodeMessage(String message) {

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message.c_str());
  if (error)
  {
    Serial.printf("JSON Format decode error, fall back to public message\n\n");
    sendGroupMessage(String("public"), message);
    return;
  }

  const char* type = doc["type"];

  if (strcmp ("gm", type) == 0) {
    const char* group_id = doc["group_id"];
    const char* to_message = doc["message"];
    
    Serial.printf("Prepare Send Group message : msg= #### %s ####\n", to_message);
    sendGroupMessage(String(group_id), String(to_message));
    Serial.printf("Prepare Send Group message done");

  } else if (strcmp ("pm", type) == 0) {
    
    uint32_t receiver_id = doc["receiver_id"];
    const char* to_message = doc["message"];
    Serial.printf("Prepare send Private Message : [%u] msg= #### %s  ####\n", receiver_id, to_message);
    sendPrivateMessage(receiver_id, String(to_message));
    Serial.printf("Prepare send Private Message done\n");
    
  } else if (strcmp ("ping", type) == 0) {
    
    uint32_t receiver_id = doc["receiver_id"];
    Serial.printf("Prepare send Ping Message : [%u] \n", receiver_id);
    sendPingMessage(receiver_id);
    Serial.printf("Prepare send Private Message done\n");
    
  }
  else {
    sendGroupMessage(String("public"), message);
  }
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {

        String messageStr = String(rxValue.c_str());
        decodeMessage(messageStr);
      }
    }

    void onRead(BLECharacteristic *pCharacteristic) {
      if (read_message_queue.size() > 0) {
        String message = read_message_queue.front();
        pCharacteristic->setValue(message.c_str());
        read_message_queue.pop_front();
      }
      else {
        // if no message in queue
        String message = "{}";
        pCharacteristic->setValue(message.c_str());
      }
    }
};

void tapHandler(Button2& btn) {
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
            decodeMessage("{\"type\": \"gm\",\"group_id\": \"public\",  \"message\": \"abcd\"}");
            decodeMessage("{\"type\": \"pm\",\"receiver_id\": 673516837,  \"message\": \"abcd\"}");

            break;
        case TRIPLE_CLICK:
            Serial.print("--------------------\n");
            Serial.print("Button A triple \n\n");
            decodeMessage("{\"type\": \"ping\",\"receiver_id\": 673516837}");

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
    switch (btn.getClickType()) {
        case SINGLE_CLICK:
            Serial.print("--------------------\n");
            Serial.print("Button B single \n\n");
            // decodeMessage("{\"type\": \"gm\",\"group_id\": \"public\",  \"message\": \"abcd\"}");
            nav.doNav(downCmd);
            break;
        case DOUBLE_CLICK:
        Serial.print("--------------------\n");
            Serial.print("Button B double \n\n");
            decodeMessage("{\"type\": \"pm\",\"receiver_id\": 673516837,  \"message\": \"abcd\"}");

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

  buttonA.setClickHandler(tapHandler);
  buttonA.setLongClickHandler(tapHandler);
  buttonA.setDoubleClickHandler(tapHandler);
  buttonA.setTripleClickHandler(tapHandler);

  // pinMode(LED, OUTPUT);

  #if DISPLAY_MODE == OLED
  initOLED();
  #elif DISPLAY_MODE == TTGOLED
  initTTGOLED();

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
  
  nav.idleTask=idle;//point a function to be used when menu is suspended
  // mainMenu[1].disable(); // can disable one of the menu item

  #endif

  mesh.setDebugMsgTypes( ERROR | DEBUG| CONNECTION | SYNC );
  // mesh.setDebugMsgTypes(ERROR | DEBUG);  // set before init() so that you can see error messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  String node_id = "UART Service - ";
  node_id += mesh.getNodeId();
  node_id += "\nVersion = " + String(VERSION);
  Serial.println(node_id);

  BLEDevice::init(node_id.c_str());

  userScheduler.addTask( taskSendHeartbeat );
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
        blinkNoNodes.enableDelayed(BLINK_PERIOD - 
            (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
      }
  });

  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_NOTIFY,
										BLECharacteristic::PROPERTY_NOTIFY
									);
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pReadCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_READ,
											BLECharacteristic::PROPERTY_READ
										);

  pReadCharacteristic->setCallbacks(new MyCallbacks());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->addServiceUUID(pService->getUUID());
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  mesh.update();
  digitalWrite(LED, !onFlag);

  buttonA.loop();

  if (deviceConnected) {
        txValue = read_message_queue.size();
        pTxCharacteristic->setValue(&txValue, 1);
        pTxCharacteristic->notify();
		delay(10); // bluetooth stack will go into congestion, if too many packets are sent
	}

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
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

    nav.poll();//this device only draws when needed

    ledcWrite(pwmLedChannelTFT, ledBacklight);
  #endif
}

void sendGroupMessage(String group_id, String message) {
  
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "gm";
  msg["group_id"] = group_id;
  msg["message"] = message;
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendGroupMessage(std::string group_id, std::string message) {
  
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "gm";
  msg["group_id"] = String(group_id.c_str());
  msg["message"] = String(message.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendPrivateMessage(uint32_t receiver_id, String message) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "pm";
  msg["receiver_id"] = receiver_id;
  msg["message"] = message;
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendPrivateMessage(uint32_t receiver_id, std::string message) {
  
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "pm";
  msg["receiver_id"] = receiver_id;
  msg["message"] = String(message.c_str());
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

  Serial.printf("Sending: [%s] heartbeat - Free Memory %s\n", node_id.c_str(), String(ESP.getFreeHeap()).c_str());
  
  taskSendHeartbeat.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
  // taskSendHeartbeat.setInterval( random(TASK_SECOND * 30, TASK_SECOND * 40) );  // heartbeat send between 30 and 40 seconds
}

void receivedCallback(uint32_t from, String & msg) {

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg.c_str());
  const char* type = doc["type"];

  if (strcmp ("heartbeat", type) == 0) {
    Serial.printf("heartbeat message from : %u\n", from);
  } else if (strcmp ("gm", type) == 0) {
    read_message_queue.push_back( String(msg.c_str()) );
    display_need_update = true;
    Serial.printf("Group message Receive: [%u] msg= #### %s  ####\n", from, msg.c_str());
  } else if (strcmp ("pm", type) == 0) {

    if (mesh.getNodeId() == doc["receiver_id"]) {
      read_message_queue.push_back( String(msg.c_str()) );
      display_need_update = true;
      Serial.printf("Private Message Receive: [%u] msg= #### %s  ####\n", from, msg.c_str());
    }
    else {
      Serial.printf("Private Message Receive but not me: from : [%u] to : [%u] msg= #### %s  ####\n", from, doc["receiver_id"],  msg.c_str());
    }
  } else if (strcmp ("ping", type) == 0) {

    if (mesh.getNodeId() == doc["receiver_id"]) {
      read_message_queue.push_back( String(msg.c_str()) );
      display_need_update = true;
      Serial.printf("Ping Message Receive: [%u] \n", from);
      sendPongMessage(from);
      Serial.printf("Pong Message Send: [%u] \n", from);
    }
  } else if (strcmp ("pong", type) == 0) {
    Serial.printf("Pong Message Receive - before filter");
    if (mesh.getNodeId() == doc["receiver_id"]) {
      read_message_queue.push_back( String(msg.c_str()) );
      display_need_update = true;
      Serial.printf("Pong Message Receive: [%u] \n", from);
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());

  display_need_update = true;
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
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
