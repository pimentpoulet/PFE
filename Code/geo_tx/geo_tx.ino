/**
 * Heltec Wireless Stick Lite V3 - Émetteur LoRa - Range test
 * -----------------------------------------------------------
 * Code optimisé 100% compatible avec heltec_unofficial.h
 */

#include <heltec_unofficial.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <Arduino.h>

/* --- Configuration LoRa --- */
#define FREQUENCY           915.0    // MHz
#define BANDWIDTH           125.0    // kHz
#define SPREADING_FACTOR    9
#define TRANSMIT_POWER      14       // dBm

float Lat = 0.0;
float Lon = 0.0;

void setup() {
  heltec_setup();
  Serial.begin(115200);
  delay(2000);

  // VEXT power on
  Serial.println("Activation de l'alimentation Vext...");
  heltec_ve(true); 
  delay(1000);
  Serial.println("-> Vext active !");

  // radio config
  Serial.println("Initialisation du module LoRa...");
  RADIOLIB_OR_HALT(radio.begin());
  radio.setFrequency(FREQUENCY);
  radio.setBandwidth(BANDWIDTH);
  radio.setSpreadingFactor(SPREADING_FACTOR);
  radio.setOutputPower(TRANSMIT_POWER);
  
  Serial.println("Heltec prêt : Attente des coordonnées de l'iPhone...");
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    if (data.startsWith("LAT:")) {
      int commaIndex = data.indexOf(",LON:");
      if (commaIndex != -1) {
        Lat = data.substring(4, commaIndex).toFloat();
        Lon = data.substring(commaIndex + 5).toFloat();

        // Affichage local pour debug
        Serial.print("Coordonnées reçues : ");
        Serial.print(Lat, 6);
        Serial.print(", ");
        Serial.println(Lon, 6);

        // --- NOUVEAU : ENVOI LORA ---
        // On prépare la chaîne de caractères à envoyer
        char txPacket[64];
        snprintf(txPacket, sizeof(txPacket), "LAT:%.6f,LON:%.6f", Lat, Lon);

        Serial.print("Envoi LoRa en cours... ");
        heltec_led(50);

        // Transmission radio
        int state = radio.transmit(txPacket);
        heltec_led(0);

        if (state == RADIOLIB_ERR_NONE) {
          Serial.println("Succès !");
          Serial.print("ACK:RECU_OK");
        } else {
          Serial.printf("Échec, code : %i\n", state);
        }
      }
    }
  }
}
