#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==============================================================
//                           OLED DISPLAY
// --------------------------------------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// ==============================================================

// ==============================================================
//                          DHT22 sensor
// --------------------------------------------------------------
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Declare a struct to define the values that sensor reading function should return
struct DHTReadings
{
  float h;
  float t;
  float f;
  float hif;
  float hic;
  bool valid;
};

// Declare a function to return sensor reading values
DHTReadings readDHT22();

// Declare function to print things in serial monitor
void printSerial(float h, float t, float f, float hic, float hif);
// ==============================================================

// ==============================================================
// Count seconds since program started to execute things every
// selected interval of time
// --------------------------------------------------------------
const unsigned long INTERVAL = 1000; // ms
unsigned long lastRun = 0;
// ==============================================================

void setup()
{
  Serial.begin(9600);
  Serial.println("DHT22 TEST!");
  dht.begin();
}

void loop()
{
  if (millis() - lastRun >= INTERVAL)
  {
    lastRun = millis();

    // Get values from DHT22 sensor
    DHTReadings readings = readDHT22();

    // Print DHT22 values in serial monitor
    printSerial(readings.h, readings.t, readings.f, readings.hic, readings.hif);
  };
};

DHTReadings readDHT22()
{
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  return {h, t, f, hif, hic, true};
};

void printSerial(float h, float t, float f, float hic, float hif)
{
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
};