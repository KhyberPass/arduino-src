/*

MQTT Pub Sub wrapper

Uses https://github.com/knolleary/pubsubclient
*/

//#define MQTT_VERSION MQTT_VERSION_3_1
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

int configUpdate;

utilMqtt::utilMqtt()
{
}
utilMqtt::~utilMqtt()
{
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp((char*)payload, "update"))
    configUpdate = true;
}

void utilMqtt::Setup(void)
{
  configUpdate = false;
  
  //client.setServer("192.168.111.100", 1883);
  client.setServer("192.168.0.203", 1883);
  client.setCallback(callback);
}

void utilMqtt::Loop(void)
{
  client.loop();
}

void utilMqtt::Connect(void)
{
  if (!client.connected())
  {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (!client.connect(clientId.c_str()))
    {
      Serial.print("MQTT connect failed, rc=");
      Serial.println(client.state());
    }
  }
}

void utilMqtt::Publish(float value)
{
  if (client.connected())
  {
    char temp[5];
    dtostrf(value, 4, 1, temp);

    client.publish("sensor/kitchenroof/temperature", temp);

    // Need to do a disconenct so the send is flushed
    // before we sleep
    client.disconnect();
  }
}

// Subscribe to config messages that we may get
// at boot time to change the execution.
// Like run the updater server
void utilMqtt::ConfigSubscribe()
{
  if (client.connected())
  {
    client.subscribe("sensor/kitchenroof/config");
  }
}

// Read if there have been an messages received that
// may change the config
int utilMqtt::ConfigCheck()
{
  return configUpdate;
}
