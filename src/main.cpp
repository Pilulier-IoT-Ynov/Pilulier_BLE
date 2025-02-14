#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0xF0, 0x9E, 0x9E, 0x3B, 0x35, 0x38};

// Structure exemple pour envoyer des données
// Doit correspondre à la structure du récepteur
typedef struct struct_message
{
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Créer une struct_message appelée myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback lorsque les données sont envoyées
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nStatut du dernier paquet envoyé:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Succès de la livraison" : "Échec de la livraison");
}

void setup()
{
  // Initialiser le moniteur série
  Serial.begin(115200);

  // Configurer l'appareil en tant que station Wi-Fi
  WiFi.mode(WIFI_STA);

  // Initialiser ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Erreur d'initialisation d'ESP-NOW");
    return;
  }

  // Une fois ESPNow initialisé avec succès, nous enregistrerons pour Send CB
  // pour obtenir le statut du paquet transmis
  esp_now_register_send_cb(OnDataSent);

  // Enregistrer le pair
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Ajouter le pair
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Échec de l'ajout du pair");
    return;
  }
}

void loop()
{
  Serial.println(WiFi.macAddress());

  // Définir les valeurs à envoyer
  strcpy(myData.a, "C'EST UN CHAR");
  myData.b = random(1, 20);
  myData.c = 1.2;
  myData.d = false;

  // Envoyer le message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK)
  {
    Serial.println("Envoyé avec succès");
  }
  else
  {
    Serial.println("Erreur lors de l'envoi des données");
  }
  delay(2000);
}

// #include <Arduino.h>
// #include "esp_bt_main.h"
// #include "esp_bt_device.h"
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>è
// #include <BLE2904.h>
// #include <BLEClient.h>
// #include <BLEScan.h>

// #define LED_PIN 2
// #define BUTTON_PIN 1
// #define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
// #define CHARACTERISTIC_UUID "12345678-1234-1234-1234-123456789abc"

// BLECharacteristic *pCharacteristic;
// BLEClient *pClient;
// BLERemoteCharacteristic *pRemoteCharacteristic;

// class MyServerCallbacks : public BLEServerCallbacks
// {
// public:
//   void onConnect(BLEServer *pServer)
//   {
//     Serial.println("Client connected");
//     Serial.flush(); // Ensure the message is completely sent
//   }

//   void onDisconnect(BLEServer *pServer)
//   {
//     Serial.println("Client disconnected, restarting ESP32...");
//     Serial.flush(); // Ensure the message is completely sent
//     delay(1000);    // Wait a moment before restarting
//     ESP.restart();
//   }
// };

// class MyCallbacks : public BLECharacteristicCallbacks
// {
//   void onWrite(BLECharacteristic *pCharacteristic)
//   {
//     std::string value = pCharacteristic->getValue();
//     if (value == "ON")
//     {
//       digitalWrite(LED_PIN, HIGH);
//     }
//     else if (value == "OFF")
//     {
//       digitalWrite(LED_PIN, LOW);
//     }
//   }
// };

// void connectToServer()
// {
//   BLEScan *pBLEScan = BLEDevice::getScan();
//   pBLEScan->setActiveScan(true);
//   BLEScanResults scanResults = pBLEScan->start(10); // Increase scan time to 10 seconds
//   for (int i = 0; i < scanResults.getCount(); i++)
//   {
//     BLEAdvertisedDevice advertisedDevice = scanResults.getDevice(i);
//     Serial.print("Found device: ");
//     Serial.println(advertisedDevice.toString().c_str());
//     if (advertisedDevice.getName() == "ESP32_JEREM")
//     {
//       Serial.println("Found our device! Connecting...");
//       pClient = BLEDevice::createClient();
//       pClient->connect(&advertisedDevice);
//       pRemoteCharacteristic = pClient->getService(BLEUUID(SERVICE_UUID))->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
//       if (pRemoteCharacteristic != nullptr)
//       {
//         Serial.println("Connected to server and found characteristic.");
//       }
//       else
//       {
//         Serial.println("Failed to find characteristic.");
//       }
//       break;
//     }
//   }
// }

// void setup()
// {
//   Serial.begin(115200);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BUTTON_PIN, INPUT_PULLUP);
//   BLEDevice::init("ESP32_JEFFREY");
//   BLEServer *pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());
//   BLEService *pService = pServer->createService(SERVICE_UUID);
//   pCharacteristic = pService->createCharacteristic(
//       CHARACTERISTIC_UUID,
//       BLECharacteristic::PROPERTY_READ |
//           BLECharacteristic::PROPERTY_WRITE |
//           BLECharacteristic::PROPERTY_NOTIFY);
//   BLE2904 *p2904Descriptor = new BLE2904();
//   p2904Descriptor->setFormat(BLE2904::FORMAT_UTF8);
//   pCharacteristic->addDescriptor(p2904Descriptor);
//   pCharacteristic->setCallbacks(new MyCallbacks());
//   pService->start();
//   BLEAdvertising *pAdvertising = pServer->getAdvertising();
//   pAdvertising->start();
//   Serial.println("Waiting for a client connection to notify...");

//   connectToServer();
// }

// void loop()
// {
//   if (digitalRead(BUTTON_PIN) == LOW)
//   {
//     digitalWrite(LED_PIN, HIGH);
//     if (pRemoteCharacteristic != nullptr)
//     {
//       pRemoteCharacteristic->writeValue("ON");
//     }
//     delay(1000); // Debounce delay
//   }
//   else
//   {
//     digitalWrite(LED_PIN, LOW);
//     if (pRemoteCharacteristic != nullptr)
//     {
//       pRemoteCharacteristic->writeValue("OFF");
//     }
//     delay(100); // Debounce delay
//   }
// }