#include "wifi_manager.h"
#include <Arduino.h>
#include "display_utils.h"

WiFiClockManager::WiFiClockManager() {
  deviceName = "ESP SmartClock";
}
// function to connect to wifi
void WiFiClockManager::begin(MD_Parola &display) {
  Serial.println("Initializing WiFi Manager...");
  
  // Store display reference
  this->display = &display;
  
  // Configure WiFiManager
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConnectTimeout(30);
  wifiManager.setDebugOutput(true);
  
  // Show startup message
  // showScrollingMessageContinuous("Clock Booting", 1000);
  
  // Try to connect with saved credentials
  if (WiFi.SSID() != "") {
    // showDisplayMessage("TRYING", "WIFI", 0);
    // showScrollingMessage("Connecting");
    
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
      showScrollingMessage("Welcome", 2000);
      return;
    }
  }
  
  // Start configuration portal
  startConfigPortal();
}

// function form connecting to wifi and diplay updates
void WiFiClockManager::startConfigPortal() {
  Serial.println("Starting configuration portal...");
  
  // Clear display
  display->displayClear(0);
  display->displayClear(1);
  
  // Show setup instructions
  // display->displayZoneText(0, "SETUP MODE", PA_CENTER, 50, 0, PA_NO_EFFECT);
  // display->displayZoneText(1, "ACTIVE", PA_CENTER, 50, 0, PA_NO_EFFECT);
  // display->displayAnimate();
  showScrollingMessage("SetUp Device", 2000);
  // delay(2000);
  
  // Show AP name
  String displayName = deviceName;
  if (displayName.length() > 8) displayName = displayName.substring(0, 8);
  
  // display->displayZoneText(0, "CONNECT TO", PA_CENTER, 50, 0, PA_NO_EFFECT);
  // display->displayZoneText(1, "THIS AP", PA_CENTER, 50, 0, PA_NO_EFFECT);
  // display->displayAnimate();
  showScrollingMessage("Connect", 2000);
  // delay(2000);
  
  
  // Start configuration portal
  if (!wifiManager.startConfigPortal(deviceName.c_str())) {
    Serial.println("Failed to connect and hit timeout");
    
    // display->displayZoneText(0, "TIMEOUT", PA_CENTER, 50, 0, PA_NO_EFFECT);
    // display->displayZoneText(1, "RETRYING", PA_CENTER, 50, 0, PA_NO_EFFECT);
    // display->displayAnimate();
    // delay(2000);
    showScrollingMessageContinuous("Failed retry some times later",6000);
    
    // Retry
    startConfigPortal();
  }
  
  wifiConfigured = true;
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