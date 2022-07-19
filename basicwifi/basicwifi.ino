#include "std-cred.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiMulti.h>
#endif

//#define STATIC_IP 1

// Setup WiFi
#if defined(ESP8266)
ESP8266WiFiMulti wifiMulti;
#elif defined(ESP32)
WiFiMulti wifiMulti;
#endif

#if defined(STATIC_IP)
// Set your Static IP address
IPAddress WifiLocalIp(192, 168, 0, 116);
// Set your Gateway IP address
IPAddress WifiGateway(192, 168, 0, 1);

IPAddress WifiSubnet(255, 255, 0, 0);
IPAddress WifiDnsPrimary(8, 8, 8, 8);
IPAddress WifiDnsSecondary(8, 8, 4, 4);
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println();
  Serial.println("Starting...");

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.print("MAC Address: ");
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.println(WiFi.macAddress());

  char hostname[20];
  sprintf(hostname, "esp-%02x%02x%02x\0", mac[3], mac[4], mac[5]);

#if defined(ESP8266)
  WiFi.hostname(hostname);
#elif defined(ESP32)
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
#endif

#if defined(STATIC_IP)
  // Configures static IP address
  if (!WiFi.config(WifiLocalIp, WifiGateway, WifiSubnet, WifiDnsPrimary, WifiDnsSecondary)) {
    Serial.println("STA Failed to configure");
  }
#endif

  // Connect to WiFi
#if 1
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  wifiMulti.addAP(WIFI_SSID_ALT1, WIFI_PASS_ALT1);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
#else
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  //WiFi.begin(WIFI_SSID_ALT1, WIFI_PASS_ALT1);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
#endif

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
