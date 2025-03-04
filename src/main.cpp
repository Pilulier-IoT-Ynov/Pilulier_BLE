#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <time.h>

#define SERVICE_UUID "12345678-1234-1234-1234-123456789012"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-210987654321"
#define BUTTON_PIN 1

bool receivedBluetooth;
bool isalreadysend;
bool buttonPressed = false;

std::string valuefromBluetooth;

typedef struct struct_message
{
  char c[32];
  char j[128];
} struct_message;

typedef struct struct_message2
{
  char r[32];
} struct_message2;

// Créer une struct_message appelée myData
struct_message myData;
struct_message2 incomingMessage;

uint8_t broadcastAddress[] = {0xF0, 0x9E, 0x9E, 0x3B, 0x38, 0x7C};

// Structure exemple pour envoyer des données
// Doit correspondre à la structure du récepteur

esp_now_peer_info_t peerInfo;

class MyCallbacks : public BLECharacteristicCallbacks, public BLEServerCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string valuefromBluetooth = pCharacteristic->getValue();
    if (valuefromBluetooth.length() > 0)
    {
      Serial.println("Received Value: ");
      for (int i = 0; i < valuefromBluetooth.length(); i++)
        Serial.print(valuefromBluetooth[i]);
      Serial.println();
      JsonDocument doc;
      deserializeJson(doc, valuefromBluetooth);
      String currentday = doc["currentDay"];
      Serial.println(currentday);

      strncpy(myData.c, "json", 32);
      strncpy(myData.j, valuefromBluetooth.c_str(), 128);
      isalreadysend = false;
      receivedBluetooth = true;
    }
  }

  void onConnect(BLEServer *pServer)
  {
    Serial.println("Client connected");
  }

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Client disconnected");
    // Restart advertising after client disconnects
    BLEDevice::getAdvertising()->start();
    Serial.println("Advertising restarted");
  }
};

void setTimeFromPillulier(int yr, int month, int mday, int wday, int hr, int minute, int sec, int isDst)
{
  struct tm tm;
  Serial.printf("Paramètrage de l'ESP-32 à l'heure du pillulier'...");

  tm.tm_year = yr - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = mday;
  tm.tm_wday = wday;
  tm.tm_hour = hr;
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;
  time_t t = mktime(&tm);
  Serial.printf("Paramètrage de l'heure: %s", asctime(&tm));
  struct timeval now = {.tv_sec = t};
  settimeofday(&now, NULL);
}

void setupBLE()
{
  BLEDevice::init("ESP32_Pilulier");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
  Serial.println("BLE Advertising started");
}

// callback lorsque les données sont envoyées
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println("Data Send");
  // Serial.print("\r\nStatut du dernier paquet envoyé:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Succès de la livraison" : "Échec de la livraison");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  Serial.println("Data received");
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Message: ");
  Serial.println(incomingMessage.r);

  if (strcmp(incomingMessage.r, "OK") == 0)
  {
    isalreadysend = true;
  }
  // else if (strcmp(incomingMessage.r, "VIDE") == 0)
  // {
  //   isalreadysend = false;
  // }
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
  esp_now_register_recv_cb(OnDataRecv);

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

  // Setup BLE
  setupBLE();

  // Setup button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
  if (isalreadysend == false && receivedBluetooth == true)
  {
    Serial.println(myData.c);
    Serial.println(myData.j);
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  }

  // Check if button is pressed
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (!buttonPressed)
    {
      buttonPressed = true;
      struct_message stopMessage;
      strncpy(stopMessage.c, "STOP", 32);
      esp_now_send(broadcastAddress, (uint8_t *)&stopMessage, sizeof(stopMessage));
      Serial.println("STOP message sent");
    }
  }
  else
  {
    buttonPressed = false;
  }

  delay(1000);
}