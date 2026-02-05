// wifi_manager.h - Complete version
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Configuration structure stored in EEPROM
struct WiFiConfig {
    char ssid[32];
    char password[64];
    bool configured;
};

class WiFiManager {
private:
    ESP8266WebServer* server;
    bool configMode;
    bool serverRunning;
    
    // HTML pages (progmem to save RAM)
    const char* indexPage;
    const char* successPage;
    
    void startAPMode();
    
public:
    WiFiManager();
    ~WiFiManager();
    
    bool begin();
    void handleClient();
    bool isConfigMode();
    bool loadConfig(WiFiConfig& config);
    bool saveConfig(const WiFiConfig& config);
    void stopConfigPortal();
    
    String getUptime();
};

#endif