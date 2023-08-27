#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

 
const char *ssid =  "eil";     // replace with your wifi ssid and wpa2 key
const char *pass =  "Apk.2023!";
String server = "https://gm21kwpk1b.execute-api.eu-west-3.amazonaws.com/prod/message/create";

const int AirValue = 725;   //you need to replace this value with Value_1
const int WaterValue = 310;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent=0;

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps

  Serial.println("Connecting to ");
       Serial.println(ssid);
 
 
       WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED) 
     {
            delay(1000);
            Serial.print("WiFi connection failed, trying again.");
     }
      Serial.println("");
      Serial.println("WiFi connected");
       // Print ESP8266 IP address
      Serial.println(WiFi.localIP());
      
  }
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Create a WiFiClient object to be used by HTTPClient
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    
    http.begin(wifiClient, server);
    http.addHeader("Content-Type", "application/json"); // Set JSON content type
    http.addHeader("Host", server);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    
    // Create the JSON data you want to send in the request body
    String jsonData = "{\"id\":\"10\",\"sensor_id\":\"10\"}";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Handle redirects
    if (httpResponseCode == 307) {
      String newUrl = http.header("Location");
      Serial.print("Redirecting to: ");
      Serial.println(newUrl);

      // Update the server URL with the new URL
      server = newUrl.c_str();

      http.end(); // Close the current connection

      // Perform the request again to the new URL
      http.begin(wifiClient, server);
      http.addHeader("Content-Type", "application/json"); // Set JSON content type
      
      // Create the JSON data you want to send in the request body
      String jsonData = "{\"id\":\"10\",\"sensor_id\":\"10\"}";

      int httpResponseCode = http.POST("{}");
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  delay(5000);
}


  
