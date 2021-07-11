
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

  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, "http://192.168.111.100/tempmon.bin");
  // Or:
  //t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server", 80, "file.bin");

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
  
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
  
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}
