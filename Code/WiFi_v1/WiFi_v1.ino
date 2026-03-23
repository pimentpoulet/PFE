#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "BELL256";
const char* password = "838A1C779DF3";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP()); // This is your board's IP address
}

void loop() {
  if (WiFi.status() != WL_CONNECTED){
    Serial.print("Reconnecting to WiFi...");
  } else {
    // create JSON body
    StaticJsonDocument<200> doc;

    // fill JSON
    doc["v"] = 3.7;
    doc["t"] = 24.5;
    doc["h"] = 46.8;
    doc["c"] = 504;

    // JSON -> String
    String requestBody;
    serializeJson(doc, requestBody);

    // send
    HTTPClient http;
    http.begin("http://192.168.2.140:5000/api/data");      // computer
    http.addHeader("Content-Type", "application/json");    // content-type

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      Serial.print("Response: ");
      Serial.println(httpResponseCode);
    }

    // close communication
    http.end();

    // wait 5s
    delay(5000);
  }
}
