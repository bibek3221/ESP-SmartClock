#include <ESP8266WiFi.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP8266 Basic Test ===");
  
  // Test 1: WiFi Connection
  Serial.println("1. Testing WiFi...");
  WiFi.begin(ssid, password);
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi Failed");
  }
  
  // Test 2: Start Server
  Serial.println("\n2. Starting Web Server...");
  server.begin();
  Serial.println("✓ Server started on port 80");
  
  // Test 3: Board Info
  Serial.println("\n3. Board Information:");
  Serial.print("Chip ID: ");
  Serial.println(ESP.getChipId());
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.println(" MB");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  
  Serial.println("\n=== Test Ready ===");
  Serial.println("Send 't' for WiFi test, 'r' for reset");
}

void loop() {
  // Check for serial input
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 't') {
      Serial.print("WiFi Status: ");
      Serial.println(WiFi.status());
      Serial.print("RSSI: ");
      Serial.println(WiFi.RSSI());
    }
    if (c == 'r') {
      Serial.println("Resetting...");
      ESP.restart();
    }
  }
  
  // Handle web clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    
    // Read request
    String request = client.readStringUntil('\r');
    Serial.print("Request: ");
    Serial.println(request);
    
    // Send response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head><title>ESP8266 Test</title></head>");
    client.println("<body>");
    client.println("<h1>ESP8266 Working!</h1>");
    client.println("<p>Board is operational</p>");
    client.println("<p>Free Memory: " + String(ESP.getFreeHeap()) + " bytes</p>");
    client.println("</body>");
    client.println("</html>");
    
    client.stop();
    Serial.println("Client disconnected");
  }
  
  delay(100);
}