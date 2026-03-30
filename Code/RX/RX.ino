/**
 * LoRa Receveur (RX uniquement)
 */

#define HELTEC_POWER_BUTTON
#define HELTEC_NO_DISPLAY

#include <heltec_unofficial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "BELL256";
const char* password = "838A1C779DF3";

#define FREQUENCY           915.0
#define BANDWIDTH           125.0
#define SPREADING_FACTOR    9

String rxdata;
volatile bool rxFlag = false;

void setup() {
  heltec_setup();

  // WiFi setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  // LoRa setup
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

      // create JSON body
      StaticJsonDocument<200> doc;

      DeserializationError error = deserializeJson(doc, rxdata);
      if (error) {
        Serial.print("Erreur de décodage : ");
        Serial.println(error.c_str());
      } else {
        // JSON -> String
        String requestBody;
        serializeJson(doc, requestBody);

        // send
        HTTPClient http;
        http.begin("http://192.168.2.200/api/data");      // RPI5
        http.addHeader("Content-Type", "application/json");    // content-type

        int httpResponseCode = http.POST(requestBody);
        if (httpResponseCode > 0) {
          Serial.print("Response: ");
          Serial.println(httpResponseCode);
        }

        // close communication
        http.end();
      }
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
