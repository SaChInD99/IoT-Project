#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "Dialog 4G 568"; // Enter your WiFi name
const char *password = "77A1B5F1";  // Enter WiFi password
// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";// broker address
const char *topic = "esp32/plant_chamber"; // define topic 
const char *mqtt_username = "emqx"; // username for authentication
const char *mqtt_password = "public";// password for authentication
const int mqtt_port = 1883;// port of MQTT over TCP

// Create an instance of the MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

//make variables for each pin
const int soilMoisturePin = 34;  // GPIO pin to which soil moisture sensor is connected
const int ldrPin = 35;           // GPIO pin to which LDR module is connected
const int motorPin = 12;         // GPIO pin to which the motor is connected
const int lightPin = 13;         // GPIO pin to which the light is connected
const int fanPin = 4;           // GPIO pin to which the fan is connected
const int dhtPin = 14;           // GPIO pin to which DHT11 sensor is connected

DHT dht(dhtPin, DHT11);          // Create a DHT object


// SETup#############################################################################
void setup() {
  //start serial communication
  Serial.begin(115200);

 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");

  //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 client.publish(topic, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic);
  //set output pins
  pinMode(motorPin, OUTPUT); // Set motor pin as output
  pinMode(lightPin, OUTPUT); // Set light pin as output
  pinMode(fanPin, OUTPUT);   // Set fan pin as output

  dht.begin();               // Initialize the DHT sensor

}

void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");
}

//loop#################################################################
void loop() {
  // Read soil moisture value from the sensor
  int soilMoisture = analogRead(soilMoisturePin);
  // Read light intensity from the LDR module
  int lightIntensity = analogRead(ldrPin);

  // Map the analog values (0-4095) to moisture percentage (0-100) and light intensity (0-100)
  int moisturePercentage = map(soilMoisture, 4095, 0, 0, 100);
  int lightPercentage = map(lightIntensity, 4095, 0, 0, 100);

  // Read temperature and humidity from the DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Soil %: ");
  Serial.println(moisturePercentage);
  Serial.print("Light Intensity %: ");
  Serial.println(lightPercentage);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

// Connect to MQTT broker********
  if (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
    }
  }
  // Check soil moisture condition
  if (moisturePercentage < 20) {
    digitalWrite(motorPin, HIGH); // Turn on the motor
    Serial.println("Watering the plant! turn on water pump"); // Print a message indicating watering
  } else {
    digitalWrite(motorPin, LOW); // Turn off the motor
  }

  // Check light condition 
  if (lightPercentage > 70) {
    digitalWrite(lightPin, HIGH); // Turn on the light
    Serial.println("Turning on the light!"); // Print a message indicating light is on
  } else {
    digitalWrite(lightPin, LOW); // Turn off the light
  }

  // Check temperature condition
  if (temperature > 30) {
    digitalWrite(fanPin, HIGH); // Turn on the fan
    Serial.println("Turning on the fan!"); // Print a message indicating fan is on
  } else {
    digitalWrite(fanPin, LOW); // Turn off the fan
  }
// Publish sensor data to MQTT topics
  client.publish("plant_chamber/temperature", String(temperature).c_str());
  client.publish("plant_chamber/humidity", String(humidity).c_str());
  client.publish("plant_chamber/light", String(lightIntensity).c_str());
  client.publish("plant_chamber/moisture", String(soilMoisture).c_str());
  client.publish("plant_chamber/moisture/presentage", String(moisturePercentage).c_str());
  client.publish("plant_chamber/light/presentage", String(lightPercentage).c_str());
  delay(5000); // Delay for 1 second
}
