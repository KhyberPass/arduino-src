#ifndef _UTIL_MQTT_H_
#define _UTIL_MQTT_H_

class utilMqtt
{
public:
  utilMqtt(void);
  ~utilMqtt(void);

  void Setup(void);
  void Loop(void);

  void Connect(void);
  void Publish(float);

  void ConfigSubscribe(void);
  int ConfigCheck(void);

private:

};

#endif // _UTIL_MQTT_H_
