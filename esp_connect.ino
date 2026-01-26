

// ESP connect code with WiFi manager and web server



#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

/* ================= CONFIG ================= */
#define AP_SSID        "ESP8266-Setup"
#define AP_PASSWORD    "12345678"
#define EEPROM_SIZE    96
#define SSID_ADDR      0
#define PASS_ADDR      32
#define WIFI_TIMEOUT   15000
/* ========================================== */

AsyncWebServer server(80);
DNSServer dnsServer;
bool isAPMode = true;

/* ================= HTML PAGES ================= */
const char setup_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>WiFi Setup</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;}
body{font-family:Arial;background:#0f172a;color:#fff;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px;}
.card{background:#1e293b;padding:30px;border-radius:16px;width:100%;max-width:400px;box-shadow:0 8px 32px rgba(0,0,0,0.3);}
h2{text-align:center;margin-bottom:24px;color:#22c55e;}
.form-group{margin-bottom:20px;}
label{display:block;margin-bottom:8px;color:#94a3b8;font-size:14px;}
input{width:100%;padding:12px;border:2px solid #334155;border-radius:8px;background:#0f172a;color:#fff;font-size:16px;}
input:focus{outline:none;border-color:#22c55e;}
button{width:100%;padding:14px;background:#22c55e;color:#fff;border:none;border-radius:8px;font-size:16px;font-weight:bold;cursor:pointer;margin-top:10px;}
button:hover{background:#16a34a;}
button:disabled{background:#64748b;cursor:not-allowed;}
.status{display:none;padding:12px;border-radius:8px;margin-top:20px;text-align:center;}
.status.show{display:block;}
.status.success{background:#22c55e;color:#fff;}
.status.error{background:#ef4444;color:#fff;}
</style>
</head>
<body>
<div class="card">
<h2>🔧 WiFi Setup</h2>
<form id="form">
<div class="form-group">
<label>WiFi Network Name (SSID)</label>
<input type="text" id="ssid" required>
</div>
<div class="form-group">
<label>WiFi Password</label>
<input type="password" id="pass">
</div>
<button type="submit" id="btn">Connect</button>
</form>
<div class="status" id="status"></div>
</div>
<script>
document.getElementById('form').onsubmit = function(e) {
  e.preventDefault();
  var btn = document.getElementById('btn');
  var status = document.getElementById('status');
  var ssid = document.getElementById('ssid').value;
  var pass = document.getElementById('pass').value;
  
  btn.disabled = true;
  btn.textContent = 'Saving...';
  status.className = 'status';
  
  fetch('/save?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass))
    .then(r => r.text())
    .then(data => {
      status.className = 'status success show';
      status.textContent = '✓ Saved! ESP is connecting to WiFi...';
      btn.textContent = '✓ Done';
      setTimeout(function() {
        status.textContent = 'Please connect to your WiFi network and go to the IP shown in serial monitor';
      }, 2000);
    })
    .catch(err => {
      status.className = 'status error show';
      status.textContent = '✗ Failed to save';
      btn.disabled = false;
      btn.textContent = 'Try Again';
    });
};
</script>
</body>
</html>
)rawliteral";

const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP8266 Settings</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;}
body{font-family:Arial;background:#0f172a;color:#fff;padding:20px;}
.container{max-width:600px;margin:0 auto;}
.card{background:#1e293b;padding:24px;border-radius:16px;margin-bottom:20px;box-shadow:0 4px 16px rgba(0,0,0,0.2);}
h1{color:#22c55e;margin-bottom:20px;text-align:center;}
h3{color:#22c55e;margin-bottom:16px;}
.info-row{display:flex;justify-content:space-between;padding:12px 0;border-bottom:1px solid #334155;}
.info-row:last-child{border-bottom:none;}
.label{color:#94a3b8;}
.value{color:#fff;font-weight:bold;}
button{width:100%;padding:14px;background:#ef4444;color:#fff;border:none;border-radius:8px;font-size:16px;font-weight:bold;cursor:pointer;margin-top:16px;}
button:hover{background:#dc2626;}
</style>
</head>
<body>
<div class="container">
<h1>⚙️ ESP8266 Settings</h1>
<div class="card">
<h3>📡 Network Info</h3>
<div class="info-row">
<span class="label">Status</span>
<span class="value" style="color:#22c55e;">✓ Connected</span>
</div>
<div class="info-row">
<span class="label">SSID</span>
<span class="value">%SSID%</span>
</div>
<div class="info-row">
<span class="label">IP Address</span>
<span class="value">%IP%</span>
</div>
<div class="info-row">
<span class="label">Signal Strength</span>
<span class="value">%RSSI% dBm</span>
</div>
<div class="info-row">
<span class="label">MAC Address</span>
<span class="value">%MAC%</span>
</div>
</div>
<div class="card">
<h3>🔧 System Info</h3>
<div class="info-row">
<span class="label">Uptime</span>
<span class="value">%UPTIME%</span>
</div>
<div class="info-row">
<span class="label">Free Memory</span>
<span class="value">%HEAP% KB</span>
</div>
<div class="info-row">
<span class="label">Chip ID</span>
<span class="value">%CHIPID%</span>
</div>
</div>
<div class="card">
<h3>⚠️ Danger Zone</h3>
<button onclick="if(confirm('Reset WiFi? ESP will restart in AP mode.')) location.href='/reset'">🔄 Reset WiFi Settings</button>
</div>
</div>
</body>
</html>
)rawliteral";

/* ================= EEPROM ================= */
void clearEEPROM() {
  Serial.println("[EEPROM] Clearing...");
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("[EEPROM] Cleared");
}

void saveWiFi(String ssid, String pass) {
  Serial.println("[EEPROM] Saving WiFi...");
  for (int i = 0; i < 32; i++) {
    EEPROM.write(SSID_ADDR + i, i < ssid.length() ? ssid[i] : 0);
    EEPROM.write(PASS_ADDR + i, i < pass.length() ? pass[i] : 0);
  }
  EEPROM.commit();
  Serial.println("[EEPROM] Saved: " + ssid);
}

bool loadWiFi(String &ssid, String &pass) {
  char s[32], p[32];
  for (int i = 0; i < 32; i++) {
    s[i] = EEPROM.read(SSID_ADDR + i);
    p[i] = EEPROM.read(PASS_ADDR + i);
  }
  s[31] = 0; p[31] = 0;
  ssid = String(s); pass = String(p);
  ssid.trim(); pass.trim();
  return ssid.length() > 0;
}

/* ================= WIFI ================= */
bool connectWiFi() {
  String ssid, pass;
  if (!loadWiFi(ssid, pass)) return false;
  
  Serial.println("[WiFi] Connecting to: " + ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WiFi] Connected!");
    Serial.println("[WiFi] IP: " + WiFi.localIP().toString());
    Serial.println("[WiFi] Access settings at: http://" + WiFi.localIP().toString());
    return true;
  }
  
  Serial.println("[WiFi] Failed");
  return false;
}

/* ================= WEB SERVER ================= */
void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (isAPMode) {
      req->send_P(200, "text/html", setup_html);
    } else {
      String html = String(settings_html);
      html.replace("%SSID%", WiFi.SSID());
      html.replace("%IP%", WiFi.localIP().toString());
      html.replace("%RSSI%", String(WiFi.RSSI()));
      html.replace("%MAC%", WiFi.macAddress());
      html.replace("%UPTIME%", String(millis()/1000) + "s");
      html.replace("%HEAP%", String(ESP.getFreeHeap()/1024));
      html.replace("%CHIPID%", String(ESP.getChipId(), HEX));
      req->send(200, "text/html", html);
    }
  });
  
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *req) {
    String ssid = req->arg("ssid");
    String pass = req->arg("pass");
    
    Serial.println("[Server] Received:");
    Serial.println("  SSID: " + ssid);
Serial.println("  Pass: " + String(pass.length() > 0 ? "***" : "(empty)"));    
    saveWiFi(ssid, pass);
    req->send(200, "text/plain", "OK");
    
    Serial.println("[System] Restarting in 3 seconds...");
    delay(3000);
    ESP.restart();
  });
  
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", "<h2 style='text-align:center;color:#22c55e;margin-top:50px;'>Resetting...</h2>");
    Serial.println("[System] Reset requested");
    clearEEPROM();
    delay(2000);
    ESP.restart();
  });
  
  server.onNotFound([](AsyncWebServerRequest *req) {
    req->redirect("/");
  });
  
  server.begin();
  Serial.println("[Server] Started");
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n========== ESP8266 WiFi Manager ==========");
  
  EEPROM.begin(EEPROM_SIZE);
  
  // Clear EEPROM on first boot (comment out after first use)
  // clearEEPROM();
  
  if (connectWiFi()) {
    isAPMode = false;
    Serial.println("[Mode] Station Mode");
    setupServer();
  } else {
    Serial.println("[Mode] AP Mode");
    Serial.println("[AP] Starting...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    delay(100);
    Serial.println("[AP] SSID: " + String(AP_SSID));
    Serial.println("[AP] Password: " + String(AP_PASSWORD));
    Serial.println("[AP] IP: " + WiFi.softAPIP().toString());
    Serial.println("[AP] Connect and open: http://" + WiFi.softAPIP().toString());
    
    dnsServer.start(53, "*", WiFi.softAPIP());
    setupServer();
  }
  
  Serial.println("========== Ready ==========\n");
}

/* ================= LOOP ================= */
void loop() {
  if (isAPMode) {
    dnsServer.processNextRequest();
  }
}