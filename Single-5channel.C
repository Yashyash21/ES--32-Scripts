#include <pgmspace.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define THINGNAME "Synetra"
#define PI_ID "SDS1001"

String relayTopic;
String esp_id;

const char AWS_IOT_ENDPOINT[] = "ne9e6696.ala.asia-southeast1.emqxsl.com";

/* ================= RELAY CONFIG ================= */

#define RELAY_COUNT 5
int relayPins[RELAY_COUNT] = {5, 18, 19, 21, 22};

#define RELAY_ON HIGH
#define RELAY_OFF LOW

/* ================= BLE ================= */

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-1234-1234-abcdefabcdef"

String wifi_ssid = "";
String wifi_password = "";
bool wifiReceived = false;
bool bleRunning = false;

unsigned long lastWifiAttempt = 0;
unsigned long lastHeartbeat = 0;
unsigned long wifiLostTime = 0;

/* ================= WIFI STORAGE ================= */

Preferences preferences;

/* ================= MQTT ================= */

WiFiClientSecure net;
PubSubClient client(net);

/* ================= ESP ID ================= */

String generateEspId()
{
  uint64_t chipid = ESP.getEfuseMac();

  char id[32];

  sprintf(id, "ESP32_%04X%08X",
          (uint16_t)(chipid >> 32),
          (uint32_t)chipid);

  return String(id);
}

/* ================= HELLO REGISTER ================= */

void sendHello()
{
  StaticJsonDocument<100> doc;
  doc["relay_count"] = RELAY_COUNT;

  char buffer[100];
  serializeJson(doc, buffer);

  String topic =
      "home/" + String(PI_ID) + "/esp32/hello/" + esp_id;

  client.publish(topic.c_str(), buffer, true);

  Serial.println("ESP REGISTER SENT");
}

/* ================= HEARTBEAT ================= */

void sendHeartbeat()
{
  if (!client.connected())
    return;

  if (millis() - lastHeartbeat > 15000)
  {
    String topic =
        "home/" + String(PI_ID) + "/" + esp_id + "/heartbeat";

    client.publish(topic.c_str(), "ONLINE");

    lastHeartbeat = millis();

    Serial.println("Heartbeat sent");
  }
}

/* ================= MQTT CONNECT ================= */

void connectAWS()
{
  if (client.connected())
    return;

  Serial.println("Connecting to MQTT...");

  client.disconnect();
  net.stop();

  client.setServer(AWS_IOT_ENDPOINT, 8883);

  if (client.connect(esp_id.c_str(), "Synetra", "Synetra@123"))
  {
    Serial.println("MQTT Connected!");

    relayTopic =
        "home/" + String(PI_ID) + "/" + esp_id + "/relay/+";

    client.subscribe(relayTopic.c_str());

    Serial.println("Subscribed:");
    Serial.println(relayTopic);

    sendHello();
  }
  else
  {
    Serial.print("MQTT Failed: ");
    Serial.println(client.state());
    delay(3000); // prevent reconnect spam
  }
}

/* ================= BLE CALLBACK ================= */

class MyCallbacks : public BLECharacteristicCallbacks
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {

    String value = pCharacteristic->getValue();

    if (value.length() > 0)
    {

      Serial.println("BLE Data Received:");

      String data = "";
      for (int i = 0; i < value.length(); i++)
      {
        data += (char)value[i];
      }

      Serial.println(data);

      StaticJsonDocument<200> doc;
      deserializeJson(doc, data);

      wifi_ssid = doc["ssid"].as<String>();
      wifi_password = doc["password"].as<String>();

      Serial.print("SSID: ");
      Serial.println(wifi_ssid);

      Serial.print("PASS: ");
      Serial.println(wifi_password);

      preferences.begin("wifi", false);
      preferences.putString("ssid", wifi_ssid);
      preferences.putString("pass", wifi_password);
      preferences.end();

      Serial.println("WiFi Saved");

      WiFi.disconnect(true); // reset WiFi
      delay(500);

      wifiReceived = true;
    }
  }
};

/* ================= MQTT MESSAGE HANDLER ================= */

void messageHandler(char *topic, byte *payload, unsigned int length)
{
  String msg;

  for (int i = 0; i < length; i++)
  {
    msg += (char)payload[i];
  }

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Message: ");
  Serial.println(msg);

  String topicStr = String(topic);

  String base =
      "home/" + String(PI_ID) + "/" + esp_id + "/relay/";

  if (topicStr.startsWith(base))
  {
    int relayNo = topicStr.substring(base.length()).toInt();

    if (relayNo >= 1 && relayNo <= RELAY_COUNT)
    {
      bool relayState = (msg == "ON");

      digitalWrite(relayPins[relayNo - 1],
                   relayState ? RELAY_ON : RELAY_OFF);

      Serial.print("Relay ");
      Serial.print(relayNo);
      Serial.print(" -> ");
      Serial.println(msg);
    }
  }
}

/* ================= BLE START ================= */

void startBLE()
{
  if (bleRunning)
    return;

  BLEDevice::deinit(true);
  BLEDevice::init(PI_ID);

  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic =
      pService->createCharacteristic(
          CHARACTERISTIC_UUID,
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("BLE Ready. Send WiFi JSON.");
  bleRunning = true;
}

/* ================= SETUP ================= */

void setup()
{
  Serial.begin(115200);

  esp_id = generateEspId();
  Serial.print("ESP ID: ");
  Serial.println(esp_id);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  net.setInsecure();
  net.setHandshakeTimeout(30);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(messageHandler);

  client.setKeepAlive(60);
  client.setSocketTimeout(30);

  for (int i = 0; i < RELAY_COUNT; i++)
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], RELAY_OFF);
  }

  preferences.begin("wifi", true);

  wifi_ssid = preferences.getString("ssid", "");
  wifi_password = preferences.getString("pass", "");

  preferences.end();

  if (wifi_ssid != "")
  {
    Serial.println("Saved WiFi Found");
    wifiReceived = true;
  }
  else
  {
    Serial.println("No WiFi Saved → Starting BLE");
    startBLE();
  }
}

/* ================= LOOP ================= */

void loop()
{

  /* Track WiFi loss time */
  if (WiFi.status() != WL_CONNECTED)
  {
    if (wifiLostTime == 0)
      wifiLostTime = millis();
  }
  else
  {
    wifiLostTime = 0;
  }

  /* Start BLE if WiFi unavailable for 30 seconds */
  if (wifiLostTime > 0 && millis() - wifiLostTime > 30000 && !bleRunning)
  {
    Serial.println("WiFi unavailable → Starting BLE");
    startBLE();
  }

  if (wifiReceived)
  {
    if (WiFi.status() == WL_DISCONNECTED &&
        millis() - lastWifiAttempt > 5000)
    {
      Serial.println("Trying WiFi...");

      WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
      lastWifiAttempt = millis();
    }

    /* If WiFi not connected within 20 seconds → restart BLE */
    if (WiFi.status() != WL_CONNECTED &&
        millis() - lastWifiAttempt > 60000)
    {
      Serial.println("WiFi connection failed → Restarting BLE");

      BLEDevice::getAdvertising()->stop();
      bleRunning = false;
      startBLE();
      wifiReceived = false;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi Connected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());

      delay(2000);

      connectAWS();

      if (bleRunning)
      {
        BLEDevice::getAdvertising()->stop();
        Serial.println("BLE Stopped");
        bleRunning = false;
      }

      wifiReceived = false;
    }
  }

  if (WiFi.status() == WL_DISCONNECTED && wifi_ssid != "")
  {
    if (millis() - lastWifiAttempt > 10000)
    {
      Serial.println("WiFi reconnecting...");
      WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
      lastWifiAttempt = millis();
    }
    return;
  }

  if (!client.connected())
  {
    connectAWS();
  }

  client.loop();

  sendHeartbeat();
}