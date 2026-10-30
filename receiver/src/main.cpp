#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <RadioLib.h>

// ==============================================================
// LoRa Receiver
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
void receiveMessage();
// ==============================================================

// ==============================================================
//                           OLED DISPLAY
// --------------------------------------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
      433.0, // Frequency
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
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Waiting for message..");
  // Receive message with LoRa
  receiveMessage();

  display.display();
};

void receiveMessage()
{
  String receivedMessage;

  int state = radio.receive(receivedMessage);

  if (state == RADIOLIB_ERR_NONE)
  {

    Serial.print("Received message: ");
    Serial.println(receivedMessage);

    Serial.print("RSSI: ");
    Serial.println(radio.getRSSI());

    Serial.print("SNR: ");
    Serial.println(radio.getSNR());

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("MESSAGE");
    display.setCursor(0, 15);
    display.print("RECEIVED");
  }
  else if (state != RADIOLIB_ERR_RX_TIMEOUT)
  {

    Serial.print("Receive failed: ");
    Serial.println(state);
  }
}