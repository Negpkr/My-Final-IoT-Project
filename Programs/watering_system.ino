//Negin Pakrooh

//Sample Arduino code; only one plant implemented, we can easily add second soil moisture sensor
// and pump to make the program handle several plants

#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <DHT.h>

// Pin Definitions
#define DHTPIN 2  
#define SOIL_MOISTURE_PIN A1  
#define PUMP_PIN 5  =
#define DHTTYPE DHT22  

// WiFi Credentials
const char* ssid = "NOKIA-2731"; 
const char* password = "******";  

// MQTT Broker (HiveMQ)
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientID = "ArduinoClient";

// MQTT Topics
const char* SENSOR_DATA_TOPIC = "plant/sensor";
const char* PUMP_CONTROL_TOPIC = "plant/pump";
const char* SOIL_CONTROL_TOPIC = "plant/soil";

DHT dht(DHTPIN, DHTTYPE);  
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);

  dht.begin();

  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);  

  // Connect to WiFi
  connectWiFi();

  // Configure MQTT client
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);

  // Connect to MQTT broker
  connectMQTT();
}

void loop() {
  // Ensure MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // Publish both sensor and soil data every 5 seconds (for testing)
  publishSensorData();
  publishSoilData();
  delay(5000);  // Wait
}

// Connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
}

// Connect to MQTT broker
void connectMQTT() {
  Serial.print("Connecting to MQTT broker...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(clientID)) {
      Serial.println("\nConnected to MQTT broker.");
      // Subscribe to pump control topic
      mqttClient.subscribe(PUMP_CONTROL_TOPIC);
      mqttClient.subscribe(SOIL_CONTROL_TOPIC);  // Subscribe to soil control topic
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

// Publish sensor data to MQTT topic
void publishSensorData() {
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println("Failed to read temperature from DHT sensor.");
    return;
  }

  // Create JSON payload with temperature data
  String payload = "{\"plant\":\"Plant 1\",\"temperature\":" + String(temperature) + "}";
  mqttClient.publish(SENSOR_DATA_TOPIC, payload.c_str());  // Publish to sensor topic
  Serial.println("Published sensor data: " + payload);
}

// Publish soil moisture data to MQTT topic
void publishSoilData() {
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  // Create JSON payload with soil moisture data
  String payload = "{\"plant\":\"Plant 1\",\"soilMoisture\":" + String(soilMoisture) + "}";
  mqttClient.publish(SOIL_CONTROL_TOPIC, payload.c_str());  // Publish to soil control topic
  Serial.println("Published soil data: " + payload);
}

// MQTT callback to handle incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message received on topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  // Handle pump control messages
  if (String(topic) == PUMP_CONTROL_TOPIC) {
    if (message == "{\"action\":\"on\"}") {
      digitalWrite(PUMP_PIN, HIGH);  // Turn on the pump
      Serial.println("Pump turned ON.");
    } else if (message == "{\"action\":\"off\"}") {
      digitalWrite(PUMP_PIN, LOW);  // Turn off the pump
      Serial.println("Pump turned OFF.");
    }
  }
}

