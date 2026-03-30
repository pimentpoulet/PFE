/**
 * Heltec Wireless Stick Lite V3 - Émetteur LoRa - Range test
 * -----------------------------------------------------------
 * Code optimisé 100% compatible avec heltec_unofficial.h
 * requires esp32 boards manager -> version 2.0.17
 */

#include <heltec_unofficial.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <Arduino.h>

/* --- Configuration LoRa --- */
#define FREQUENCY           915.0    // MHz
#define BANDWIDTH           125.0    // kHz
#define SPREADING_FACTOR    12
#define TRANSMIT_POWER      21       // dBm

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

        // --- ENVOI LORA ---

        // 4 bytes each = 8 bytes total
        float coords[2] = {Lat, Lon};

        Serial.print("Envoi LoRa binaire... ");
        heltec_led(50);
        int state = radio.transmit((uint8_t*)coords, 8);
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
