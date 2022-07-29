/**

Temperature Monitor

Featuring
- Read temperature sensor
- Send data to cloud
- Deep sleep for battery power
- Http update

*/

#define DEEP_SLEEP_ENABLE 1
#define ENABLE_ADAFRUIT_IO 1
//#define WAIT_TIME_SYNC 1
//#define STATIC_IP 1
#define DEBUG_LOCAL 1

#ifdef DEBUG_LOCAL
#define SERIALPRINTLN(x) Serial.println(x)
#define SERIALPRINT(x) Serial.print(x)
#else
#define SERIALPRINTLN(x)
#define SERIALPRINT(x)
#endif

#include <Arduino.h>

// For WiFi
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiMulti.h>
#endif

#include "tempmon-cred.h"
#include "util-time.h"
#include "util-httpupdate.h"
#include "util-dht.h"
//#if ENABLE_ADAFRUIT_IO
#include "util-adafruitio.h"
//#endif

#define MEASUREMENT_INTERVAL_MIN 15
#define WAKE_EARLY_MIN 1
#define DEEP_SLEEP_TIME_US ((MEASUREMENT_INTERVAL_MIN - WAKE_EARLY_MIN) * 60 * 1e6)

#if defined(ESP8266)
ESP8266WiFiMulti WiFiMulti;
#elif defined(ESP32)
WiFiMulti WiFiMulti;
#endif

#if defined(STATIC_IP)
// Set your Static IP address
IPAddress WifiLocalIp(192, 168, 111, 62);
// Set your Gateway IP address
IPAddress WifiGateway(192, 168, 111, 1);

IPAddress WifiSubnet(255, 255, 255, 0);
IPAddress WifiDnsPrimary(192, 168, 111, 1);
IPAddress WifiDnsSecondary(192, 168, 111, 1);
#endif
#if defined(STATIC_IP_WORK)
// Set your Static IP address
IPAddress WifiLocalIp(192, 168, 0, 220);
// Set your Gateway IP address
IPAddress WifiGateway(192, 168, 0, 1);

IPAddress WifiSubnet(255, 255, 255, 0);
IPAddress WifiDnsPrimary(8, 8, 8, 8);
IPAddress WifiDnsSecondary(8, 8, 4, 4);
#endif

utilTime theTime;
utilHttpUpdate httpUpdater;
utilDht sensorDht;

#if defined(ENABLE_ADAFRUIT_IO)
utilAdafruitIo adafruitIo;
#endif

float currentTemperature;
float currentHumidity;
unsigned long startTime;
unsigned long connTime;

void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  startTime = millis();

#ifdef DEBUG_LOCAL
  // Setup serial port
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  while (!Serial) { ; }
  Serial.println();
  Serial.println("TempMon Starting...");
#endif
/*
  // Set the hostname
  SERIALPRINT("MAC Address: ");
  uint8_t mac[6];
  WiFi.macAddress(mac);
  SERIALPRINTLN(WiFi.macAddress());

  char hostname[20];
  sprintf(hostname, "esp-%02x%02x%02x\0", mac[3], mac[4], mac[5]);

#if defined(ESP8266)
  WiFi.hostname(hostname);
#elif defined(ESP32)  
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
#endif
*/
  // Setup wifi
#if 0
  WiFi.mode(WIFI_STA);

#if defined(STATIC_IP)
  // Configures static IP address
  if (!WiFi.config(WifiLocalIp, WifiGateway, WifiSubnet, WifiDnsPrimary, WifiDnsSecondary)) {
    SERIALPRINTLN("STA Failed to configure");
  }
#endif

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  //wifiMulti.addAP(WIFI_SSID_ALT1, WIFI_PASS_ALT1);

  // Wait for connect
  SERIALPRINT("WiFi multi connecting .");
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(1000);
    SERIALPRINT('.');
  }
#else
  WiFi.mode(WIFI_STA);
  
#if defined(STATIC_IP)
  // Configures static IP address
  if (!WiFi.config(WifiLocalIp, WifiGateway, WifiSubnet, WifiDnsPrimary, WifiDnsSecondary)) {
    SERIALPRINTLN("STA Failed to configure");
  }
#endif

  // channel = 6
  uint8_t bssid[] = {0x5C,0xB1,0x3E,0x5F,0x41,0xFC};

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  //WiFi.begin(WIFI_SSID, WIFI_PASS, 6, bssid, true);

  // Wait for connect
  SERIALPRINT("WiFi connecting .");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    SERIALPRINT('.');
  }
#endif
  connTime = millis();

  SERIALPRINTLN("");
  SERIALPRINT("WiFi connected ");
  SERIALPRINTLN(connTime - startTime);
  SERIALPRINT("IP address: ");
  SERIALPRINTLN(WiFi.localIP());

  // Get NTP time
  theTime.Setup();

#if ENABLE_ADAFRUIT_IO
  // Setup cloud
  adafruitIo.Setup();

#ifdef UPDATE_TRIGGER_ENABLE
  // Check for updates if flag has been set
  adafruitIo.requestCommandUpdate();
  while (adafruitIo.WaitForMessage()) {
    delay(10);
  }

  if (adafruitIo.getCommandUpdate()) {
    SERIALPRINTLN("Updating - yes");
    httpUpdater.Setup();
  }
  else {
    SERIALPRINTLN("Updating - not");
  }
#endif
#endif

  // Setup the sensor
  sensorDht.Setup();

  digitalWrite(2, HIGH);
  SERIALPRINT("TS ");
  SERIALPRINTLN(millis() - startTime);
}

void loop() {
  // Update the time  
  theTime.Loop();

  // Check the time, we only send on 15 min intervals
  uint8_t currentMinute;
  currentMinute = theTime.getMinute();

#ifdef WAIT_TIME_SYNC
  if (currentMinute % MEASUREMENT_INTERVAL_MIN == 0)
#endif
  {
    // Read the sensors
    currentTemperature = sensorDht.getTemperature();
    currentHumidity = sensorDht.getHumidity();

#if ENABLE_ADAFRUIT_IO
    // Send the data
    if (!isnan(currentTemperature)) {
      adafruitIo.sendTemperature(currentTemperature);
    }
    
    if (!isnan(currentHumidity)) {
      adafruitIo.sendHumidity(currentHumidity);
    }
#endif

#ifdef DEEP_SLEEP_ENABLE
    SERIALPRINT("Sleeping ");
    SERIALPRINTLN(millis() - startTime);

    // Put the DTH sensor in the right state before sleep.
    // Otherwise when we wake up a read error happnes.
    sensorDht.Reset();

    // Find the time when we need to wake up again
    // we want to run every 15 mins
    uint8_t currentMinute;
    uint8_t currentSecond;
    uint8_t nextFifteenMinute;
    currentMinute = theTime.getMinute();
    currentSecond = theTime.getSecond();
    
    // Find the next 15 min interval
    // The following takes 5.8us
    nextFifteenMinute = (currentMinute + MEASUREMENT_INTERVAL_MIN) / MEASUREMENT_INTERVAL_MIN * MEASUREMENT_INTERVAL_MIN;
    // The following take 5.6us
    //nextFifteenMinute = currentMinute % MEASUREMENT_INTERVAL_MIN;
    //nextFifteenMinute = MEASUREMENT_INTERVAL_MIN - nextFifteenMinute + currentMinute;
    
    digitalWrite(2, LOW);
    // Go to deep sleep
    ESP.deepSleep((((nextFifteenMinute - currentMinute) * 60) + (60 + 1 - currentSecond)) * 1e6);
    //ESP.deepSleep(10 * 1e6);
      
    // Wait for the sleep to cut in
    delay(60000);
#else
    sensorDht.Loop();
#endif
  }

  // Calculate how long to delay for so we loop every minute
  uint8_t currentSecond;
  currentSecond = theTime.getSecond();
  currentSecond = 60 - currentSecond;
  delay(currentSecond * 1000);
}
