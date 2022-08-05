/*

MQTT Pub Sub wrapper

Uses https://github.com/knolleary/pubsubclient
*/

//#define MQTT_VERSION MQTT_VERSION_3_1
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

utilMqtt::utilMqtt()
{
}
utilMqtt::~utilMqtt()
{
}

void utilMqtt::Setup(void)
{
  client.setServer("192.168.111.100", 1883);
  //client.setServer("192.168.0.203", 1883);
  //client.setCallback(callback);
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

    client.publish("sensor/temp/kitchenroof", temp);

    // Need to do a disconenct so the send is flushed
    // before we sleep
    client.disconnect();
  }
}
