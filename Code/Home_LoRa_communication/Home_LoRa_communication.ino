/*
 * Heltec Wireless Stick Lite V3 - Récepteur LoRa / Passerelle Wi-Fi
 */

#include "LoRaWan_APP.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>

// --- Config Wi-Fi ---
const char* ssid = "BELL256";
const char* password = "83A1C779DF3";

// Remplace par l'adresse IP locale de ton ordi (ex: 192.168.1.15)
const char* serverName = "http://192.168.2.141:5000/api/data";

// --- LoRa config ---
#define RF_FREQUENCY                  915000000 // Hz
#define TX_OUTPUT_POWER               14        
#define LORA_BANDWIDTH                0         
#define LORA_SPREADING_FACTOR         9         
#define LORA_CODINGRATE               1         
#define LORA_PREAMBLE_LENGTH          8         
#define LORA_SYMBOL_TIMEOUT           0         
#define LORA_FIX_LENGTH_PAYLOAD_ON    false
#define LORA_IQ_INVERSION_ON          false

#define LED_PIN 35

static RadioEvents_t RadioEvents;

// Variables pour transférer la donnée entre la radio et le Wi-Fi
bool nouvelleDonnee = false;
char jsonRecu[100];

// Déclaration des fonctions radio
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }

    // Initialisation Wi-Fi
    Serial.print("Connexion au Wi-Fi");
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connecté ! IP: ");
    Serial.println(WiFi.localIP());

    // Initialisation LoRa
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;

    Mcu.begin();
    pinMode(LED_PIN, OUTPUT);

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(
        MODEM_LORA,TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
        true, 0, 0, LORA_IQ_INVERSION_ON, 3000
    );
    
    Serial.println("LoRa en écoute...");
    Radio.Rx(0); // Écoute infinie
}

void loop() {
    Radio.IrqProcess(); // Gère la puce LoRa en arrière-plan

    // Si le drapeau est levé, on envoie au serveur Flask
    if (nouvelleDonnee) {
        nouvelleDonnee = false; // On rebaisse le drapeau

        if(WiFi.status() == WL_CONNECTED){
            HTTPClient http;
            http.begin(serverName); // On pointe vers ton ordi
            http.addHeader("Content-Type", "application/json"); // On prévient que c'est du JSON
            
            // Envoi de la requête POST
            int httpResponseCode = http.POST(jsonRecu);
            
            Serial.print("Code de réponse HTTP : ");
            Serial.println(httpResponseCode);
            
            http.end(); // Libère les ressources
        } else {
            Serial.println("Erreur: Wi-Fi déconnecté.");
        }
        
        // On remet la radio en mode écoute
        Radio.Rx(0);
    }
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    // Copie sécurisée du message
    memcpy(jsonRecu, payload, size);
    jsonRecu[size] = '\0'; 
    
    Serial.print("Reçu par LoRa : ");
    Serial.println(jsonRecu);
    
    // On lève le drapeau pour que le Wi-Fi fasse son travail dans le loop()
    nouvelleDonnee = true; 
}

void OnRxTimeout(void) {
    Serial.println("Erreur : Délai de réception dépassé (Timeout).");
    Radio.Rx(0); 
}
