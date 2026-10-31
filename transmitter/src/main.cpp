#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <RadioLib.h>

// ================================================================================================
// BITMAPS
// ------------------------------------------------------------------------------------------------
// '2way', 13x13px
const unsigned char twoWay[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40, 0x28, 0xa0, 0x4f, 0x90, 0x80, 0x08, 0x4f, 0x90,
    0x28, 0xa0, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// ================================================================================================

// ==============================================================
//                           OLED DISPLAY
// --------------------------------------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void printTitles();
void displayValues(float h, float t, float f, float hic, float hif);
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
// LoRa Transmitter
// --------------------------------------------------------------
// ESP32 pin configuration
#define LORA_CS 5
#define LORA_DIO0 26
#define LORA_RST 14

// Create the LoRa radio object
SX1278 radio = new Module(
    LORA_CS,
    LORA_DIO0,
    LORA_RST,
    -1);

// Function to send a simple message
void sendMessage();

// Function to send sensor data
void sendSensor(const DHTReadings &readings);

// ==============================================================
// Count seconds since program started to execute things every
// selected interval of time
// --------------------------------------------------------------
const unsigned long INTERVAL = 2000; // ms
unsigned long lastRun = 0;
// ==============================================================

void setup()
{
  Serial.begin(9600);

  // Initialize DHT22 Sensor
  Serial.println("DHT22 TEST!");
  dht.begin();

  // Initialize LoRa
  Serial.println("Initializing LoRa...");

  // Initialize radio
  int state = radio.begin(
      470.0, // Frequency
      125.0, // Bandwidth
      9,     // Spreading factor
      7,     // Coding rate
      0x12,  // Sync word
      10,    // TX power
      8      // Preamble
  );

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa initialized!");
  }
  else
  {
    Serial.print("Initialization failed: ");
    Serial.println(state);

    while (true)
    {
      delay(10);
    }
  };

  // Initialize OLED Display

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.print("STARTING...");
  display.display();
  delay(1000);
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

    display.clearDisplay();
    printTitles();

    if (readings.valid)
    {
      displayValues(readings.h, readings.t, readings.f, readings.hic, readings.hif);
    }
    else
    {
      display.setCursor(0, 0);
      display.print("SENSOR FAIL");
    };

    // Send message with LoRa
    // sendMessage();

    // Send sensor data with LoRa
    sendSensor(readings);

    display.display();
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
    return {0, 0, 0, 0, 0, false};
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

void printTitles()
{
  // display.setCursor(0, 0);
  // display.print("DHT22 Sensor");

  display.drawLine(0, 14, 128, 14, WHITE);
  display.setCursor(0, 16);
  display.print("Humidity: ");

  display.drawLine(0, 25, 128, 25, WHITE);

  display.setCursor(0, 27);
  display.print("Temp in ");
  display.write(247);
  display.print("C");

  display.setCursor(65, 27);
  display.print("Temp in ");
  display.write(247);
  display.print("F");
  display.drawLine(63, 27, 63, 64, WHITE);
};

void displayValues(float h, float t, float f, float hic, float hif)
{
  display.setCursor(55, 16);
  display.print(h);
  display.print(" %");

  display.setTextSize(2);
  display.setCursor(0, 37);
  display.print(t);

  display.setCursor(65, 37);
  display.print(f);

  display.setTextSize(1);

  display.setCursor(0, 54);
  display.print("FL: ");
  display.print(hic);
  display.setCursor(65, 54);
  display.print("FL: ");
  display.print(hif);
}

void sendMessage()
{
  Serial.println("Sending message...");

  // Transmit a message
  int state = radio.transmit("Hello from ESP32!");

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("Message sent successfully!");

    // display.setCursor(0, 0);
    // display.print("Out ok!-- ");

    // Wait for the response
    String response;

    state = radio.receive(response);

    if (state == RADIOLIB_ERR_NONE)
    {
      Serial.print("Response: ");
      Serial.println(response);
    }
  }
  else
  {
    Serial.print("Transmission failed: ");
    Serial.println(state);

    display.setCursor(0, 0);
    display.print("TRANSM. FAILED!");
  }
}

void sendSensor(const DHTReadings &readings)
{
  // Check whether the sensor readings are valid
  if (!readings.valid)
  {
    Serial.println("Invalid sensor readings. Transmission skipped.");
    return;
  }

  Serial.println("Preparing weather data...");

  // Create a buffer to store the message
  char message[100];

  // Format the sensor readings into a string
  snprintf(
      message,
      sizeof(message),
      "H=%.1f,T=%.1f,F=%.1f,HIC=%.1f,HIF=%.1f",
      readings.h,
      readings.t,
      readings.f,
      readings.hic,
      readings.hif);

  Serial.print("Sending: ");
  Serial.println(message);

  // Transmit the message
  int state = radio.transmit(message);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("Message sent successfully!");

    display.setCursor(0, 3);
    display.print("TX OK!  ");

    // Wait for the response
    String response;

    state = radio.receive(response);

    if (state == RADIOLIB_ERR_NONE)
    {
      Serial.print("Response: ");
      Serial.println(response);

      // display.print(response);
      display.drawBitmap(40, 0, twoWay, 13, 13, WHITE);
    }
  }
  else
  {
    Serial.print("Transmission failed: ");
    Serial.println(state);

    display.setCursor(0, 0);
    display.print("TRANSM. FAILED!");
  }
}