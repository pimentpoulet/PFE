/**
 * Unité Mobile (RX) - Écoute LoRa
 * Reçoit le ping et envoie le RSSI/SNR au script Python.
 * Code optimisé 100% compatible avec heltec_unofficial.h
 * requires esp32 boards manager -> version 2.0.17
 */

#include <heltec_unofficial.h>

#define FREQUENCY 915.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 12

volatile bool rxFlag = false;

void setup() {
  heltec_setup();
  Serial.begin(115200);
  delay(2000);

  Serial.println("Radio init (Mobile RX)");
  RADIOLIB_OR_HALT(radio.begin());
  radio.setDio1Action(rx);

  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));

  // Début de l'écoute continue
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  Serial.println("Heltec prêt : Écoute en cours...");
}

void loop() {
  heltec_loop();

  if (rxFlag) {
    rxFlag = false;
    String receivedData;
    int state = radio.readData(receivedData);

    if (state == RADIOLIB_ERR_NONE) {
      // Impression dans le format attendu par Python
      Serial.printf("DEBUG: RSSI: %.2f dBm | SNR: %.2f dB\n", radio.getRSSI(), radio.getSNR());
    }

    // On relance l'écoute
    radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF);
  }
}

void rx() {
  rxFlag = true;
}
