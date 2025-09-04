#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 15 // Digital pin connected to the Dallas sensor
#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

float temperatureC;
float temperatureF;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Function declaration
void messageHandler(char *topic, byte *payload, unsigned int length);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["device"] = "ESP32_Dallas";
  doc["temperature_celsius"] = temperatureC;
  doc["temperature_fahrenheit"] = temperatureF;
  doc["timestamp"] = millis();

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  if (result)
  {
    Serial.println("Message published successfully!");
    Serial.println(jsonBuffer);
  }
  else
  {
    Serial.println("Failed to publish message");
  }
}

void messageHandler(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Incoming message from topic: ");
  Serial.println(topic);

  // Convert payload to string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Message: ");
  Serial.println(message);

  // Try to parse as JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (!error)
  {
    const char *msg = doc["message"];
    if (msg)
    {
      Serial.print("Parsed message: ");
      Serial.println(msg);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("ESP32 Dallas DS18B20 + AWS IoT Starting...");

  // Initialize Dallas Temperature sensor
  sensors.begin();
  Serial.println("Dallas Temperature sensor initialized");

  // Connect to AWS
  connectAWS();
}

void loop()
{
  // Request temperature readings
  sensors.requestTemperatures();

  // Read temperature in Celsius and Fahrenheit
  temperatureC = sensors.getTempCByIndex(0);
  temperatureF = sensors.getTempFByIndex(0);

  // Check if reading is valid
  if (temperatureC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data from Dallas sensor!");
    delay(1000);
    return;
  }

  // Print readings to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.print("°C / ");
  Serial.print(temperatureF);
  Serial.println("°F");

  // Publish to AWS IoT
  publishMessage();

  // Keep MQTT connection alive
  client.loop();

  // Check if still connected to AWS IoT
  if (!client.connected())
  {
    Serial.println("Lost connection to AWS IoT. Attempting to reconnect...");
    connectAWS();
  }

  delay(5000); // Send data every 5 seconds
}