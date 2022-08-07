
// For HTTP Update
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#define httpUpdate ESPhttpUpdate
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#endif

utilHttpUpdate::utilHttpUpdate()
{
}
utilHttpUpdate::~utilHttpUpdate()
{
}

void utilHttpUpdate::Setup(void)
{
  // Look for a update
  WiFiClient client;

  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  
  t_httpUpdate_return ret = httpUpdate.update(client, "http://192.168.111.100/espimage.bin");
  // Or:
  //t_httpUpdate_return ret = ESPhttpUpdater.update(client, "server", 80, "file.bin");

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
  
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
  
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}
