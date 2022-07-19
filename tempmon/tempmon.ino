/**

Temperature Monitor

Featureing 
- Read temperature sensor
- Send data to cloud
- Deep sleep for battery power
- Http update

*/

#define ENABLE_ADAFRUIT_IO 1

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

#if 0
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
#endif

utilTime theTime;
utilHttpUpdate httpUpdater;
utilDht sensorDht;

#if defined(ENABLE_ADAFRUIT_IO)
utilAdafruitIo adafruitIo;
#endif

float currentTemperature;
float currentHumidity;

void setup() {

  // Setup serial port
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  while (!Serial) { ; }
  Serial.println();
  Serial.println("TempMon Starting...");

  // Set the hostname
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

  // Setup wifi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  //wifiMulti.addAP(WIFI_SSID_ALT1, WIFI_PASS_ALT1);

  // Wait for connect
  Serial.print("WiFi connecting .");
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  

  // Get NTP time
  theTime.Setup();

#if ENABLE_ADAFRUIT_IO
  // Setup cloud
  adafruitIo.Setup();

  // Check for updates if flag has been set
  adafruitIo.requestCommandUpdate();
  while (adafruitIo.WaitForMessage()) {
    delay(10);
  }
  
  if (adafruitIo.getCommandUpdate()) {
    Serial.println("Updating - yes");
    httpUpdater.Setup();
  }
  else {
    Serial.println("Updating - not");
  }
#endif

  // Setup the sensor
  sensorDht.Setup();
}

void loop() {
  // Update the time  
  theTime.Loop();

  // Check the time, we only send on 15 min intervals
  uint8_t currentMinute;
  currentMinute = theTime.getMinute();
  Serial.println(currentMinute);
  if (currentMinute % MEASUREMENT_INTERVAL_MIN == 0) {
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

    // Find the time when we need to wake up again
    // we want to run every 15 mins
    // TODO
  
    
    Serial.println(" Sleeping");
    
    // Put the DTH sensor in the right state before sleep.
    // Otherwise when we wake up a read error happnes.
    sensorDht.Reset();
    
    // Go to deep sleep
    ESP.deepSleep(DEEP_SLEEP_TIME_US);
    
    // Wait for the sleep to cut in
    delay(60000);
#else
    sensorDht.Loop();
#endif
  }

  // Wait for 1 min
  //ESP.deepSleep(30 * 1e6);
  // Calculate how long to delay for so we loop every minute
  Serial.print(currentMinute);
  Serial.println(" Looping");
  uint8_t currentSecond;
  currentSecond = theTime.getSecond();
  currentSecond = 60 - currentSecond;
  delay(currentSecond * 1000);
}
