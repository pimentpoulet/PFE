/**
 * LoRa Emetteur (TX uniquement)
 */

#define HELTEC_POWER_BUTTON
#define HELTEC_NO_DISPLAY
#include <heltec_unofficial.h>

#define PAUSE               10
#define FREQUENCY           915.0
#define BANDWIDTH           125.0
#define SPREADING_FACTOR    9
#define TRANSMIT_POWER      8

long counter = 0;
uint64_t last_tx = 0;
uint64_t tx_time;
uint64_t minimum_pause;

void setup() {
  heltec_setup();
  Serial.println("Radio init Emetteur");
  RADIOLIB_OR_HALT(radio.begin());

  // Configuration de la radio
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
}

void loop() {
  heltec_loop();

  bool tx_legal = millis() > last_tx + minimum_pause;

  // Transmet un paquet toutes les PAUSE secondes ou quand le bouton est pressé
  if ((PAUSE && tx_legal && millis() - last_tx > (PAUSE * 1000)) || button.isSingleClick()) {

    if (!tx_legal) {
      Serial.printf("Limite légale (Duty Cycle), attendez %i sec.\n", (int)((minimum_pause - (millis() - last_tx)) / 1000) + 1);
      return;
    }

    Serial.printf("TX [%s] ", String(counter).c_str());
    heltec_led(50); // Allume la LED pendant l'émission

    tx_time = millis();
    RADIOLIB(radio.transmit(String(counter++).c_str()));
    tx_time = millis() - tx_time;

    heltec_led(0); // Éteint la LED

    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      Serial.printf("OK (%i ms)\n", (int)tx_time);
    } else {
      Serial.printf("Echec (%i)\n", _radiolib_status);
    }

    // Respect du duty cycle (maximum 1% du temps d'émission)
    minimum_pause = tx_time * 100;
    last_tx = millis();
  }
}
