#include <Arduino.h>

float Lat = 0.0;
float Lon = 0.0;

void setup() {
  Serial.begin(115200); // Vitesse de communication USB
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n'); // Lit la ligne envoyée par l'ordi
    
    // On cherche "LAT:" et "LON:" dans la chaîne
    if (data.startsWith("LAT:")) {
      int commaIndex = data.indexOf(",LON:");
      if (commaIndex != -1) {
        Lat = data.substring(4, commaIndex).toFloat();
        Lon = data.substring(commaIndex + 5).toFloat();
        
        Serial.print("Coordonnées reçues sur le Heltec : ");
        Serial.print(Lat, 6);
        Serial.print(", ");
        Serial.println(Lon, 6);
        Serial.print("ACK:RECU_OK");
      }
    }
  }
}
