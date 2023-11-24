
#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);



const char *WIFI_SSID = "AndroidAP";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "chzk3278";      // Put here your Wi-Fi password



uint8_t ldr_Pin = 34;
float ldr_Value;

uint8_t soil_Pin = 35;
float soil_Value;

float humidity;
float temperature;

int lamp = 18;
int pump = 19;
int farm_Lamp = 0;
int farm_Pump = 0;


const int PUBLISH_FREQUENCY = 30000; // Update rate in milliseconds
unsigned long timer;


//Ubidots ubidots(UBIDOTS_TOKEN);
//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

int states = 0;

//float value1;


void setup()
{
  pinMode(lamp, OUTPUT);
  pinMode(pump, OUTPUT);
  digitalWrite(lamp, HIGH);
  digitalWrite(pump, HIGH);

  // initialize the LCD
  lcd.begin(16, 2);

  // Turn on the blacklight and print a message.
  lcd.backlight();


  Serial.begin(115200);
 
  timer = millis();

  Serial.println(F("DHTxx test!"));
  dht.begin();

}


void loop()
{
  
  delay(500);
  read_DHT11();
  read_Sensors();

}
}


void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void read_DHT11() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, humidity);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
}

void read_Sensors() {
  ldr_Value = analogRead(ldr_Pin); // LDR
  Serial.print(ldr_Value);
  if (ldr_Value < 500.0) {
    digitalWrite(lamp, LOW);
    farm_Lamp = 1;
  }
  else {
    digitalWrite(lamp, HIGH);
    farm_Lamp = 0;
  }

  ldr_Value = map(ldr_Value, 0, 4096, 0, 100);

  delay(500);

  soil_Value = analogRead(soil_Pin); // LDR
  Serial.print(soil_Value);

  if (soil_Value < 2000.0) {
    digitalWrite(pump, HIGH);
    farm_Pump = 0;
  }
  else {
    digitalWrite(pump, LOW);
    farm_Pump = 1;
  }

  soil_Value = map(soil_Value, 0, 4096, 100, 0);


  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  int tem = temperature;
  int humi = humidity;
  int soil = soil_Value;
  int ldr = ldr_Value;
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


}