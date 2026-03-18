/**
 * Heltec Wireless Stick Lite V3 - Émetteur LoRa + Capteur SCD41
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

/* --- Configuration Matérielle --- */
#define PIN_SCL          41
#define PIN_SDA          42
#define SCD4x_ADDRESS    0x62    // Adresse I2C du SCD41

// Temps de sommeil en secondes (12h = 43200 | Test = 10)
#define SLEEP_TIME_SEC  10

/* --- Variables Globales --- */
SensirionI2cScd4x sensor;
uint64_t serialNumber = 0;
bool sensorFound = false;

/* --- Setup Principal --- */
void setup() {
    // 1. Initialisation système Heltec (Gère la radio et les pins internes)
    heltec_setup();
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n--- DEMARRAGE SYSTEME ISLE ---");

    // 2. VEXT power on
    Serial.println("Activation de l'alimentation Vext...");
    heltec_ve(true); 
    delay(1000); // Laisse le temps au capteur de s'allumer
    Serial.println("-> Vext active !");

    // 3. Initialisation du bus I2C et du Capteur
    Serial.println("Recherche du capteur SCD41...");
    Wire1.begin(PIN_SDA, PIN_SCL);
    sensor.begin(Wire1, SCD4x_ADDRESS);

    if (sensor.getSerialNumber(serialNumber) == 0) {
        Serial.print("-> Capteur trouve ! Serial: 0x");
        Serial.print((uint32_t)(serialNumber >> 32), HEX);
        Serial.println((uint32_t)(serialNumber & 0xFFFFFFFF), HEX);
        sensorFound = true;
        sensor.stopPeriodicMeasurement(); // Préparation Single Shot
    } else {
        Serial.println("-> ERREUR : Capteur introuvable. Verifiez le cablage.");
    }

    // 4. Initialisation de la Radio
    Serial.println("Initialisation du module LoRa...");
    RADIOLIB_OR_HALT(radio.begin());
    radio.setFrequency(FREQUENCY);
    radio.setBandwidth(BANDWIDTH);
    radio.setSpreadingFactor(SPREADING_FACTOR);
    radio.setOutputPower(TRANSMIT_POWER);

    Serial.println("--- SYSTEME PRET ---");

    // 5. Mesures
    float temp = 0.0, hum = 0.0;
    uint16_t co2 = 0;
    
    // Lecture de la tension avec la fonction native Heltec
    float vbat = heltec_vbat();

    if (sensorFound) {
        Serial.println("\nLancement de la mesure SCD41 (attente 5s)...");
        sensor.measureSingleShot();
        delay(5000); // 5s strictes requises par le SCD41
        
        if (sensor.readMeasurement(co2, temp, hum) == 0) {
            Serial.printf("Donnees -> CO2: %d ppm | Temp: %.2f C | Hum: %.2f %% | Vbat: %.2f V\n", co2, temp, hum, vbat);
        } else {
            Serial.println("Erreur lors de la lecture des donnees du capteur.");
        }
    } else {
        Serial.println("\nCapteur non detecte, affichage batterie uniquement.");
        Serial.printf("Vbat: %.2f V\n", vbat);
    }

    // 6. Préparation et envoi du JSON
    char txPacket[128];
    snprintf(txPacket, sizeof(txPacket), "{\"v\":%.2f,\"t\":%.2f,\"h\":%.2f,\"c\":%d}", vbat, temp, hum, co2);

    heltec_led(50); 
    Serial.print("Envoi LoRa : "); 
    Serial.println(txPacket);

    int state = radio.transmit(txPacket);
    heltec_led(0); 

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(">>> Paquet LoRa envoye avec succes !");
    } else {
        Serial.printf(">>> Erreur de transmission LoRa, code : %i\n", state);
    }

    // 7. Mise en veille profonde via Heltec
    Serial.printf("Mise en veille propre pour %d secondes...\n", SLEEP_TIME_SEC);
    
    // heltec_deep_sleep s'occupe d'éteindre Vext, la radio, et de paramétrer le timer
    heltec_deep_sleep(SLEEP_TIME_SEC); 
}

void loop() {
    // Jamais atteint
}
