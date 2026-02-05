// wifi_manager.cpp - Complete version
#include "wifi_manager.h"

// HTML pages stored in PROGMEM
static const char INDEX_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Clock Setup</title>
<style>
body {
  font-family: Arial, sans-serif;
  text-align: center;
  margin: 40px;
  background: #f0f0f0;
}
.container {
  max-width: 400px;
  margin: 0 auto;
  background: white;
  padding: 30px;
  border-radius: 10px;
  box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}
input {
  width: 100%;
  padding: 12px;
  margin: 10px 0;
  border: 1px solid #ddd;
  border-radius: 5px;
  box-sizing: border-box;
}
button {
  background: #4CAF50;
  color: white;
  padding: 12px;
  border: none;
  border-radius: 5px;
  width: 100%;
  margin: 5px 0;
  cursor: pointer;
  font-size: 16px;
}
button:hover {
  background: #45a049;
}
.info-box {
  background: #e8f4fd;
  padding: 15px;
  border-radius: 5px;
  margin: 20px 0;
  border-left: 4px solid #2196F3;
}
</style>
</head>
<body>
<div class="container">
  <h2>🕒 LED Clock Setup</h2>
  
  <div class="info-box">
    <p><strong>Connect to WiFi:</strong> LED-Matrix-Clock</p>
    <p><strong>Then open:</strong> 192.168.4.1</p>
  </div>
  
  <form action="/save" method="POST">
    <input type="text" name="ssid" placeholder="Your WiFi Name" required>
    <input type="password" name="password" placeholder="Your WiFi Password" required>
    <button type="submit">Save & Connect</button>
  </form>
  
  <p style="font-size: 12px; color: #666; margin-top: 20px;">
    After saving, the clock will restart and connect to your WiFi.
  </p>
</div>
</body>
</html>
)rawliteral";

static const char SUCCESS_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Success!</title>
<style>
body {
  font-family: Arial, sans-serif;
  text-align: center;
  margin: 40px;
  background: #f0f0f0;
}
.container {
  max-width: 400px;
  margin: 0 auto;
  background: white;
  padding: 30px;
  border-radius: 10px;
  box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}
.success {
  background: #d4edda;
  color: #155724;
  padding: 20px;
  border-radius: 5px;
  margin: 20px 0;
  border: 1px solid #c3e6cb;
}
.info {
  background: #d1ecf1;
  color: #0c5460;
  padding: 15px;
  border-radius: 5px;
  margin: 15px 0;
  border: 1px solid #bee5eb;
}
</style>
</head>
<body>
<div class="container">
  <h2>✅ Success!</h2>
  
  <div class="success">
    <strong>WiFi configured successfully!</strong><br>
    The clock will now restart and connect.
  </div>
  
  <div class="info">
    <p><strong>Next Steps:</strong></p>
    <p>1. Clock will connect to: <strong>%SSID%</strong></p>
    <p>2. Check your router for the clock's IP address</p>
    <p>3. Access web interface at: http://[clock-ip]:8080</p>
  </div>
  
  <p style="color: #666; margin-top: 20px;">
    This page will close in 5 seconds...
  </p>
</div>
<script>
setTimeout(function() {
  window.close();
}, 5000);
</script>
</body>
</html>
)rawliteral";

WiFiManager::WiFiManager() : configMode(false), serverRunning(false) {
    server = new ESP8266WebServer(80);
    indexPage = INDEX_PAGE;
    successPage = SUCCESS_PAGE;
}

WiFiManager::~WiFiManager() {
    if (server) {
        server->stop();
        delete server;
    }
}

bool WiFiManager::begin() {
    EEPROM.begin(sizeof(WiFiConfig));
    
    WiFiConfig config;
    loadConfig(config);
    
    Serial.print("Stored SSID: ");
    Serial.println(config.ssid);
    Serial.print("Configured: ");
    Serial.println(config.configured);
    
    if (!config.configured || strlen(config.ssid) == 0) {
        Serial.println("No WiFi config found. Starting AP mode...");
        startAPMode();
        return false;
    }
    
    // Try to connect to stored WiFi
    Serial.print("Attempting to connect to: ");
    Serial.println(config.ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        configMode = false;
        return true;
    } else {
        Serial.println("\nFailed to connect to WiFi. Starting AP mode...");
        WiFi.disconnect(true);
        delay(100);
        startAPMode();
        return false;
    }
}

void WiFiManager::startAPMode() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP("LED-Matrix-Clock", "")) {
        Serial.println("Failed to start AP!");
        return;
    }
    
    delay(100); // Give AP time to start
    
    Serial.println("AP Mode Started");
    Serial.print("SSID: LED-Matrix-Clock");
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
    
    configMode = true;
    serverRunning = true;
    
    // Setup server routes
    server->on("/", HTTP_GET, [this]() {
        server->send_P(200, "text/html", indexPage);
    });
    
    server->on("/save", HTTP_POST, [this]() {
        if (server->hasArg("ssid") && server->hasArg("password")) {
            String ssid = server->arg("ssid");
            String password = server->arg("password");
            
            Serial.print("Saving WiFi config - SSID: ");
            Serial.println(ssid);
            
            WiFiConfig config;
            strlcpy(config.ssid, ssid.c_str(), sizeof(config.ssid));
            strlcpy(config.password, password.c_str(), sizeof(config.password));
            config.configured = true;
            
            if (saveConfig(config)) {
                Serial.println("Config saved to EEPROM");
                
                // Show success page
                String page = FPSTR(successPage);
                page.replace("%SSID%", ssid);
                server->send(200, "text/html", page);
                
                // Give time for page to display
                delay(3000);
                
                // Stop server and restart
                stopConfigPortal();
                configMode = false;
                
                Serial.println("Restarting with new WiFi settings...");
                delay(1000);
                ESP.restart();
            } else {
                server->send(500, "text/plain", "Failed to save configuration");
            }
        } else {
            server->send(400, "text/plain", "Bad Request: Missing SSID or Password");
        }
    });
    
    server->onNotFound([this]() {
        server->send(404, "text/plain", "404: Not Found");
    });
    
    server->begin();
    Serial.println("HTTP server started");
}

void WiFiManager::handleClient() {
    if (serverRunning && server) {
        server->handleClient();
    }
}

bool WiFiManager::isConfigMode() {
    return configMode;
}

bool WiFiManager::loadConfig(WiFiConfig& config) {
    EEPROM.get(0, config);
    
    // Debug print
    Serial.print("Loaded from EEPROM - SSID: '");
    Serial.print(config.ssid);
    Serial.print("', Configured: ");
    Serial.println(config.configured);
    
    // Validate config
    if (strlen(config.ssid) == 0 || strcmp(config.ssid, "") == 0) {
        config.configured = false;
    }
    
    return true;
}

bool WiFiManager::saveConfig(const WiFiConfig& config) {
    EEPROM.put(0, config);
    bool result = EEPROM.commit();
    return result;
}

void WiFiManager::stopConfigPortal() {
    if (serverRunning && server) {
        server->stop();
        serverRunning = false;
    }
    
    // Turn off AP mode
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    
    Serial.println("Configuration portal stopped");
}

String WiFiManager::getUptime() {
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%lud %02lu:%02lu:%02lu", days, hours, minutes, seconds);
    return String(buffer);
}