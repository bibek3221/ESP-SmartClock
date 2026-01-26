

// This shows weather information on a web page served by the ESP8266. 


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

/*---------- HTML PAGE ----------*/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP Weather</title>

<style>
*{margin:0;padding:0;box-sizing:border-box}
body{
  font-family:system-ui,-apple-system,BlinkMacSystemFont;
  background:#f4f6f8;
  color:#222;
  padding:20px;
  text-align:center
}
h2{margin:15px 0;font-weight:600}

.card{
  background:#fff;
  padding:20px;
  border-radius:12px;
  max-width:420px;
  margin:20px auto;
  box-shadow:0 10px 25px rgba(0,0,0,.08)
}

input{
  width:100%;
  padding:12px;
  border:1px solid #ddd;
  border-radius:8px;
  margin:12px 0;
  font-size:16px
}

button{
  width:100%;
  padding:12px;
  border:none;
  border-radius:8px;
  background:#4f46e5;
  color:#fff;
  font-size:16px;
  cursor:pointer
}
button:hover{background:#4338ca}

.value{font-size:26px;font-weight:600;margin-top:6px}
.label{font-size:14px;color:#777}

.info{
  text-align:left;
  margin-top:10px
}
.row{
  display:flex;
  justify-content:space-between;
  padding:8px 0;
  border-bottom:1px solid #eee;
  font-size:14px
}
.row:last-child{border:none}

/* ---------- Spinner ---------- */
#spinner{
  position:fixed;
  inset:0;
  background:rgba(255,255,255,.85);
  display:none;
  align-items:center;
  justify-content:center;
  flex-direction:column;
  z-index:999
}
.loader{
  width:45px;
  height:45px;
  border:4px solid #ddd;
  border-top:4px solid #4f46e5;
  border-radius:50%;
  animation:spin 1s linear infinite
}
@keyframes spin{to{transform:rotate(360deg)}}

#alert{
  margin-top:15px;
  color:#333;
  font-size:14px
}
</style>
</head>

<body>

<h2 id="title">ESP Weather</h2>

<div id="setup" class="card" style="display:none">
  <input id="city" placeholder="Enter city name">
  <button onclick="saveCity()">Save City</button>
</div>

<div id="weather" style="display:none">

  <div class="card">
    <div class="label">Temperature</div>
    <div class="value"><span id="t">--</span>°C</div>

    <div class="label">Feels Like</div>
    <div class="value"><span id="f">--</span>°C</div>

    <div class="label">Humidity</div>
    <div class="value"><span id="h">--</span>%</div>

    <div class="label">Wind</div>
    <div class="value"><span id="w">--</span> kph</div>

    <button style="margin-top:15px;background:#e11d48" onclick="changeCity()">Change City</button>
  </div>

  <div class="card info">
    <div class="row"><span>WiFi</span><span id="wifi">--</span></div>
    <div class="row"><span>Signal</span><span id="signal">--</span></div>
    <div class="row"><span>IP</span><span id="ip">--</span></div>
    <div class="row"><span>Uptime</span><span id="uptime">--</span></div>
    <div class="row"><span>Free Heap</span><span id="heap">--</span></div>
    <div class="row"><span>Chip ID</span><span id="chip">--</span></div>
  </div>

</div>

<!-- Spinner -->
<div id="spinner">
  <div class="loader"></div>
  <div id="alert">Saving city… Please wait</div>
</div>

<script>
let changing=false;

async function load(){
  if(changing) return;
  const r=await fetch('/weather');
  const j=await r.json();

  if(!j.citySelected){
    setup.style.display='block';
    weather.style.display='none';
    title.innerText='Select City';
  }else{
    setup.style.display='none';
    weather.style.display='block';
    title.innerText=j.city;

    t.innerText=j.temperature;
    f.innerText=j.feelsLike;
    h.innerText=j.humidity;
    w.innerText=j.windSpeed;

    wifi.innerText=j.wifi;
    signal.innerText=j.signal+' dBm';
    ip.innerText=j.ip;
    uptime.innerText=j.uptime;
    heap.innerText=j.freeHeap+' KB';
    chip.innerText=j.chipId;
  }
}

async function saveCity(){
  const c=city.value.trim();
  if(!c) return alert('Enter city name');

  changing=true;
  spinner.style.display='flex';
  alert.innerText='City saved. Refresh page after 5 seconds';

  await fetch('/setCity?name='+encodeURIComponent(c));

  setTimeout(()=>{
    spinner.style.display='none';
    changing=false;
  },5000);
}

function changeCity(){
  setup.style.display='block';
  weather.style.display='none';
  title.innerText='Select City';
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