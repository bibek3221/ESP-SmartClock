#include "wifi_manager.h"
#include <Arduino.h>

WiFiClockManager::WiFiClockManager() {
  deviceName = "MatrixClock-" + String(ESP.getChipId(), HEX);
}

void WiFiClockManager::begin(MD_Parola &display) {
  Serial.println("Initializing WiFi Manager...");
  
  // Store display reference
  this->display = &display;
  
  // Configure WiFiManager
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectTimeout(30);
  wifiManager.setDebugOutput(true);
  
  // Show startup message
  showDisplayMessage("CLOCK", "BOOT", 2000);
  
  // Try to connect with saved credentials
  if (WiFi.SSID() != "") {
    showDisplayMessage("TRYING", "WIFI", 0);
    
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
    
    unsigned long startTime = millis();
    int dots = 0;
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
      Serial.print(".");
      
      // Show connecting animation
      char dotStr[5] = "....";
      for (int i = 0; i < dots; i++) dotStr[i] = '.';
      dots = (dots + 1) % 5;
      
      display.displayZoneText(1, dotStr, PA_CENTER, 50, 0, PA_NO_EFFECT);
      display.displayAnimate();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      wifiConfigured = true;
      Serial.println("\nConnected to saved WiFi!");
      showDisplayMessage("WIFI", "OK", 2000);
      return;
    }
  }
  
  // Start configuration portal
  startConfigPortal();
}

void WiFiClockManager::startConfigPortal() {
  Serial.println("Starting configuration portal...");
  
  // Show AP mode instructions
  showDisplayMessage("CONNECT", "TO AP", 2000);
  
  // Show AP name (truncated for display)
  String displayName = deviceName;
  if (displayName.length() > 8) displayName = displayName.substring(0, 8);
  showDisplayMessage("AP:", displayName.c_str(), 3000);
  
  showDisplayMessage("AP MODE", "ACTIVE", 2000);
  
  // Start configuration portal
  if (!wifiManager.startConfigPortal(deviceName.c_str())) {
    Serial.println("Failed to connect and hit timeout");
    showDisplayMessage("TIMEOUT", "RETRY", 2000);
    delay(5000);
    startConfigPortal();
  }
  
  wifiConfigured = true;
  Serial.println("\nWiFi configured successfully!");
}

void WiFiClockManager::showDisplayMessage(const char* line1, const char* line2, int duration) {
  if (display) {
    display->displayZoneText(0, line1, PA_CENTER, 50, duration, PA_NO_EFFECT);
    display->displayZoneText(1, line2, PA_CENTER, 50, duration, PA_NO_EFFECT);
    display->displayAnimate();
    if (duration > 0) delay(duration);
  }
}

bool WiFiClockManager::isConfigured() {
  return wifiConfigured && (WiFi.status() == WL_CONNECTED);
}

void WiFiClockManager::resetSettings() {
  wifiManager.resetSettings();
  wifiConfigured = false;
  Serial.println("WiFi settings reset");
}

String WiFiClockManager::getDeviceName() {
  return deviceName;
}

String WiFiClockManager::getIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
} 