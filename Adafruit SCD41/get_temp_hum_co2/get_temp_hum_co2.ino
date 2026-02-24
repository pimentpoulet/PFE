/*
* Code adapté pour Wireless Stick Lite (ESP32-S3) + SCD41
* Pinout: SDA = GPIO 46, SCL = GPIO 45
*/

#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>

// Définition des pins personnalisées
#define PIN_SCL 45
#define PIN_SDA 46

SensirionI2cScd4x sensor;

static char errorMessage[64];
static int16_t error;

void PrintUint64(uint64_t& value) {
    Serial.print("0x");
    Serial.print((uint32_t)(value >> 32), HEX);
    Serial.print((uint32_t)(value & 0xFFFFFFFF), HEX);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // --- MODIFICATION CRITIQUE ICI ---
    // Initialisation du bus I2C sur tes pins spécifiques
    // Wire.begin(SDA, SCL);
    Wire.begin(PIN_SDA, PIN_SCL);
    // --------------------------------

    sensor.begin(Wire, SCD41_I2C_ADDR_62);

    uint64_t serialNumber = 0x62;
    delay(30);

    // Étape 1 : Réveiller le capteur (il est en idle par défaut)
    error = sensor.wakeUp();
    if (error != 0) {
        Serial.print("Erreur wakeUp(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }

    // Étape 2 : Arrêter les mesures en cours (au cas où il a été redémarré à chaud)
    error = sensor.stopPeriodicMeasurement();
    if (error != 0) {
        Serial.print("Erreur stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }

    // Étape 3 : Réinitialisation logicielle
    error = sensor.reinit();
    if (error != 0) {
        Serial.print("Erreur reinit(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }

    // Lecture du numéro de série pour confirmer la connexion
    error = sensor.getSerialNumber(serialNumber);
    if (error != 0) {
        Serial.print("Erreur getSerialNumber(). Vérifiez le câblage ! ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        // On bloque ici si le capteur n'est pas trouvé
        while (1) {
            delay(1000);
            Serial.println("Capteur introuvable...");
        }
    }

    Serial.print("Capteur trouvé ! Serial number: ");
    PrintUint64(serialNumber);
    Serial.println();

    // Démarrage de la mesure périodique (toutes les 5 secondes)
    error = sensor.startPeriodicMeasurement();
    if (error != 0) {
        Serial.print("Erreur startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    Serial.println("Démarrage des mesures...");
}

void loop() {
    bool dataReady = false;
    uint16_t co2Concentration = 0;
    float temperature = 0.0;
    float relativeHumidity = 0.0;

    // On attend 5 secondes car le capteur ne mesure pas plus vite en mode normal
    delay(5000);

    // Vérification si la donnée est prête
    error = sensor.getDataReadyStatus(dataReady);
    if (error != 0) {
        Serial.print("Erreur getDataReadyStatus(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }

    // Si la donnée est prête, on la lit
    if (dataReady) {
        error = sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
        if (error != 0) {
            Serial.print("Erreur readMeasurement(): ");
            errorToString(error, errorMessage, sizeof errorMessage);
            Serial.println(errorMessage);
        } else {
            // Affichage des résultats
            Serial.println("-----------------------------");
            Serial.print("CO2 : ");
            Serial.print(co2Concentration);
            Serial.println(" ppm");

            Serial.print("Temp: ");
            Serial.print(temperature);
            Serial.println(" °C");

            Serial.print("Hum : ");
            Serial.print(relativeHumidity);
            Serial.println(" %RH");
        }
    }
}
