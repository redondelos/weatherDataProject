#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <RadioLib.h>
#include <map>
#include <vector>

// ================================================================================================
//                                       SYMBOLS FOR BITMAPS
// ------------------------------------------------------------------------------------------------
const unsigned char waitingSymbol[] PROGMEM = {
    0x3f, 0xf0, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x08, 0x40, 0x0c, 0xc0, 0x07, 0x80, 0x07, 0x80,
    0x0c, 0xc0, 0x08, 0x40, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x3f, 0xf0};

// ================================================================================================

// ==============================================================
//                        LoRa RECEIVER
// --------------------------------------------------------------
// ESP32 pin configuration
#define LORA_CS 5
#define LORA_DIO0 26
#define LORA_RST 14

// Create the LoRa radio object
SX1278 radio = new Module(LORA_CS, LORA_DIO0, LORA_RST, -1);

// Function to send a simple message
String receiveMessage();

// Parse string message into variables
void parseMessage(String message);

// Function to check if value is valid once parsed
bool isValidNumber(String value);
// ==============================================================

// ==============================================================
//                           OLED DISPLAY
// --------------------------------------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void printTitles();

// Display values in OLED
void displayValues(float h, float t, float f, float hic, float hif);

void setup()
{
  Serial.begin(9600);

  // Initialize OLED Display

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);

  display.clearDisplay();
  display.setTextColor(WHITE);

  // Initialize LoRa
  Serial.println("Initializing LoRa...");

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("LoRa initializing");
  display.display();

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

    display.setCursor(0, 15);
    display.print("SUCCESS!");

    display.display();
  }
  else
  {
    Serial.print("Initialization failed: ");
    Serial.println(state);

    display.setCursor(0, 15);
    display.println("FAIL! - RESET");
    display.display();

    while (true)
    {
      delay(10);
    }
  };
}

void loop()
{
  display.clearDisplay();

  printTitles();

  // Parse message
  String message = receiveMessage();

  if (message.length() > 0)
  {

    parseMessage(message);
  }

  // displayValues(parseMessage);

  display.display();
};

void printTitles()
{
  // display.setCursor(0, 0);
  // display.print("DHT22 Sensor");

  display.setCursor(0, 0);
  display.println("TX: ");

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

String receiveMessage()
{
  String receivedMessage;

  int state = radio.receive(receivedMessage);

  if (state == RADIOLIB_ERR_NONE)
  {

    Serial.println();
    Serial.print("Received message: ");
    Serial.println(receivedMessage);

    Serial.print("RSSI: ");
    Serial.println(radio.getRSSI());

    Serial.print("SNR: ");
    Serial.println(radio.getSNR());

    Serial.print("TX: ");
    Serial.println(receivedMessage);

    display.setCursor(15, 0);
    display.print(" Data received!");

    // RESPONSE
    int txState = radio.transmit("Wave Back");

    if (txState == RADIOLIB_ERR_NONE)
    {
      Serial.println("Response transmitted!");
    }
  }
  else if (state != RADIOLIB_ERR_RX_TIMEOUT)
  {

    Serial.print("Receive failed: ");
    Serial.println(state);
  }
  else if (state != RADIOLIB_ERR_NONE)
  {
    display.setCursor(15, 0);
    display.print(" Waiting...");
    display.drawBitmap(80, 0, waitingSymbol, 14, 14, WHITE);
  };

  return receivedMessage;
}

void parseMessage(String message)
{
  // Find the beginning of each field
  int hStart = message.indexOf("H=");
  int tStart = message.indexOf("T=");
  int fStart = message.indexOf("F=");
  int hicStart = message.indexOf("HIC=");
  int hifStart = message.indexOf("HIF=");

  // Check that all labels exist
  if (hStart < 0 || tStart < 0 || fStart < 0 ||
      hicStart < 0 || hifStart < 0)
  {
    Serial.println("Invalid message!");
    return;
  }

  // Find the ending positions
  int hEnd = message.indexOf(",", hStart);
  int tEnd = message.indexOf(",", tStart);
  int fEnd = message.indexOf(",", fStart);
  int hicEnd = message.indexOf(",", hicStart);
  int hifEnd = message.length();

  // Check that the separators exist
  if (hEnd < 0 || tEnd < 0 || fEnd < 0 || hicEnd < 0)
  {
    Serial.println("Invalid message!");
    return;
  }

  // Extract text
  String humidityTxt = message.substring(hStart + 2, hEnd);
  String temperatureCTxt = message.substring(tStart + 2, tEnd);
  String temperatureFTxt = message.substring(fStart + 2, fEnd);
  String hicTxt = message.substring(hicStart + 4, hicEnd);
  String hifTxt = message.substring(hifStart + 4, hifEnd);

  // Validate all numerical fields
  if (!isValidNumber(humidityTxt) ||
      !isValidNumber(temperatureCTxt) ||
      !isValidNumber(temperatureFTxt) ||
      !isValidNumber(hicTxt) ||
      !isValidNumber(hifTxt))
  {
    Serial.println("Invalid numerical data!");
    return;
  }

  // Convert strings into floats
  float humidity = humidityTxt.toFloat();
  float temperatureC = temperatureCTxt.toFloat();
  float temperatureF = temperatureFTxt.toFloat();
  float heatIndexC = hicTxt.toFloat();
  float heatIndexF = hifTxt.toFloat();

  // Validate humidity range
  if (humidity < 0 || humidity > 100)
  {
    Serial.println("Humidity out of range!");
    return;
  }

  // Print measurements
  Serial.println("--- Parsed measurements ---");

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Temperature C: ");
  Serial.println(temperatureC);

  Serial.print("Temperature F: ");
  Serial.println(temperatureF);

  Serial.print("Heat Index C: ");
  Serial.println(heatIndexC);

  Serial.print("Heat Index F: ");
  Serial.println(heatIndexF);

  // Print in OLED
  displayValues(humidity, temperatureC, temperatureF, heatIndexC, heatIndexF);
}

bool isValidNumber(String value)
{
  bool decimalFound = false;
  bool negativeFound = false;

  if (value.length() == 0)
  {
    return false;
  }

  for (int i = 0; i < value.length(); i++)
  {

    char character = value.charAt(i);

    if (isDigit(character))
    {
      continue;
    }

    if (character == '.')
    {

      if (decimalFound)
      {
        return false;
      }

      decimalFound = true;
    }

    else if (character == '-')
    {

      if (i != 0 || negativeFound)
      {
        return false;
      }

      negativeFound = true;
    }

    else
    {
      return false;
    }
  }

  if (value == "-" || value == "." || value == "-.")
  {
    return false;
  }

  return true;
}

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