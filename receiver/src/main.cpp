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

void setup()
{
  Serial.begin(9600);

  // Initialize LoRa
  Serial.println("Initializing LoRa...");

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
}

void loop()
{
  // Receive message with LoRa
  receiveMessage();
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
  }
  else if (state != RADIOLIB_ERR_RX_TIMEOUT)
  {

    Serial.print("Receive failed: ");
    Serial.println(state);
  }
}