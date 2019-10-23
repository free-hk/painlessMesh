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
#include <painlessMesh.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// https://gitlab.com/painlessMesh/painlessMesh

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define   LED             2       // GPIO number of connected LED, ON ESP-12 IS GPIO2

#define   BLINK_PERIOD    3000 // milliseconds until cycle repeat
#define   BLINK_DURATION  200  // milliseconds LED is on for

#define   MESH_SSID       "FreeHK"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Prototypes
void decodeMessage(String message);
void sendGroupMessage(std::string group_id, std::string message);
void sendGroupMessage(String group_id, String message);
void sendPrivateMessage(std::string receiver_id, std::string message);
void sendPingMessage(std::string receiver_id);
void sendPongMessage(std::string receiver_id);
void sendBroadcastMessage(String message, bool includeSelf);
void sendHeartbeat(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;
// SimpleBLE ble;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendHeartbeat() ; // Prototype
Task taskSendHeartbeat( TASK_SECOND * 1, TASK_FOREVER, &sendHeartbeat ); // start with a one second interval

// Task to blink the number of nodes
Task blinkNoNodes;
bool onFlag = false;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void decodeMessage(String message) {
  sendGroupMessage(String("public"), message);
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {

        String messageStr = String(rxValue.c_str());
        decodeMessage(messageStr);
        // sendGroupMessage(rxValue, false);

        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};


void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  mesh.setDebugMsgTypes(ERROR | DEBUG);  // set before init() so that you can see error messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  String node_id = "UART Service - ";
  node_id += mesh.getNodeId();
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
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

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

void onButton(){
    String out = "Group Message : ";
    out += String(millis() / 1000);
    Serial.println(out);
    
    sendGroupMessage(String("public"), out);
}

void loop() {
  mesh.update();
  digitalWrite(LED, !onFlag);

  if (deviceConnected) {
        pTxCharacteristic->setValue(&txValue, 1);
        pTxCharacteristic->notify();
        txValue++;
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

  // setup button event
  static uint8_t lastPinState = 1;
  uint8_t pinState = digitalRead(0);
  if(!pinState && lastPinState){
      onButton();
  }
  lastPinState = pinState;
}

void sendGroupMessage(String group_id, String message) {
  
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "group_message";
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
  
  msg["type"] = "group_message";
  msg["group_id"] = String(group_id.c_str());
  msg["message"] = String(message.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendPrivateMessage(std::string receiver_id, std::string message) {
  
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "private_message";
  msg["receiver_id"] = String(receiver_id.c_str());
  msg["message"] = String(message.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendPingMessage(std::string receiver_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "ping";
  msg["receiver_id"] = String(receiver_id.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendPongMessage(std::string receiver_id) {
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  
  msg["type"] = "pong";
  msg["receiver_id"] = String(receiver_id.c_str());
  msg["source_id"] = mesh.getNodeId();

  String str;
  serializeJson(msg, str);
  
  sendBroadcastMessage(str, false);
}

void sendBroadcastMessage(String body, bool includeSelf = false) {
  
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
  
  // taskSendHeartbeat.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
  taskSendHeartbeat.setInterval( random(TASK_SECOND * 30, TASK_SECOND * 40) );  // heartbeat send between 30 and 40 seconds
}

void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("Receive: [%u] msg= #### %s  ####\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000))/1000);
 
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
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
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}
