/*
 * Heltec Wireless Stick Lite V3 - Émetteur LoRa
 * Puce LoRa : SX1262
 * Puce MCU : ESP32-S3
 */

#include "LoRaWan_APP.h"
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <Arduino.h>

// LoRa config
#define RF_FREQUENCY                  915000000 // Hz

#define TX_OUTPUT_POWER               14        // dBm (Puissance)
#define LORA_BANDWIDTH                0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_SPREADING_FACTOR         9         // [SF7..SF12]
#define LORA_CODINGRATE               1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH          8         // Longueur préambule
#define LORA_SYMBOL_TIMEOUT           0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON    false
#define LORA_IQ_INVERSION_ON          false

// define pins
#define LED_PIN 35
#define PIN_SCL 45
#define PIN_SDA 46

// sensor init
SensirionI2cScd4x sensor;
uint16_t serialNumber[3];

// 12h entre chaque envoi de données
#define SLEEP_TIME_SEC 43200

/*
Data acquisition variables
*/

static char errorMessage[64];
static int16_t error;

void PrintUint64(uint64_t& value) {
    Serial.print("0x");
    Serial.print((uint32_t)(value >> 32), HEX);
    Serial.print((uint32_t)(value & 0xFFFFFFFF), HEX);
}

/*
LoRa communication variables
*/

// Buffer pour le message
char txPacket[100];
static RadioEvents_t RadioEvents;

void OnTxDone(void);
void OnTxTimeout(void);

/*
Setup function
*/
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // Initialisation des callbacks radio
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;

    // Heltec startup
    Mcu.begin();
    pinMode(LED_PIN, OUTPUT);

    // SX1262 init
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(
        MODEM_LORA,TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
        true, 0, 0, LORA_IQ_INVERSION_ON, 3000
    );
    Serial.println("Initialisation LoRa V3 terminée.");
    Serial.print("Fréquence : "); Serial.print(RF_FREQUENCY / 1000000.0); Serial.println(" MHz");

    // SCD41 setup
    Wire.begin(PIN_SDA, PIN_SCL);

    // check for errors
    error = sensor.wakeUp();
    if (error != 0) {
        Serial.print("Erreur wakeUp(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    error = sensor.stopPeriodicMeasurement();
    if (error != 0) {
        Serial.print("Erreur stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
    error = sensor.reinit();
    if (error != 0) {
        Serial.print("Erreur reinit(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }
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

    // passes if sensor is found
    Serial.print("Capteur trouvé ! Serial number: ");
    PrintUint64(serialNumber);
    Serial.println();

    Serial.println("Démarrage des mesures...");
}

/*
Main function
*/
void loop() {
    // battery level
    float battery_level = getBatteryVoltage();

    // sensing variables
    bool dataReady = false;
    float temperature = 0.0;
    float relativeHumidity = 0.0;
    uint16_t co2Concentration = 0;

    Serial.println("Lancement d'une mesure Single Shot (attente de 5s)...");
    
    // Unique measure
    error = sensor.measureSingleShot(); 
    if (error != 0) {
        Serial.print("Erreur measureSingleShot(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    }

    // 5s delay for acquisition time
    delay(5000); 

    error = sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
    
    if (error != 0) {
        Serial.print("Erreur readMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
    } else {
        Serial.println("-----------------------------");
        Serial.print("CO2 : "); Serial.print(co2Concentration); Serial.println(" ppm");
        Serial.print("Temp: "); Serial.print(temperature); Serial.println(" °C");
        Serial.print("Hum : "); Serial.print(relativeHumidity); Serial.println(" %RH");
    }

    // eencode JSON message
    sprintf(txPacket, "{\"v\":%.2f, \"t\":%.2f, \"h\":%.2f, \"c\":%d}", battery_level, temperature, relativeHumidity, co2Concentration);
    Serial.print("Envoi du paquet : ");
    Serial.println(txPacket);

    digitalWrite(LED_PIN, HIGH); 

    // LoRa send
    Radio.Send((uint8_t *)txPacket, strlen(txPacket));
    delay(100);

    // setup deep sleep
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * 1000000ULL);
    digitalWrite(Vext, HIGH);
    esp_deep_sleep_start();
}

// callbacks
void OnTxDone(void)
{
    Serial.println("LoRa TX terminé avec succès !");
    digitalWrite(LED_PIN, LOW); // Éteindre la LED
    Radio.Sleep(); // Mettre la radio en veille pour économiser la batterie
}

void OnTxTimeout(void)
{
    Serial.println("Erreur : LoRa TX Timeout");
    digitalWrite(LED_PIN, LOW);
    Radio.Sleep();
}

// battery readings
float getBatteryVoltage() {
    // Vext control (active low)
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(10);

    // ADC reading on GPIO 1 -> 12 bits [V]
    float voltage = analogRead(1) * (3.3 / 4095.0) * 2.0;
    digitalWrite(Vext, HIGH); 

    return voltage;
}
