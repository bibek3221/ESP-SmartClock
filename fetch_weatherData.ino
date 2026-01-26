#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "Bibek";
const char* password = "Bankim@2004--indranil";
const char* apikey = "4e8068c62ffc4034a07163742262401"

void setup() {
  Serial.begin(9600);
  delay(1000);

  WiFi.begin(ssid, password);
  // Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Serial.println("\nWiFi connected");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();   // disable SSL check

    HTTPClient https;

    String url =
      `https://api.weatherapi.com/v1/current.json?`
      `key=${apikey}&q=Kolkata&aqi=no`;

    // Serial.println("\nRequesting Weather API...");

    if (https.begin(client, url)) {

      // 🔥 REQUIRED HEADERS
      https.addHeader("User-Agent", "ESP8266");
      https.addHeader("Accept", "application/json");

      int httpCode = https.GET();

      // Serial.print("HTTP Code: ");
      // Serial.println(httpCode);

      if (httpCode > 0) {
        String payload = https.getString();
        // Serial.println(payload);
    
      // 🔥 Parse JSON
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        // serializeJsonPretty(doc, Serial);


        if (error) {
          Serial.print("JSON parse failed: ");
          Serial.println(error.c_str());
          return;
        }
        const char* city = doc["location"]["name"];
        int humidity = doc["current"]["humidity"];
        float temp_c = doc["current"]["temp_c"];
        Serial.print("Temp : ");
        Serial.println(temp_c);
        Serial.print("Humidity : ");
        Serial.println(humidity);
        Serial.print("City :");
        Serial.println(city);

      }

      https.end();
    }
  }

  delay(10000);
}
