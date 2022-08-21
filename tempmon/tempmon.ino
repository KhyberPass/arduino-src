/**

Temperature Monitor

Featuring
- Read temperature sensor
- Send data to cloud
- Deep sleep for battery power
- Http update

*/

#define DEEP_SLEEP_ENABLE 1
//#define ADAFRUIT_IO_ENABLE 1
#define MQTT_ENABLE 1
//#define WAIT_TIME_SYNC 1
//#define STATIC_IP 1
#define WIFI_FAST_CONNECT 1
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

#include <ArduinoJson.h>

#include "tempmon-cred.h"
#include "util-time.h"
#include "util-httpupdate.h"
#include "util-dht.h"
#include "util-adafruitio.h"
#include "util-mqtt.h"

#define MEASUREMENT_INTERVAL_MIN 15
#define WAKE_EARLY_MIN 1
#define DEEP_SLEEP_TIME_US ((MEASUREMENT_INTERVAL_MIN - WAKE_EARLY_MIN) * 60 * 1e6)

#if defined(ESP8266)
ESP8266WiFiMulti WiFiMulti;
#elif defined(ESP32)
WiFiMulti WiFiMulti;
#endif

#ifdef STATIC_IP
// Set your Static IP address
IPAddress WifiLocalIp(192, 168, 111, 62);
// Set your Gateway IP address
IPAddress WifiGateway(192, 168, 111, 1);

IPAddress WifiSubnet(255, 255, 255, 0);
IPAddress WifiDnsPrimary(192, 168, 111, 1);
IPAddress WifiDnsSecondary(192, 168, 111, 1);
#endif
#ifdef STATIC_IP_WORK
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

#ifdef ADAFRUIT_IO_ENABLE
utilAdafruitIo adafruitIo;
#endif

#ifdef MQTT_ENABLE
utilMqtt mqttClient;
#endif

// channel = 6
//uint8_t wifiBssid[] = {0x5C,0xB1,0x3E,0x5F,0x41,0xFC};
//uint8_t wifiBssid[] = {0x00,0x14,0x6C,0x62,0xC4,0x3E};

int configProcessUpdate;
float currentTemperature;
float currentHumidity;
unsigned long startTime;
unsigned long stampTime;
unsigned long diffTime;
char printbuffer[64];

char dataJsonStr[256];
StaticJsonDocument<64> dataJson;

void progressTimePrint(char* message)
{
  diffTime = millis() - stampTime;
  stampTime = millis();
  sprintf(printbuffer, "%s %d %d", message, stampTime, diffTime);
  SERIALPRINTLN(printbuffer);
}


void setup() {
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  startTime = millis();
  stampTime = startTime;

  // For a fast start this will turn on WiFi if it is not already
  WiFi.mode(WIFI_STA);

#ifdef DEBUG_LOCAL
  // Setup serial port
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  while (!Serial) { ; }
  Serial.println();
  Serial.println("TempMon Starting...");
#endif

  // Set the hostname
  // kitchenroof = 80:7D:3A:11:A0:C9
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

  // Setup wifi

#ifdef STATIC_IP
  // Configures static IP address
  if (!WiFi.config(WifiLocalIp, WifiGateway, WifiSubnet, WifiDnsPrimary, WifiDnsSecondary)) {
    SERIALPRINTLN("STA Failed to configure");
  }
#endif

#if 0
  WiFi.mode(WIFI_STA);

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

#ifdef WIFI_FAST_CONNECT
  WiFi.begin();
  WiFi.waitForConnectResult();
#endif

  // If we did not connect in fast mode then
  // do the normal conenct
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting slow");

    if (WiFi.SSID() != WIFI_SSID)
    {
      // Save the WiFi SSID and password in flash for
      // fast connect next time
      WiFi.persistent(true);
    }
        
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    //WiFi.begin(WIFI_SSID_ALT1, WIFI_PASS_ALT1);
    //WiFi.begin(WIFI_SSID_ALT1, WIFI_PASS_ALT1, 11, wifiBssid);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
  }
  
#endif
  stampTime = millis();

  SERIALPRINT("WiFi connected ");
  SERIALPRINTLN(stampTime - startTime);
  SERIALPRINT("IP address: ");
  SERIALPRINTLN(WiFi.localIP());

#ifdef ADAFRUIT_IO_ENABLE
  // Setup cloud
  adafruitIo.Setup();
#endif

#ifdef MQTT_ENABLE
  // Setup mqtt now and subscribe for updates
  // so any pubs will arrive will we are setting
  // up the time as this take about 700ms
  mqttClient.Setup();
  mqttClient.Connect();
  mqttClient.ConfigSubscribe();
#endif

  progressTimePrint("IO setup");

  // Get NTP time
  theTime.Setup();
  progressTimePrint("Time setup");

  // Setup the sensor
  sensorDht.Setup();
  progressTimePrint("Sensor setup");

#ifdef MQTT_ENABLE
  mqttClient.Loop();
  mqttClient.Loop();
  int config;
  config = mqttClient.ConfigCheck();
  
  if (config) // == "update"
    configProcessUpdate = true;
    
#endif
  
  digitalWrite(2, HIGH);
  progressTimePrint("Setup done");
}

void loop() {
  // Update the time  
  theTime.Loop();
  progressTimePrint("Time loop");

  if (configProcessUpdate)
    httpUpdater.Setup();

  // Check the time. Sometimes we wake from sleep a bit
  // early. If we are close then just delay, otherwise
  // continue and we will go to sleep a bit more.
  uint8_t currentMinute;
  currentMinute = theTime.getMinute();

  // If we are within 1 min from the wake interval then 
  // just delay a bit
  if ((currentMinute % MEASUREMENT_INTERVAL_MIN) == (MEASUREMENT_INTERVAL_MIN - 1))
  {
    SERIALPRINT("Waiting ");
    SERIALPRINTLN(theTime.getSecond());
    do
    {
      delay(100);
      currentMinute = theTime.getMinute();
    }
    while (currentMinute % MEASUREMENT_INTERVAL_MIN != 0);
  }

#ifdef WAIT_TIME_SYNC
  // Check the time, we only send on 15 min intervals
  uint8_t currentMinute;
  currentMinute = theTime.getMinute();
  if (currentMinute % MEASUREMENT_INTERVAL_MIN == 0)
#endif
  {
    // Read the sensors
    currentTemperature = sensorDht.getTemperature();
    SERIALPRINT(F("Temperature: "));
    SERIALPRINT(currentTemperature);
    SERIALPRINTLN(F("°C"));
    
    currentHumidity = sensorDht.getHumidity();
    SERIALPRINT(F("Humidity: "));
    SERIALPRINT(currentHumidity);
    SERIALPRINTLN(F("%"));

    progressTimePrint("Sensor read");

#ifdef ADAFRUIT_IO_ENABLE
    // Send the data
    if (!isnan(currentTemperature)) {
      adafruitIo.sendTemperature(currentTemperature);
    }
    
    if (!isnan(currentHumidity)) {
      adafruitIo.sendHumidity(currentHumidity);
    }
#endif

#ifdef MQTT_ENABLE
    // Send the data
    dataJson["time"] = theTime.Now();
    dataJson["temperature"] = currentTemperature;
    dataJson["humidity"] = currentHumidity;
    dataJson["debug"] = millis() - startTime;

    serializeJson(dataJson, dataJsonStr);
    mqttClient.Publish(dataJsonStr);
#endif

    progressTimePrint("IO loop");

#ifdef DEEP_SLEEP_ENABLE
    progressTimePrint("Sleeping");

    // Put the DTH sensor in the right state before sleep.
    // Otherwise when we wake up a read error might happen.
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
    ESP.deepSleep((((nextFifteenMinute - currentMinute) * 60) + (60 - currentSecond)) * 1e6);
    //ESP.deepSleep(20 * 1e6);
      
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
