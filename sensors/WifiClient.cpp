#include <ESP8266WiFi.h>
#include "WifiClient.h"

WifiClient::WifiClient(char *ssid, char *pass) {
  _ssid = ssid;
  _pass = pass;
}

void WifiClient::connect() {
  WiFiClient wifiClient;
  Serial.print("Connecting to WPA SSID [" + String(_ssid)+ "]...");
  WiFi.begin(_ssid, _pass);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
    Serial.print(WiFi.status());

  }

  IPAddress ip = WiFi.localIP();
  Serial.print(" done, IP: ");
  Serial.println(ip);
}

