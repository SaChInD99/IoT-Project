#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi
const char *ssid = "AndroidAP"; // Enter your WiFi name
const char *password = "chzk3278";  // Enter WiFi password
// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";// broker address
const char *topic = "esp32/anubias_chamber"; // define topic 
const char *mqtt_username = "emqx"; // username for authentication
const char *mqtt_password = "public";// password for authentication
const int mqtt_port = 1883;// port of MQTT over TCP

// Create an instance of the MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

//make variables for each pin
const int soilMoisturePin = 35;  // GPIO pin to which soil moisture sensor is connected
const int ldrPin = 34;           // GPIO pin to which LDR module is connected
const int motorPin = 19;         // GPIO pin to which the motor is connected
const int lightPin = 18;         // GPIO pin to which the light is connected
//const int farm_Lamp = 0;
//const int farm_Pump = 0;
const int dhtPin = 13;           // GPIO pin to which DHT11 sensor is connected

DHT dht(dhtPin, DHT11);          // Create a DHT object

//int states = 0;

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


  dht.begin();               // Initialize the DHT sensor
  // initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2); // Initialize a 16x2 display

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
  if (moisturePercentage < 50) {
    digitalWrite(motorPin, HIGH); // Turn on the motor
    //farm_Pump = 0;
    Serial.println("Watering the plant! turn on water pump"); // Print a message indicating watering
  } else {
    digitalWrite(motorPin, LOW); // Turn off the motor
    //farm_Pump = 1;
  }

  // Check light condition 
  if (lightPercentage > 60) {
    digitalWrite(lightPin, HIGH); // Turn on the light
    //farm_Lamp = 0;
    Serial.println("Turning on the light!"); // Print a message indicating light is on
  } else {
    digitalWrite(lightPin, LOW); // Turn off the light
    //farm_Lamp = 1;
  }

// (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  int tem = temperature;
  int humi = humidity;
  int soil = moisturePercentage;
  int ldr = lightPercentage;
  lcd.setCursor(0, 0);
  lcd.print("T> ");
  lcd.print(tem);
  lcd.print("C");

  lcd.setCursor(9, 0);
  lcd.print("H> ");
  lcd.print(humi);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("S> ");
  lcd.print(soil);
  lcd.print("%");

  lcd.setCursor(9, 1);
  lcd.print("L> ");
  lcd.print(ldr);
  lcd.print("%");

// Publish sensor data to MQTT topics
  client.publish("anubias_chamber/temperature", String(temperature).c_str());
  client.publish("anubias_chamber/humidity", String(humidity).c_str());
  client.publish("anubias_chamber/light", String(lightIntensity).c_str());
  client.publish("anubias_chamber/moisture", String(soilMoisture).c_str());
  client.publish("anubias_chamber/moisture/presentage", String(moisturePercentage).c_str());
  client.publish("anubias_chamber/light/presentage", String(lightPercentage).c_str());
  delay(5000); // Delay for 1 second
}
