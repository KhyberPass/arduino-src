/**

Temperature Monitor

Featureing 
- Read temperature sensor
- Send data to cloud
- Deep sleep for battery power
- Http update

*/

#include <Arduino.h>

// For WiFi
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

// For HTTP Update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "tempmon-cred.h"
#include "util-time.h"
#include "util-httpupdate.h";
#include "util-dht.h";
#include "util-adafruitio.h"

// Deep sleep timeout 15 mins
#define DEEP_SLEEP_TIME_US (15 * 60 * 1e6)

ESP8266WiFiMulti WiFiMulti;
utilTime theTime;
utilHttpUpdate httpUpdate;
utilDht sensorDht;
utilAdafruitIo adafruitIo;

float currentTemperature;
float currentHumidity;

void setup() {

  // Setup serial port
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  while (!Serial) { ; }
  Serial.println();
  Serial.println("TempMon Starting...");

 
  // Setup wifi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

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

  // Setup cloud
  adafruitIo.Setup();

  // Check for updates if flag has been set
  adafruitIo.requestCommandUpdate();
  while (adafruitIo.WaitForMessage()) {
    delay(10);
  }
  
  if (adafruitIo.getCommandUpdate()) {
    Serial.println("Updating - yes");
    httpUpdate.Setup();
  }
  else {
    Serial.println("Updating - not");
  }
  
  // Read the sensor
  sensorDht.Setup();
  currentTemperature = sensorDht.getTemperature();
  currentHumidity = sensorDht.getHumidity();

  // Check the time, we only send on 15 min intervals
  // TODO
  
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

  
  // Go to deep sleep
  if (adafruitIo.getCommandSleep()) {
    ESP.deepSleep(DEEP_SLEEP_TIME_US);
  }
}

void loop() {

  Serial.println("Running loop");
  
  theTime.Loop();

  delay(5000);
}
