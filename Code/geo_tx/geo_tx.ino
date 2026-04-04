/**
 * Base Fixe (TX) - Émetteur LoRa continu
 * Envoie un ping constant pour le test de portée.
 */

#include <heltec_unofficial.h>

#define FREQUENCY           915.0
#define BANDWIDTH           125.0
#define SPREADING_FACTOR    12
#define TRANSMIT_POWER      21 // Puissance maximale conservée

void setup() {
  heltec_setup();
  Serial.begin(115200);
  delay(2000);

  Serial.println("Initialisation du module LoRa (Base TX)...");
  RADIOLIB_OR_HALT(radio.begin());
  radio.setFrequency(FREQUENCY);
  radio.setBandwidth(BANDWIDTH);
  radio.setSpreadingFactor(SPREADING_FACTOR);
  radio.setOutputPower(TRANSMIT_POWER);

  Serial.println("Base prête : Envoi d'un ping constant.");
}

void loop() {
  String payload = "PING"; // Message simple
  
  Serial.print("Envoi du signal... ");
  heltec_led(50);
  int state = radio.transmit(payload);
  heltec_led(0);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Succès !");
  } else {
    Serial.printf("Échec, code : %i\n", state);
  }
  
  delay(1000); // Attente de 1 seconde entre chaque envoi
}
