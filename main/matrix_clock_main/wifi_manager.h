#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <MD_Parola.h>

class WiFiClockManager {
  private:
    WiFiManager wifiManager;
    MD_Parola* display = nullptr;
    bool wifiConfigured = false;
    String deviceName;
    
    // void showDisplayMessage(const char* line1, const char* line2, int duration = 2000);
    void startConfigPortal();
    
  public:
    WiFiClockManager();
    
    void begin(MD_Parola &display);
    bool isConfigured();
    void resetSettings();
    String getDeviceName();
    String getIPAddress();
};

#endif