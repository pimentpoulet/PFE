/**
 * LoRa Receveur (RX uniquement)
 * requires esp32 boards manager -> version 2.0.17
 */

#define HELTEC_POWER_BUTTON
#define HELTEC_NO_DISPLAY

#include <heltec_unofficial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define FREQUENCY 915.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 12

String rxdata;
volatile bool rxFlag = false;

void setup() {
  heltec_setup();

  // LoRa setup
  Serial.println("Radio init Receveur");
  RADIOLIB_OR_HALT(radio.begin());
  radio.setDio1Action(rx);

  // radio config
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));

  // listen
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
}

void loop() {
  heltec_loop();
  if (rxFlag) {
    rxFlag = false;

    // buffer to hold the 8 bytes
    float receivedCoords[2];
    int state = radio.readData((uint8_t*)receivedCoords, 8);

    if (state == RADIOLIB_ERR_NONE) {
      float receivedLat = receivedCoords[0];
      float receivedLon = receivedCoords[1];

      Serial.print("Coordonnées Reçues : ");
      Serial.print(receivedLat, 6);
      Serial.print(", ");
      Serial.println(receivedLon, 6);

      Serial.printf("DEBUG: RSSI: %.2f dBm | SNR: %.2f dB\n", radio.getRSSI(), radio.getSNR());
    }

    // listen again
    radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF);
  }
}

void rx() {
  rxFlag = true;
}
