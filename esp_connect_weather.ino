#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

/* ================= CONFIG ================= */
#define AP_SSID        "ESP8266-Setup"
#define AP_PASSWORD    "12345678"
#define EEPROM_SIZE    128
#define SSID_ADDR      0
#define PASS_ADDR      32
#define CITY_ADDR      64
#define LAT_ADDR       96
#define LON_ADDR       100
#define WIFI_TIMEOUT   15000
#define APIKEY         "4e8068c62ffc4034a07163742262401"  // Replace with your API key
/* ========================================== */

AsyncWebServer server(80);
DNSServer dnsServer;
bool isAPMode = true;

/* ---------- WEATHER VARIABLES ---------- */
float temperature = 0;
float humidityVal = 0;
float feelsLike = 0;
float windSpeed = 0;
String cityName = "";
float latitude = 0;
float longitude = 0;
bool useLocation = false;

/* ---------- FLAGS ---------- */
bool citySelected = false;
unsigned long lastFetch = 0;
unsigned long startTime = 0;

/* ================= HTML PAGES ================= */

// WiFi Setup Page (unchanged from previous version)
const char setup_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>WiFi Setup</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;}
body{font-family:'Segoe UI',system-ui,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px;}
.container{max-width:420px;width:100%;}
.card{background:rgba(255,255,255,0.95);backdrop-filter:blur(10px);padding:35px;border-radius:20px;box-shadow:0 15px 35px rgba(0,0,0,0.2);}
.header{text-align:center;margin-bottom:30px;}
.header h2{color:#4a5568;margin-bottom:8px;font-size:28px;font-weight:600;}
.header p{color:#718096;font-size:15px;}
.form-group{margin-bottom:22px;}
label{display:block;margin-bottom:8px;color:#4a5568;font-weight:500;font-size:14px;}
input{width:100%;padding:14px 16px;border:2px solid #e2e8f0;border-radius:10px;background:#fff;color:#2d3748;font-size:16px;transition:all 0.3s;}
input:focus{outline:none;border-color:#667eea;box-shadow:0 0 0 3px rgba(102,126,234,0.1);}
button{width:100%;padding:16px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;border:none;border-radius:10px;font-size:16px;font-weight:600;cursor:pointer;margin-top:8px;transition:all 0.3s;}
button:hover{transform:translateY(-2px);box-shadow:0 10px 20px rgba(102,126,234,0.3);}
button:disabled{background:#a0aec0;cursor:not-allowed;transform:none;}
.status{display:none;padding:14px;border-radius:10px;margin-top:25px;text-align:center;font-weight:500;}
.status.show{display:block;}
.status.success{background:#48bb78;color:#fff;}
.status.error{background:#f56565;color:#fff;}
.footer{text-align:center;margin-top:25px;color:#718096;font-size:14px;}
</style>
</head>
<body>
<div class="container">
<div class="card">
<div class="header">
<h2>🔗 WiFi Setup</h2>
<p>Connect your ESP8266 to WiFi network</p>
</div>
<form id="form">
<div class="form-group">
<label for="ssid">WiFi Network Name (SSID)</label>
<input type="text" id="ssid" placeholder="Enter SSID" required>
</div>
<div class="form-group">
<label for="pass">WiFi Password</label>
<input type="password" id="pass" placeholder="Enter password">
</div>
<button type="submit" id="btn">Connect to WiFi</button>
</form>
<div class="status" id="status"></div>
<div class="footer">
After saving, connect to your WiFi and check serial monitor for IP
</div>
</div>
</div>
<script>
document.getElementById('form').onsubmit = async function(e) {
  e.preventDefault();
  var btn = document.getElementById('btn');
  var status = document.getElementById('status');
  var ssid = document.getElementById('ssid').value.trim();
  var pass = document.getElementById('pass').value;
  
  if(!ssid) {
    status.className = 'status error show';
    status.textContent = '⚠️ Please enter WiFi SSID';
    return;
  }
  
  btn.disabled = true;
  btn.textContent = 'Saving...';
  status.className = 'status';
  
  try {
    const response = await fetch('/save?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass));
    const data = await response.text();
    
    status.className = 'status success show';
    status.textContent = '✅ WiFi saved! Connecting...';
    btn.textContent = '✅ Saved';
    
    setTimeout(function() {
      status.innerHTML = '🔄 Restarting... Please wait 10 seconds, then connect to your WiFi.<br>Check serial monitor for IP address.';
    }, 1500);
    
  } catch(err) {
    status.className = 'status error show';
    status.textContent = '❌ Failed to save WiFi';
    btn.disabled = false;
    btn.textContent = 'Try Again';
  }
};
</script>
</body>
</html>
)rawliteral";

// Weather Page with Location Support
const char weather_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP Weather</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{
  font-family:'Segoe UI',system-ui,sans-serif;
  background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);
  color:#2d3748;
  padding:20px;
  min-height:100vh;
}
.container{
  max-width:480px;
  margin:0 auto;
}
.header{
  text-align:center;
  margin-bottom:25px;
}
.header h2{
  color:white;
  font-size:32px;
  font-weight:700;
  margin-bottom:5px;
  text-shadow:0 2px 4px rgba(0,0,0,0.1);
}
.header p{
  color:rgba(255,255,255,0.9);
  font-size:15px;
}
.card{
  background:rgba(255,255,255,0.97);
  backdrop-filter:blur(10px);
  padding:28px;
  border-radius:20px;
  margin-bottom:20px;
  box-shadow:0 15px 35px rgba(0,0,0,0.15);
  border:1px solid rgba(255,255,255,0.2);
}
.weather-grid{
  display:grid;
  grid-template-columns:1fr 1fr;
  gap:20px;
  margin-bottom:25px;
}
.weather-item{
  text-align:center;
  padding:20px;
  background:rgba(102,126,234,0.08);
  border-radius:15px;
  transition:transform 0.3s;
}
.weather-item:hover{
  transform:translateY(-3px);
}
.weather-item .icon{
  font-size:28px;
  margin-bottom:10px;
  color:#667eea;
}
.weather-item .label{
  font-size:13px;
  color:#718096;
  font-weight:600;
  margin-bottom:8px;
  text-transform:uppercase;
  letter-spacing:0.5px;
}
.weather-item .value{
  font-size:28px;
  font-weight:700;
  color:#2d3748;
}
.button-group{
  display:flex;
  gap:15px;
  margin-top:10px;
}
.btn{
  flex:1;
  padding:16px;
  border:none;
  border-radius:12px;
  font-size:15px;
  font-weight:600;
  cursor:pointer;
  transition:all 0.3s;
  text-align:center;
  text-decoration:none;
  display:inline-block;
}
.btn-primary{
  background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);
  color:white;
}
.btn-secondary{
  background:#edf2f7;
  color:#4a5568;
}
.btn:hover{
  transform:translateY(-2px);
  box-shadow:0 8px 20px rgba(0,0,0,0.15);
}
.btn-primary:hover{
  box-shadow:0 8px 25px rgba(102,126,234,0.4);
}
.btn-secondary:hover{
  background:#e2e8f0;
}
.info-grid{
  display:grid;
  gap:12px;
}
.info-row{
  display:flex;
  justify-content:space-between;
  padding:14px 0;
  border-bottom:1px solid #e2e8f0;
}
.info-row:last-child{
  border-bottom:none;
}
.info-label{
  color:#718096;
  font-weight:500;
}
.info-value{
  color:#2d3748;
  font-weight:600;
}
#setup{
  display:none;
}
.tab-buttons{
  display:flex;
  gap:10px;
  margin-bottom:20px;
  border-bottom:2px solid #e2e8f0;
  padding-bottom:15px;
}
.tab-btn{
  flex:1;
  padding:12px;
  background:#edf2f7;
  border:none;
  border-radius:8px;
  font-size:14px;
  font-weight:600;
  color:#718096;
  cursor:pointer;
  transition:all 0.3s;
}
.tab-btn.active{
  background:#667eea;
  color:white;
}
.tab-content{
  display:none;
}
.tab-content.active{
  display:block;
}
.input-group{
  margin-bottom:20px;
}
.input-group label{
  display:block;
  margin-bottom:8px;
  color:#4a5568;
  font-weight:500;
  font-size:14px;
}
.input-group input{
  width:100%;
  padding:14px;
  border:2px solid #e2e8f0;
  border-radius:10px;
  font-size:16px;
  background:#fff;
  color:#2d3748;
}
.input-group input:focus{
  outline:none;
  border-color:#667eea;
}
.coords-row{
  display:flex;
  gap:15px;
}
.coords-row .input-group{
  flex:1;
}
.location-info{
  background:#f7fafc;
  padding:15px;
  border-radius:10px;
  margin-bottom:20px;
  border:1px solid #e2e8f0;
}
.location-info p{
  margin:5px 0;
  color:#4a5568;
}
.spinner{
  position:fixed;
  top:0;left:0;right:0;bottom:0;
  background:rgba(255,255,255,0.95);
  display:none;
  align-items:center;
  justify-content:center;
  flex-direction:column;
  z-index:1000;
}
.loader{
  width:50px;
  height:50px;
  border:4px solid #e2e8f0;
  border-top:4px solid #667eea;
  border-radius:50%;
  animation:spin 1s linear infinite;
}
@keyframes spin{
  to{transform:rotate(360deg)}
}
.spinner-text{
  margin-top:20px;
  color:#4a5568;
  font-size:16px;
}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h2 id="title">🌤️ ESP Weather</h2>
    <p id="subtitle">Real-time weather updates</p>
  </div>

  <!-- Location Setup -->
  <div id="setup" class="card">
    <div class="tab-buttons">
      <button class="tab-btn active" onclick="showTab('cityTab')">📍 City</button>
      <button class="tab-btn" onclick="showTab('locationTab')">🌍 My Location</button>
    </div>
    
    <!-- City Tab -->
    <div id="cityTab" class="tab-content active">
      <div class="input-group">
        <label for="city">Enter city name</label>
        <input type="text" id="city" placeholder="e.g., Kolkata, London, Tokyo">
      </div>
      <button class="btn btn-primary" onclick="saveCity()">Use This City</button>
    </div>
    
    <!-- Location Tab -->
    <div id="locationTab" class="tab-content">
      <div class="location-info">
        <p><strong>📍 Use your current location</strong></p>
        <p style="font-size:13px;color:#718096;">We'll get your GPS coordinates and use them to fetch accurate local weather.</p>
      </div>
      
      <div class="coords-row">
        <div class="input-group">
          <label for="latitude">Latitude</label>
          <input type="number" id="latitude" placeholder="e.g., 22.5726" step="0.0001">
        </div>
        <div class="input-group">
          <label for="longitude">Longitude</label>
          <input type="number" id="longitude" placeholder="e.g., 88.3639" step="0.0001">
        </div>
      </div>
      
      <div style="margin:20px 0;">
        <button class="btn btn-secondary" onclick="getMyLocation()">📍 Get My Location</button>
      </div>
      
      <button class="btn btn-primary" onclick="saveLocation()">Use This Location</button>
    </div>
  </div>

  <!-- Weather Display -->
  <div id="weather" class="card">
    <div class="weather-grid">
      <div class="weather-item">
        <div class="icon">🌡️</div>
        <div class="label">Temperature</div>
        <div class="value"><span id="t">--</span>°C</div>
      </div>
      <div class="weather-item">
        <div class="icon">🤔</div>
        <div class="label">Feels Like</div>
        <div class="value"><span id="f">--</span>°C</div>
      </div>
      <div class="weather-item">
        <div class="icon">💧</div>
        <div class="label">Humidity</div>
        <div class="value"><span id="h">--</span>%</div>
      </div>
      <div class="weather-item">
        <div class="icon">💨</div>
        <div class="label">Wind Speed</div>
        <div class="value"><span id="w">--</span> kph</div>
      </div>
    </div>
    
    <div style="margin:15px 0;text-align:center;">
      <span id="locationType" style="color:#718096;font-size:14px;"></span>
    </div>
    
    <div class="button-group">
      <button class="btn btn-primary" onclick="changeLocation()">Change Location</button>
      <a href="/settings" class="btn btn-secondary">⚙️ Settings</a>
    </div>
  </div>

  <!-- System Info -->
  <div class="card">
    <div class="info-grid">
      <div class="info-row">
        <span class="info-label">📡 WiFi</span>
        <span class="info-value" id="wifi">--</span>
      </div>
      <div class="info-row">
        <span class="info-label">📶 Signal</span>
        <span class="info-value" id="signal">-- dBm</span>
      </div>
      <div class="info-row">
        <span class="info-label">📍 IP Address</span>
        <span class="info-value" id="ip">--</span>
      </div>
      <div class="info-row">
        <span class="info-label">⏱️ Uptime</span>
        <span class="info-value" id="uptime">--</span>
      </div>
      <div class="info-row">
        <span class="info-label">💾 Free Memory</span>
        <span class="info-value" id="heap">-- KB</span>
      </div>
      <div class="info-row">
        <span class="info-label">🆔 Chip ID</span>
        <span class="info-value" id="chip">--</span>
      </div>
    </div>
  </div>
</div>

<!-- Loading Spinner -->
<div class="spinner" id="spinner">
  <div class="loader"></div>
  <div class="spinner-text" id="spinnerText">Loading...</div>
</div>

<script>
let updating = false;
let currentTab = 'cityTab';

function showTab(tabId) {
  // Hide all tabs
  document.querySelectorAll('.tab-content').forEach(tab => {
    tab.classList.remove('active');
  });
  document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.classList.remove('active');
  });
  
  // Show selected tab
  document.getElementById(tabId).classList.add('active');
  document.querySelector(`[onclick="showTab('${tabId}')"]`).classList.add('active');
  currentTab = tabId;
}

function getMyLocation() {
  if (!navigator.geolocation) {
    alert('Geolocation is not supported by your browser');
    return;
  }
  
  showSpinner('Getting your location...');
  
  navigator.geolocation.getCurrentPosition(
    (position) => {
      document.getElementById('latitude').value = position.coords.latitude.toFixed(6);
      document.getElementById('longitude').value = position.coords.longitude.toFixed(6);
      hideSpinner();
    },
    (error) => {
      hideSpinner();
      alert('Unable to get your location. Please enter coordinates manually.');
      console.error('Geolocation error:', error);
    }
  );
}

async function loadWeather() {
  if(updating) return;
  
  try {
    const response = await fetch('/weather');
    const data = await response.json();
    
    if(!data.citySelected) {
      // Show location setup
      document.getElementById('setup').style.display = 'block';
      document.getElementById('weather').style.display = 'none';
      document.getElementById('title').textContent = 'Set Location';
      document.getElementById('subtitle').textContent = 'Choose how to get weather data';
    } else {
      // Show weather
      document.getElementById('setup').style.display = 'none';
      document.getElementById('weather').style.display = 'block';
      document.getElementById('title').textContent = data.displayName + ' Weather';
      document.getElementById('subtitle').textContent = 'Last updated: ' + data.lastUpdate;
      
      // Update location type
      document.getElementById('locationType').textContent = 
        data.useLocation ? '📍 Using your location' : '🏙️ Using city: ' + data.city;
      
      // Update weather values
      document.getElementById('t').textContent = data.temperature;
      document.getElementById('f').textContent = data.feelsLike;
      document.getElementById('h').textContent = data.humidity;
      document.getElementById('w').textContent = data.windSpeed;
      
      // Update system info
      document.getElementById('wifi').textContent = data.wifi;
      document.getElementById('signal').textContent = data.signal + ' dBm';
      document.getElementById('ip').textContent = data.ip;
      document.getElementById('uptime').textContent = data.uptime;
      document.getElementById('heap').textContent = data.freeHeap + ' KB';
      document.getElementById('chip').textContent = data.chipId;
    }
  } catch(error) {
    console.error('Error loading weather:', error);
  }
}

async function saveCity() {
  const cityInput = document.getElementById('city');
  const city = cityInput.value.trim();
  
  if(!city) {
    alert('Please enter a city name');
    return;
  }
  
  updating = true;
  showSpinner('Saving city...');
  
  try {
    await fetch('/setCity?name=' + encodeURIComponent(city) + '&type=city');
    
    // Wait a moment then reload
    setTimeout(() => {
      hideSpinner();
      updating = false;
      loadWeather();
    }, 1500);
    
  } catch(error) {
    hideSpinner();
    updating = false;
    alert('Failed to save city. Please try again.');
    console.error('Error saving city:', error);
  }
}

async function saveLocation() {
  const latInput = document.getElementById('latitude');
  const lonInput = document.getElementById('longitude');
  const lat = latInput.value.trim();
  const lon = lonInput.value.trim();
  
  if(!lat || !lon) {
    alert('Please enter both latitude and longitude');
    return;
  }
  
  // Validate coordinates
  const latNum = parseFloat(lat);
  const lonNum = parseFloat(lon);
  
  if(isNaN(latNum) || isNaN(lonNum) || latNum < -90 || latNum > 90 || lonNum < -180 || lonNum > 180) {
    alert('Please enter valid coordinates:\nLatitude: -90 to 90\nLongitude: -180 to 180');
    return;
  }
  
  updating = true;
  showSpinner('Saving location...');
  
  try {
    await fetch('/setLocation?lat=' + encodeURIComponent(lat) + '&lon=' + encodeURIComponent(lon) + '&type=location');
    
    // Wait a moment then reload
    setTimeout(() => {
      hideSpinner();
      updating = false;
      loadWeather();
    }, 1500);
    
  } catch(error) {
    hideSpinner();
    updating = false;
    alert('Failed to save location. Please try again.');
    console.error('Error saving location:', error);
  }
}

function changeLocation() {
  document.getElementById('setup').style.display = 'block';
  document.getElementById('weather').style.display = 'none';
  document.getElementById('title').textContent = 'Change Location';
  document.getElementById('subtitle').textContent = 'Choose how to get weather data';
  
  // Reset tabs
  showTab('cityTab');
  document.getElementById('city').value = '';
  document.getElementById('latitude').value = '';
  document.getElementById('longitude').value = '';
}

function showSpinner(text) {
  document.getElementById('spinnerText').textContent = text;
  document.getElementById('spinner').style.display = 'flex';
}

function hideSpinner() {
  document.getElementById('spinner').style.display = 'none';
}

// Auto-refresh every 30 seconds
loadWeather();
setInterval(loadWeather, 30000);

// Handle Enter key in inputs
document.getElementById('city')?.addEventListener('keypress', function(e) {
  if(e.key === 'Enter') saveCity();
});
document.getElementById('latitude')?.addEventListener('keypress', function(e) {
  if(e.key === 'Enter') saveLocation();
});
document.getElementById('longitude')?.addEventListener('keypress', function(e) {
  if(e.key === 'Enter') saveLocation();
});
</script>
</body>
</html>
)rawliteral";

// Settings Page (updated with location info)
const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP Settings</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{
  font-family:'Segoe UI',system-ui,sans-serif;
  background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);
  color:#2d3748;
  padding:20px;
  min-height:100vh;
}
.container{
  max-width:480px;
  margin:0 auto;
}
.header{
  text-align:center;
  margin-bottom:25px;
}
.header h1{
  color:white;
  font-size:32px;
  font-weight:700;
  margin-bottom:5px;
}
.header p{
  color:rgba(255,255,255,0.9);
  font-size:15px;
}
.card{
  background:rgba(255,255,255,0.97);
  backdrop-filter:blur(10px);
  padding:28px;
  border-radius:20px;
  margin-bottom:20px;
  box-shadow:0 15px 35px rgba(0,0,0,0.15);
  border:1px solid rgba(255,255,255,0.2);
}
.card h2{
  color:#4a5568;
  font-size:22px;
  margin-bottom:20px;
  padding-bottom:12px;
  border-bottom:2px solid #e2e8f0;
}
.info-grid{
  display:grid;
  gap:14px;
}
.info-row{
  display:flex;
  justify-content:space-between;
  padding:14px 0;
  border-bottom:1px solid #e2e8f0;
}
.info-row:last-child{
  border-bottom:none;
}
.info-label{
  color:#718096;
  font-weight:500;
  display:flex;
  align-items:center;
  gap:8px;
}
.info-value{
  color:#2d3748;
  font-weight:600;
}
.btn-group{
  display:grid;
  gap:12px;
  margin-top:25px;
}
.btn{
  padding:16px;
  border:none;
  border-radius:12px;
  font-size:15px;
  font-weight:600;
  cursor:pointer;
  transition:all 0.3s;
  text-align:center;
  text-decoration:none;
  display:block;
}
.btn-danger{
  background:linear-gradient(135deg,#f56565 0%,#ed64a6 100%);
  color:white;
}
.btn-secondary{
  background:#edf2f7;
  color:#4a5568;
}
.btn:hover{
  transform:translateY(-2px);
  box-shadow:0 8px 20px rgba(0,0,0,0.15);
}
.btn-danger:hover{
  box-shadow:0 8px 25px rgba(245,101,101,0.4);
}
.btn-secondary:hover{
  background:#e2e8f0;
}
.location-badge{
  display:inline-block;
  padding:4px 12px;
  border-radius:20px;
  font-size:12px;
  font-weight:600;
  margin-left:10px;
}
.location-badge.city{
  background:#c6f6d5;
  color:#22543d;
}
.location-badge.gps{
  background:#bee3f8;
  color:#2c5282;
}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>⚙️ ESP Settings</h1>
    <p>System information and configuration</p>
  </div>

  <div class="card">
    <h2>📡 Network Information</h2>
    <div class="info-grid">
      <div class="info-row">
        <span class="info-label">📶 Status</span>
        <span class="info-value" style="color:#48bb78;">Connected ✓</span>
      </div>
      <div class="info-row">
        <span class="info-label">🌐 SSID</span>
        <span class="info-value">%SSID%</span>
      </div>
      <div class="info-row">
        <span class="info-label">📍 IP Address</span>
        <span class="info-value">%IP%</span>
      </div>
      <div class="info-row">
        <span class="info-label">📡 Signal</span>
        <span class="info-value">%RSSI% dBm</span>
      </div>
      <div class="info-row">
        <span class="info-label">🆔 MAC Address</span>
        <span class="info-value">%MAC%</span>
      </div>
    </div>
  </div>

  <div class="card">
    <h2>📍 Location Settings</h2>
    <div class="info-grid">
      <div class="info-row">
        <span class="info-label">🎯 Type</span>
        <span class="info-value">
          %LOC_TYPE%
          <span class="location-badge %LOC_BADGE%">%LOC_BADGE_TEXT%</span>
        </span>
      </div>
      <div class="info-row">
        <span class="info-label">🏙️ City Name</span>
        <span class="info-value">%CITY%</span>
      </div>
      <div class="info-row">
        <span class="info-label">📍 Latitude</span>
        <span class="info-value">%LAT%</span>
      </div>
      <div class="info-row">
        <span class="info-label">📍 Longitude</span>
        <span class="info-value">%LON%</span>
      </div>
      <div class="info-row">
        <span class="info-label">⏱️ Last Update</span>
        <span class="info-value">%LAST_UPDATE%</span>
      </div>
    </div>
  </div>

  <div class="card">
    <h2>🔧 System Information</h2>
    <div class="info-grid">
      <div class="info-row">
        <span class="info-label">⏱️ Uptime</span>
        <span class="info-value">%UPTIME%</span>
      </div>
      <div class="info-row">
        <span class="info-label">💾 Free Memory</span>
        <span class="info-value">%HEAP% KB</span>
      </div>
      <div class="info-row">
        <span class="info-label">🆔 Chip ID</span>
        <span class="info-value">%CHIPID%</span>
      </div>
    </div>
  </div>

  <div class="card">
    <h2 style="color:#f56565;">⚠️ Danger Zone</h2>
    <p style="color:#718096;margin-bottom:20px;">
      Warning: These actions cannot be undone. ESP will restart in WiFi setup mode.
    </p>
    <div class="btn-group">
      <button class="btn btn-danger" onclick="if(confirm('This will erase all settings. Continue?')) location.href='/reset'">
        🔄 Reset All Settings
      </button>
      <a href="/" class="btn btn-secondary">⬅️ Back to Weather</a>
    </div>
  </div>
</div>
</body>
</html>
)rawliteral";

/* ================= EEPROM FUNCTIONS ================= */
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
  Serial.println("[EEPROM] Saved WiFi: " + ssid);
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

void saveCity(String city) {
  Serial.println("[EEPROM] Saving city: " + city);
  EEPROM.write(CITY_ADDR, city.length());
  for(int i = 0; i < city.length(); i++) {
    EEPROM.write(CITY_ADDR + 1 + i, city[i]);
  }
  // Mark as city mode (type=0)
  EEPROM.write(CITY_ADDR + 31, 0);
  EEPROM.commit();
  Serial.println("[EEPROM] City saved");
}

void saveLocation(float lat, float lon) {
  Serial.println("[EEPROM] Saving location: " + String(lat, 6) + ", " + String(lon, 6));
  
  // Save city name as "Lat,Lon" for display
  String locStr = String(lat, 6) + "," + String(lon, 6);
  EEPROM.write(CITY_ADDR, locStr.length());
  for(int i = 0; i < locStr.length(); i++) {
    EEPROM.write(CITY_ADDR + 1 + i, locStr[i]);
  }
  
  // Mark as location mode (type=1)
  EEPROM.write(CITY_ADDR + 31, 1);
  
  // Save lat/lon as floats (4 bytes each)
  uint8_t *latBytes = (uint8_t*)&lat;
  uint8_t *lonBytes = (uint8_t*)&lon;
  for(int i = 0; i < 4; i++) {
    EEPROM.write(LAT_ADDR + i, latBytes[i]);
    EEPROM.write(LON_ADDR + i, lonBytes[i]);
  }
  
  EEPROM.commit();
  Serial.println("[EEPROM] Location saved");
}

String loadCity() {
  int len = EEPROM.read(CITY_ADDR);
  if(len <= 0 || len > 30) return "";
  char buf[32];
  for(int i = 0; i < len; i++) {
    buf[i] = EEPROM.read(CITY_ADDR + 1 + i);
  }
  buf[len] = '\0';
  
  // Check if we're in location mode
  useLocation = (EEPROM.read(CITY_ADDR + 31) == 1);
  
  // If in location mode, load lat/lon from their addresses
  if(useLocation && len > 0) {
    uint8_t latBytes[4], lonBytes[4];
    for(int i = 0; i < 4; i++) {
      latBytes[i] = EEPROM.read(LAT_ADDR + i);
      lonBytes[i] = EEPROM.read(LON_ADDR + i);
    }
    latitude = *(float*)latBytes;
    longitude = *(float*)lonBytes;
    Serial.println("[EEPROM] Loaded location: " + String(latitude, 6) + ", " + String(longitude, 6));
  }
  
  return String(buf);
}

/* ================= WIFI CONNECTION ================= */
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
    Serial.println("\n[WiFi] Connected successfully!");
    Serial.println("[WiFi] SSID: " + WiFi.SSID());
    Serial.println("[WiFi] IP Address: " + WiFi.localIP().toString());
    Serial.println("[WiFi] Signal Strength: " + String(WiFi.RSSI()) + " dBm");
    Serial.println("\n══════════════════════════════════════════");
    Serial.println("🌐 Weather Station is ready!");
    Serial.println("🌍 Open in browser: http://" + WiFi.localIP().toString());
    Serial.println("══════════════════════════════════════════\n");
    return true;
  }
  
  Serial.println("\n[WiFi] Failed to connect");
  return false;
}

/* ================= WEATHER FETCH ================= */
void fetchWeather() {
  if(cityName == "" && !useLocation) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String url;
  if(useLocation) {
    // Use latitude/longitude
    url = "https://api.weatherapi.com/v1/current.json?key=" +
          String(APIKEY) + "&q=" + 
          String(latitude, 6) + "," + String(longitude, 6) + "&aqi=no";
    Serial.println("[Weather] Fetching by location: " + String(latitude, 6) + ", " + String(longitude, 6));
  } else {
    // Use city name
    url = "https://api.weatherapi.com/v1/current.json?key=" +
          String(APIKEY) + "&q=" + cityName + "&aqi=no";
    Serial.println("[Weather] Fetching by city: " + cityName);
  }

  if(https.begin(client, url)) {
    int code = https.GET();
    if(code > 0) {
      String payload = https.getString();
      
      // Extract location name from response
      int nameIdx = payload.indexOf("\"name\":\"");
      if(nameIdx > 0) {
        int nameEndIdx = payload.indexOf("\"", nameIdx + 8);
        cityName = payload.substring(nameIdx + 8, nameEndIdx);
      }
      
      int tempIdx = payload.indexOf("\"temp_c\":");
      int humIdx = payload.indexOf("\"humidity\":");
      int feelsIdx = payload.indexOf("\"feelslike_c\":");
      int windIdx = payload.indexOf("\"wind_kph\":");
      
      if(tempIdx > 0) {
        temperature = payload.substring(tempIdx + 9, payload.indexOf(",", tempIdx)).toFloat();
      }
      if(humIdx > 0) {
        humidityVal = payload.substring(humIdx + 11, payload.indexOf(",", humIdx)).toFloat();
      }
      if(feelsIdx > 0) {
        feelsLike = payload.substring(feelsIdx + 14, payload.indexOf(",", feelsIdx)).toFloat();
      }
      if(windIdx > 0) {
        windSpeed = payload.substring(windIdx + 11, payload.indexOf(",", windIdx)).toFloat();
      }
      
      Serial.println("[Weather] Fetched: " + cityName + 
                    " Temp=" + String(temperature,1) + "°C" +
                    " Humidity=" + String(humidityVal,0) + "%" +
                    " Wind=" + String(windSpeed,1) + "kph");
    } else {
      Serial.println("[Weather] Failed to fetch, code: " + String(code));
    }
    https.end();
  }
}

/* ================= HELPER FUNCTIONS ================= */
String getUptime() {
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

String getLastUpdateTime() {
  unsigned long secondsSince = (millis() - lastFetch) / 1000;
  if(lastFetch == 0) return "Never";
  
  if(secondsSince < 60) return "Just now";
  else if(secondsSince < 3600) return String(secondsSince / 60) + " minutes ago";
  else if(secondsSince < 86400) return String(secondsSince / 3600) + " hours ago";
  else return String(secondsSince / 86400) + " days ago";
}

String getDisplayName() {
  if(useLocation) {
    return cityName; // cityName will contain the location name from API response
  }
  return cityName;
}

/* ================= WEB SERVER ROUTES ================= */
void setupWeatherRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", weather_html);
  });

  server.on("/setCity", HTTP_GET, [](AsyncWebServerRequest *req) {
    if(req->hasParam("name")) {
      cityName = req->getParam("name")->value();
      useLocation = false;
      saveCity(cityName);
      citySelected = true;
      fetchWeather(); // Fetch immediately after setting city
      req->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      req->send(400, "application/json", "{\"error\":\"No city name\"}");
    }
  });

  server.on("/setLocation", HTTP_GET, [](AsyncWebServerRequest *req) {
    if(req->hasParam("lat") && req->hasParam("lon")) {
      latitude = req->getParam("lat")->value().toFloat();
      longitude = req->getParam("lon")->value().toFloat();
      useLocation = true;
      saveLocation(latitude, longitude);
      citySelected = true;
      fetchWeather(); // Fetch immediately after setting location
      req->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      req->send(400, "application/json", "{\"error\":\"Missing coordinates\"}");
    }
  });

  server.on("/weather", HTTP_GET, [](AsyncWebServerRequest *req) {
    String json = "{";
    json += "\"citySelected\":" + String(citySelected ? "true" : "false") + ",";
    json += "\"city\":\"" + cityName + "\",";
    json += "\"displayName\":\"" + getDisplayName() + "\",";
    json += "\"useLocation\":" + String(useLocation ? "true" : "false") + ",";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidityVal, 0) + ",";
    json += "\"feelsLike\":" + String(feelsLike, 1) + ",";
    json += "\"windSpeed\":" + String(windSpeed, 1) + ",";
    json += "\"lastUpdate\":\"" + getLastUpdateTime() + "\",";
    
    // System information
    json += "\"wifi\":\"" + WiFi.SSID() + "\",";
    json += "\"signal\":" + String(WiFi.RSSI()) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"uptime\":\"" + getUptime() + "\",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap() / 1024.0, 2) + ",";
    json += "\"chipId\":\"0x" + String(ESP.getChipId(), HEX) + "\"";
    
    json += "}";
    req->send(200, "application/json", json);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *req) {
    String html = String(settings_html);
    html.replace("%SSID%", WiFi.SSID());
    html.replace("%IP%", WiFi.localIP().toString());
    html.replace("%RSSI%", String(WiFi.RSSI()));
    html.replace("%MAC%", WiFi.macAddress());
    html.replace("%UPTIME%", getUptime());
    html.replace("%HEAP%", String(ESP.getFreeHeap() / 1024));
    html.replace("%CHIPID%", String(ESP.getChipId(), HEX));
    
    // Location info
    html.replace("%CITY%", cityName);
    html.replace("%LAT%", String(latitude, 6));
    html.replace("%LON%", String(longitude, 6));
    html.replace("%LAST_UPDATE%", getLastUpdateTime());
    
    if(useLocation) {
      html.replace("%LOC_TYPE%", "GPS Coordinates");
      html.replace("%LOC_BADGE%", "gps");
      html.replace("%LOC_BADGE_TEXT%", "GPS");
    } else {
      html.replace("%LOC_TYPE%", "City Name");
      html.replace("%LOC_BADGE%", "city");
      html.replace("%LOC_BADGE_TEXT%", "City");
    }
    
    req->send(200, "text/html", html);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", 
      "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"
      "<style>body{font-family:sans-serif;background:#667eea;color:white;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;}"
      ".card{background:white;color:#2d3748;padding:40px;border-radius:20px;text-align:center;max-width:400px;margin:20px;box-shadow:0 20px 40px rgba(0,0,0,0.1);}"
      "</style></head><body><div class='card'>"
      "<h2 style='color:#667eea;'>🔄 Resetting...</h2>"
      "<p>All settings will be cleared. ESP will restart in WiFi setup mode.</p>"
      "<p style='color:#718096;margin-top:20px;'>Please reconnect to 'ESP8266-Setup' WiFi.</p>"
      "</div></body></html>");
    
    Serial.println("[System] Reset requested - clearing all settings");
    delay(1000);
    clearEEPROM();
    delay(2000);
    ESP.restart();
  });

  server.onNotFound([](AsyncWebServerRequest *req) {
    req->redirect("/");
  });
}

void setupAPRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", setup_html);
  });
  
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *req) {
    String ssid = req->arg("ssid");
    String pass = req->arg("pass");
    
    Serial.println("\n[WiFi Setup] Received credentials:");
    Serial.println("  SSID: " + ssid);
    Serial.println("  Pass: " + String(pass.length() > 0 ? "***" : "(empty)"));
    
    saveWiFi(ssid, pass);
    req->send(200, "text/plain", "OK");
    
    Serial.println("\n[System] WiFi saved. Restarting in 3 seconds...");
    Serial.println("After restart, connect to your WiFi network and check serial monitor for IP address.");
    Serial.println("══════════════════════════════════════════\n");
    
    delay(3000);
    ESP.restart();
  });

  server.onNotFound([](AsyncWebServerRequest *req) {
    req->redirect("/");
  });
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n══════════════════════════════════════════");
  Serial.println("        🌟 ESP8266 Weather Station");
  Serial.println("══════════════════════════════════════════\n");
  
  EEPROM.begin(EEPROM_SIZE);
  startTime = millis();
  
  // Clear EEPROM on first boot (uncomment if needed)
  // clearEEPROM();
  
  Serial.println("[System] Initializing...");
  
  if (connectWiFi()) {
    isAPMode = false;
    Serial.println("[Mode] Station Mode - Weather Station Active");
    
    // Load location settings from EEPROM
    cityName = loadCity();
    citySelected = cityName != "";
    
    if (citySelected) {
      if(useLocation) {
        Serial.println("[Location] Loaded from EEPROM: " + String(latitude, 6) + ", " + String(longitude, 6));
      } else {
        Serial.println("[City] Loaded from EEPROM: " + cityName);
      }
      fetchWeather();
    } else {
      Serial.println("[Location] No location configured - will prompt on first visit");
    }
    
    setupWeatherRoutes();
    server.begin();
    
  } else {
    isAPMode = true;
    Serial.println("[Mode] AP Mode - WiFi Setup Required");
    Serial.println("[AP] Starting WiFi Access Point...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    delay(100);
    
    Serial.println("\n══════════════════════════════════════════");
    Serial.println("📶 WiFi Setup Mode");
    Serial.println("SSID: " + String(AP_SSID));
    Serial.println("Password: " + String(AP_PASSWORD));
    Serial.println("IP Address: " + WiFi.softAPIP().toString());
    Serial.println("");
    Serial.println("📱 Connect to this WiFi and open:");
    Serial.println("🌐 http://" + WiFi.softAPIP().toString());
    Serial.println("══════════════════════════════════════════\n");
    
    dnsServer.start(53, "*", WiFi.softAPIP());
    setupAPRoutes();
    server.begin();
  }
  
  Serial.println("[Web Server] Started on port 80");
}

/* ================= LOOP ================= */
void loop() {
  if (isAPMode) {
    dnsServer.processNextRequest();
  } else if (citySelected && millis() - lastFetch > 300000) { // Fetch every 5 minutes
    lastFetch = millis();
    fetchWeather();
  }
}