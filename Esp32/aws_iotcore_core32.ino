#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>

// MQTT topics
#define AWS_IOT_PUBLISH_TOPIC "/telemetry"
#define AWS_IOT_SUBSCRIBE_TOPIC "/downlink"

// DHT11 inställningar
#define DHTPIN 21        // GPIO-pinnen där DHT11 är ansluten
#define DHTTYPE DHT11   // Typ av sensor
DHT dht(DHTPIN, DHTTYPE);

long sendInterval = 10000; // Interval mellan publiceringar (10 sekunder)

String THINGNAME = "YourDeviceName";

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(1024);

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Generera THINGNAME från MAC-adress
  THINGNAME = WiFi.macAddress();
  for (int i = 0; i < THINGNAME.length(); i++) {
    if (THINGNAME.charAt(i) == ':') {
      THINGNAME.remove(i, 1);
      i--;
    }
  }

  Serial.println();
  Serial.print("MAC Address (THINGNAME): ");
  Serial.println(THINGNAME);

  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Konfigurera AWS IoT
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // MQTT-hantering
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IoT");
  while (!client.connect(THINGNAME.c_str())) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  client.subscribe(THINGNAME + AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("\nAWS IoT Connected!");
}

void setupShadow() {
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/get/accepted");
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/get/rejected");
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/update/delta");

  client.publish("$aws/things/" + THINGNAME + "/shadow/get");
}

bool publishTelemetry(String payload) {
  Serial.print("Publishing: ");
  Serial.println(payload);
  return client.publish(THINGNAME + AWS_IOT_PUBLISH_TOPIC, payload);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("Incoming: " + topic + " - " + payload);
}

void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected, reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
  }
}


void setup() {
  Serial.begin(115200);
  dht.begin();  // Starta DHT11
  delay(2000);
  connectAWS();
  setupShadow();
}

void loop() {
  static unsigned long previousMillis = -sendInterval;

  ensureWiFiConnected(); // Kontrollera och återanslut Wi-Fi vid behov

  client.loop(); // Hantera MQTT-loop

  if (millis() - previousMillis >= sendInterval) {
    previousMillis = millis();

    // Läser temperatur och luftfuktighet från DHT11
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Skapar JSON payload
    String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    if (!publishTelemetry(payload)) {
      Serial.println("Failed to publish, reconnecting MQTT...");
      connectAWS();
    }
  }
}