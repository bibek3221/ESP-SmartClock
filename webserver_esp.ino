#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#define EEPROM_SIZE 64
#define CITY_ADDR 0

const char* ssid = "nope";
const char* password = "nope";
const char* apikey = "nope";

AsyncWebServer server(80);

/* ---------- WEATHER VARIABLES ---------- */
float temperature = 0;
float humidityVal = 0;
float feelsLike = 0;
float windSpeed = 0;
String cityName = "";

/* ---------- FLAGS ---------- */
bool citySelected = false;
unsigned long lastFetch = 0;
unsigned long startTime = 0;

/* ---------- HTML ---------- */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Weather</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial;background:#1a1a1a;color:#fff;padding:20px;text-align:center}
h2{margin:20px 0}
.box{background:#2a2a2a;padding:20px;border-radius:8px;max-width:400px;margin:20px auto}
input{width:100%;padding:12px;border:none;border-radius:5px;margin:10px 0;font-size:16px}
button{width:100%;padding:12px;border:none;border-radius:5px;background:#4CAF50;color:#fff;font-size:16px;cursor:pointer;margin:5px 0}
button:hover{background:#45a049}
.btn-change{background:#2196F3}
.btn-change:hover{background:#0b7dda}
p{margin:15px 0;font-size:18px}
.label{color:#888;font-size:14px}
.info{background:#1a1a1a;padding:15px;border-radius:8px;margin:20px auto;max-width:400px;text-align:left}
.info-item{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #333}
.info-item:last-child{border-bottom:none}
.info-label{color:#888;font-size:14px}
.info-value{color:#fff;font-size:14px;font-weight:bold}
</style>
</head>
<body>

<h2 id="title">Weather Station</h2>

<div id="setup" style="display:none">
  <div class="box">
    <input id="city" placeholder="Enter city name">
    <button onclick="saveCity()">Save</button>
  </div>
</div>

<div id="weather" style="display:none">
  <div class="box">
    <p><span class="label">Temperature</span><br><span id="t">--</span>°C</p>
    <p><span class="label">Feels Like</span><br><span id="f">--</span>°C</p>
    <p><span class="label">Humidity</span><br><span id="h">--</span>%</p>
    <p><span class="label">Wind Speed</span><br><span id="w">--</span> kph</p>
    <button class="btn-change" onclick="changeCity()">Change City</button>
  </div>

  <div class="info">
    <div class="info-item">
      <span class="info-label">WiFi SSID</span>
      <span class="info-value" id="wifi">--</span>
    </div>
    <div class="info-item">
      <span class="info-label">Signal Strength</span>
      <span class="info-value" id="signal">--</span>
    </div>
    <div class="info-item">
      <span class="info-label">IP Address</span>
      <span class="info-value" id="ip">--</span>
    </div>
    <div class="info-item">
      <span class="info-label">Uptime</span>
      <span class="info-value" id="uptime">--</span>
    </div>
    <div class="info-item">
      <span class="info-label">Free Heap</span>
      <span class="info-value" id="heap">--</span>
    </div>
    <div class="info-item">
      <span class="info-label">Chip ID</span>
      <span class="info-value" id="chip">--</span>
    </div>
  </div>
</div>

<script>
let isChanging = false;

async function load(){
  if(isChanging) return;
  
  const r = await fetch('/weather');
  const j = await r.json();

  if(!j.citySelected){
    document.getElementById('setup').style.display='block';
    document.getElementById('weather').style.display='none';
    document.getElementById('title').innerText='Select City';
  }else{
    document.getElementById('setup').style.display='none';
    document.getElementById('weather').style.display='block';
    document.getElementById('title').innerText=j.city;
    document.getElementById('t').innerText=j.temperature;
    document.getElementById('f').innerText=j.feelsLike;
    document.getElementById('h').innerText=j.humidity;
    document.getElementById('w').innerText=j.windSpeed;
    
    // System info
    document.getElementById('wifi').innerText=j.wifi;
    document.getElementById('signal').innerText=j.signal + ' dBm';
    document.getElementById('ip').innerText=j.ip;
    document.getElementById('uptime').innerText=j.uptime;
    document.getElementById('heap').innerText=j.freeHeap + ' KB';
    document.getElementById('chip').innerText=j.chipId;
  }
}

async function saveCity(){
  const c=document.getElementById('city').value.trim();
  if(!c) return alert('Please enter a city name');
  
  await fetch('/setCity?name='+encodeURIComponent(c));
  document.getElementById('city').value='';
  
  // Wait for weather to be fetched, then show it
  isChanging = false;
  setTimeout(async () => {
    await load();
  }, 2000);
}

function changeCity(){
  isChanging = true;
  document.getElementById('setup').style.display='block';
  document.getElementById('weather').style.display='none';
  document.getElementById('title').innerText='Select City';
  document.getElementById('city').focus();
}

load();
setInterval(load,10000);
</script>

</body>
</html>
)rawliteral";

/* ---------- EEPROM ---------- */
void saveCity(String city){
  EEPROM.write(CITY_ADDR, city.length());
  for(int i=0;i<city.length();i++)
    EEPROM.write(CITY_ADDR+1+i, city[i]);
  EEPROM.commit();
}

String loadCity(){
  int len = EEPROM.read(CITY_ADDR);
  if(len <= 0 || len > 30) return "";
  char buf[32];
  for(int i=0;i<len;i++)
    buf[i] = EEPROM.read(CITY_ADDR+1+i);
  buf[len]='\0';
  return String(buf);
}

/* ---------- WEATHER FETCH ---------- */
void fetchWeather(){
  if(cityName=="") return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String url = "https://api.weatherapi.com/v1/current.json?key=" +
               String(apikey) + "&q=" + cityName + "&aqi=no";

  if(https.begin(client, url)){
    int code = https.GET();
    if(code > 0){
      String payload = https.getString();
      
      int tempIdx = payload.indexOf("\"temp_c\":");
      int humIdx = payload.indexOf("\"humidity\":");
      int feelsIdx = payload.indexOf("\"feelslike_c\":");
      int windIdx = payload.indexOf("\"wind_kph\":");
      
      if(tempIdx > 0){
        temperature = payload.substring(tempIdx + 9, payload.indexOf(",", tempIdx)).toFloat();
      }
      if(humIdx > 0){
        humidityVal = payload.substring(humIdx + 11, payload.indexOf(",", humIdx)).toFloat();
      }
      if(feelsIdx > 0){
        feelsLike = payload.substring(feelsIdx + 14, payload.indexOf(",", feelsIdx)).toFloat();
      }
      if(windIdx > 0){
        windSpeed = payload.substring(windIdx + 11, payload.indexOf(",", windIdx)).toFloat();
      }
    }
    https.end();
  }
}

/* ---------- HELPER FUNCTIONS ---------- */
String getUptime(){
  unsigned long uptime = (millis() - startTime) / 1000;
  unsigned long days = uptime / 86400;
  uptime %= 86400;
  unsigned long hours = uptime / 3600;
  uptime %= 3600;
  unsigned long minutes = uptime / 60;
  unsigned long seconds = uptime % 60;
  
  String result = "";
  if(days > 0) result += String(days) + "d ";
  if(hours > 0 || days > 0) result += String(hours) + "h ";
  if(minutes > 0 || hours > 0 || days > 0) result += String(minutes) + "m ";
  result += String(seconds) + "s";
  return result;
}

/* ---------- SETUP ---------- */
void setup(){
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  startTime = millis();

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(500);

  cityName = loadCity();
  citySelected = cityName != "";

  server.on("/",HTTP_GET,[](AsyncWebServerRequest *r){
    r->send_P(200,"text/html",index_html);
  });

  server.on("/setCity",HTTP_GET,[](AsyncWebServerRequest *r){
    if(r->hasParam("name")){
      cityName = r->getParam("name")->value();
      saveCity(cityName);
      citySelected = true;
      fetchWeather(); // Fetch immediately after setting city
    }
    r->send(200,"text/plain","OK");
  });

  server.on("/weather",HTTP_GET,[](AsyncWebServerRequest *r){
    String json = "{";
    json += "\"citySelected\":" + String(citySelected ? "true" : "false") + ",";
    json += "\"city\":\"" + cityName + "\",";
    json += "\"temperature\":" + String(temperature) + ",";
    json += "\"humidity\":" + String(humidityVal) + ",";
    json += "\"feelsLike\":" + String(feelsLike) + ",";
    json += "\"windSpeed\":" + String(windSpeed) + ",";
    
    // System information
    json += "\"wifi\":\"" + String(ssid) + "\",";
    json += "\"signal\":" + String(WiFi.RSSI()) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"uptime\":\"" + getUptime() + "\",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap() / 1024.0, 2) + ",";
    json += "\"chipId\":\"0x" + String(ESP.getChipId(), HEX) + "\"";
    
    json += "}";
    r->send(200,"application/json",json);
  });

  server.begin();
  
  if(citySelected){
    fetchWeather(); // Fetch on startup if city is already set
  }
}

/* ---------- LOOP ---------- */
void loop(){
  if(citySelected && millis()-lastFetch>10000){
    lastFetch=millis();
    fetchWeather();
  }
}