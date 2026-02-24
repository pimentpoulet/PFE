/*
 * Heltec Wireless Stick Lite V3 - Émetteur LoRa
 * Puce LoRa : SX1262
 * Puce MCU : ESP32-S3
 */

#include "LoRaWan_APP.h"
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

// 12h entre chaque réception de données
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
char TxPacket[100];
static RadioEvents_t RadioEvents;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);

/*
Setup function
*/
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // Initialisation des callbacks radio
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;

    // Heltec startup
    Mcu.begin();
    pinMode(LED_PIN, OUTPUT);

    // SX1262 init
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(
        MODEM_LORA,TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
        true, 0, 0, LORA_IQ_INVERSION_ON, 3000
    );
    Serial.println("Initialisation LoRa V3 terminée.");
    Serial.print("Fréquence : "); Serial.print(RF_FREQUENCY / 1000000.0); Serial.println(" MHz");

    Radio.Rx(0);
}

//
void loop() {
    Radio.IrqProcess();
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    char rxPacket[100];
    memcpy(rxPacket, payload, size);
    rxPacket[size] = '\0'; // Caractère de fin de chaîne obligatoire en C++

    float battery_level, temperature, relativeHumidity;
    int co2Concentration;

    // decode JSON message
    sscanf(rxPacket, "{\"v\":%f, \"t\":%f, \"h\":%f, \"c\":%d}", &battery_level, &temperature, &relativeHumidity, &co2Concentration);

    // 4. On affiche le message reçu et la force du signal (RSSI)
    Serial.print("Réception (RSSI "); Serial.print(rssi); Serial.print(") : ");
    Serial.println(rxPacket);
}

void OnRxTimeout(void) {
    Serial.println("Erreur : Délai de réception dépassé (Timeout).");
    // On relance l'écoute infinie au cas où
    Radio.Rx(0); 
}
