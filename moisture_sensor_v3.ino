#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);

String apiServer = "https://gm21kwpk1b.execute-api.eu-west-3.amazonaws.com/prod/message/create";

const int AirValue = 725;   //you need to replace this value with Value_1
const int WaterValue = 310;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilMosturePercent = 0;

const char *credentialsFile = "/credentials.txt";

bool wifiConnected = false; // Track Wi-Fi connection status

void handleRoot() {
  String html = "<html><head><style>";
  html += "<meta http-equiv='refresh' content='10'>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; }";
  html += ".container { text-align: center; margin-top: 100px; }";
  html += "input[type='text'], input[type='password'] { padding: 10px; width: 250px; border: 1px solid #ccc; border-radius: 4px; }";
  html += "input[type='submit'] { background-color: #4caf50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }";
  html += "</style></head><body><div class='container'>";
  html += "<form action='/' method='post'>";
  html += "<h2>Wi-Fi Setup</h2>";
  html += "SSID: <br><input type='text' name='ssid'><br>";
  html += "Password: <br><input type='password' name='password'><br>";
  html += "<br><input type='submit' value='Connect'>";
  html += "</style></head><body><div class='container'>";
  
  if (!wifiConnected) {
    html += "<p style='color: red;'>Wi-Fi connection failed. Check credentials or network.</p>";
  }
  
  html += "</form></div></body></html>";
  server.send(200, "text/html", html);
}

void handleLogin() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  File credsFile = SPIFFS.open(credentialsFile, "w");
  if (credsFile) {
    credsFile.println(ssid);
    credsFile.println(password);
    credsFile.close();
    server.send(200, "text/plain", "Credentials saved. Restarting...");
    delay(1000);
    ESP.restart();
  } else {
    server.send(500, "text/plain", "Failed to save credentials");
  }
}

void startApServer() {
  WiFi.softAP("Wifiapp", "password");
  IPAddress apIP = WiFi.softAPIP();
  Serial.println("AP IP address: " + apIP.toString());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handleLogin);
  server.begin();
}

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
  SPIFFS.begin();

  startApServer();

    // Check if credentials file exists
    if (SPIFFS.exists(credentialsFile)) {
      File credsFile = SPIFFS.open(credentialsFile, "r");
      if (credsFile) {
        String ssid = credsFile.readStringUntil('\n');
        String password = credsFile.readStringUntil('\n');
        credsFile.close();

        // Attempt to connect to Wi-Fi using stored credentials
        WiFi.begin(ssid.c_str(), password.c_str());
        int timeout = 0;
        Serial.print("Connecting to ");
        Serial.println(ssid);
        while (WiFi.status() != WL_CONNECTED && timeout < 10) {
          delay(1000);
          Serial.print(".");
          timeout++;
        }
       wifiConnected = WiFi.status() == WL_CONNECTED;

        if (wifiConnected) {
          Serial.println("\nConnected to WiFi");
          Serial.println(WiFi.localIP());
          // Redirect to root page
          server.sendHeader("Location", "/", true);
          server.send(302, "text/plain", "");
        } else {
          WiFi.disconnect();
          Serial.println("\nWiFi connection failed. Check credentials or network.");
        }
      }
    }


  }
void loop() {
  server.handleClient();
  if (WiFi.status() == WL_CONNECTED) {
    
    soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
    Serial.println(soilMoistureValue);
    soilMosturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    Serial.print(soilMosturePercent);
    Serial.println("%");
    
    HTTPClient http;

    // Create a WiFiClient object to be used by HTTPClient
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    
    http.begin(wifiClient, apiServer);
    http.addHeader("Content-Type", "application/json"); // Set JSON content type
    http.addHeader("Host", apiServer);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    
    // Create the JSON data you want to send in the request body
    String jsonData = "{\"id\":\"10\",\"sensor_id\":\"10\",\"moisture\":\""+String(soilMosturePercent)+"\"}";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  }
  delay(5000);
}


  
