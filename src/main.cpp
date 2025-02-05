#include <Arduino.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2904.h>
#include <BLEClient.h>
#include <BLEScan.h>

#define LED_PIN 2
#define BUTTON_PIN 1
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "12345678-1234-1234-1234-123456789abc"

BLECharacteristic *pCharacteristic;
BLEClient *pClient;
BLERemoteCharacteristic *pRemoteCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
public:
  void onConnect(BLEServer *pServer)
  {
    Serial.println("Client connected");
    Serial.flush(); // Ensure the message is completely sent
  }

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Client disconnected, restarting ESP32...");
    Serial.flush(); // Ensure the message is completely sent
    delay(1000);    // Wait a moment before restarting
    ESP.restart();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    if (value == "ON")
    {
      digitalWrite(LED_PIN, HIGH);
    }
    else if (value == "OFF")
    {
      digitalWrite(LED_PIN, LOW);
    }
  }
};

void connectToServer()
{
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  BLEScanResults scanResults = pBLEScan->start(10); // Increase scan time to 10 seconds
  for (int i = 0; i < scanResults.getCount(); i++)
  {
    BLEAdvertisedDevice advertisedDevice = scanResults.getDevice(i);
    Serial.print("Found device: ");
    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.getName() == "ESP32_JEREM")
    {
      Serial.println("Found our device! Connecting...");
      pClient = BLEDevice::createClient();
      pClient->connect(&advertisedDevice);
      pRemoteCharacteristic = pClient->getService(BLEUUID(SERVICE_UUID))->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
      if (pRemoteCharacteristic != nullptr)
      {
        Serial.println("Connected to server and found characteristic.");
      }
      else
      {
        Serial.println("Failed to find characteristic.");
      }
      break;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  BLEDevice::init("ESP32_JEFFREY");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);
  BLE2904 *p2904Descriptor = new BLE2904();
  p2904Descriptor->setFormat(BLE2904::FORMAT_UTF8);
  pCharacteristic->addDescriptor(p2904Descriptor);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("Waiting for a client connection to notify...");

  connectToServer();
}

void loop()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    digitalWrite(LED_PIN, HIGH);
    if (pRemoteCharacteristic != nullptr)
    {
      pRemoteCharacteristic->writeValue("ON");
    }
    delay(1000); // Debounce delay
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
    if (pRemoteCharacteristic != nullptr)
    {
      pRemoteCharacteristic->writeValue("OFF");
    }
    delay(100); // Debounce delay
  }
}