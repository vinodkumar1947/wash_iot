/*
   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// the number of the LED pin
const int ledPin = 2;  // 16 corresponds to GPIO16
int LED_BUILTIN = 2;
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

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

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
        if (rxValue == "1")
        {
          Serial.println("Am in 1");
          for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
            // changing the LED brightness with PWM
            ledcWrite(ledChannel, dutyCycle);
            delay(15);
          }
          for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
            // changing the LED brightness with PWM
            ledcWrite(ledChannel, dutyCycle);
            delay(15);
          }
        }
        else if (rxValue == "2")
        {
          Serial.println("Am in 2");
          digitalWrite(LED_BUILTIN, HIGH);
          digitalWrite(23, HIGH);
          delay(2000);
          digitalWrite(23, HIGH);
        }
        else if (rxValue == "3") {

          for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
            // changing the LED brightness with PWM
            ledcWrite(ledChannel, dutyCycle);
            delay(15);
          }
          for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
            // changing the LED brightness with PWM
            ledcWrite(ledChannel, dutyCycle);
            delay(15);
          }
        }
        else
        {
          Serial.println("No match");
          digitalWrite(LED_BUILTIN, LOW);
        }
      }
    }
};
void setup() {
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode (23, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);

  // Create the BLE Device
  BLEDevice::init("UART Service");

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
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

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
}
