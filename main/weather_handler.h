#ifndef WEATHER_HANDLER_H
#define WEATHER_HANDLER_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

class WeatherHandler {
private:
  // Weather data
  float temperature;
  float feelsLike;
  int humidity;
  bool dataAvailable;
  unsigned long lastUpdate;
  
  // Display state
  int displayState;  // 0=temp, 1=feels like, 2=humidity
  unsigned long lastStateChange;
  
  // Buffers for display
  char line1[20];
  char line2[20];
  
  // Constants
  static const unsigned long UPDATE_INTERVAL = 1800000;  // 30 minutes
  static const unsigned long STATE_DURATION = 3000;      // 3 seconds per state
  
  // API URL
  char apiUrl[200];
  
  void buildApiUrl() {
    snprintf(apiUrl, sizeof(apiUrl), 
             "%s?key=%s&q=%s", 
             WEATHER_API_URL, 
             WEATHER_API_KEY, 
             WEATHER_LOCATION);
  }
  
public:
  WeatherHandler() : temperature(0), feelsLike(0), humidity(0), 
                     dataAvailable(false), lastUpdate(0),
                     displayState(0), lastStateChange(0) {
    line1[0] = '\0';
    line2[0] = '\0';
  }
  
  void begin() {
    buildApiUrl();
    fetchData();
  }
  
  void update(unsigned long currentMillis) {
    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
      fetchData();
    }
  }
  
  bool fetchData() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("Fetching weather data...");
    http.begin(client, apiUrl);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        temperature = doc["current"]["temp_c"];
        feelsLike = doc["current"]["feelslike_c"];
        humidity = doc["current"]["humidity"];
        dataAvailable = true;
        lastUpdate = millis();
        
        Serial.printf("Weather: %.1fC (feels %.1fC), %d%% humidity\n", 
                     temperature, feelsLike, humidity);
      } else {
        Serial.println("Failed to parse weather JSON");
      }
    } else {
      Serial.printf("HTTP error: %d\n", httpCode);
    }
    
    http.end();
    return dataAvailable;
  }
  
  void updateDisplayState(unsigned long currentMillis) {
    if (currentMillis - lastStateChange >= STATE_DURATION) {
      displayState = (displayState + 1) % 3;
      lastStateChange = currentMillis;
      updateDisplayText();
    }
  }
  
  void resetDisplayState(unsigned long currentMillis) {
    displayState = 0;
    lastStateChange = currentMillis;
    updateDisplayText();
  }
  
  const char* getLine1() { return line1; }
  const char* getLine2() { return line2; }
  bool isAvailable() { return dataAvailable; }
  
private:
  void updateDisplayText() {
    switch (displayState) {
      case 0:  // Temperature
        dtostrf(temperature, 4, 1, line1);
        strcat(line1, "C");
        strcpy(line2, "TEMP");
        break;
        
      case 1:  // Feels like
        dtostrf(feelsLike, 4, 1, line1);
        strcat(line1, "C");
        strcpy(line2, "FEEL");
        break;
        
      case 2:  // Humidity
        sprintf(line1, "%d%%", humidity);
        strcpy(line2, "HUMI");
        break;
    }
  }
};

#endif