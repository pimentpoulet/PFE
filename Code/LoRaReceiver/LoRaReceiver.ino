/**
 * LoRa Receveur (RX uniquement)
 */

#define HELTEC_POWER_BUTTON
#define HELTEC_NO_DISPLAY
#include <heltec_unofficial.h>

#define FREQUENCY           915.0
#define BANDWIDTH           125.0
#define SPREADING_FACTOR    9

String rxdata;
volatile bool rxFlag = false;

void setup() {
  heltec_setup();
  Serial.println("Radio init Receveur");
  RADIOLIB_OR_HALT(radio.begin());

  // Définit la fonction de callback pour la réception
  radio.setDio1Action(rx);

  // Configuration de la radio (doit être identique à l'émetteur)
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));

  // Démarre la réception (écoute infinie)
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
}

void loop() {
  heltec_loop();

  // Si un paquet a été reçu, on l'affiche avec son RSSI et SNR
  if (rxFlag) {
    rxFlag = false;
    radio.readData(rxdata);

    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      Serial.printf("RX [%s]\n", rxdata.c_str());
      Serial.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
      Serial.printf("  SNR: %.2f dB\n", radio.getSNR());
    }

    // Relance l'écoute après avoir lu le message
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  }
}

// Fonction appelée par interruption lors de la réception
// Il ne faut pas y mettre de Serial.print() ici pour des raisons de performance
void rx() {
  rxFlag = true;
}
