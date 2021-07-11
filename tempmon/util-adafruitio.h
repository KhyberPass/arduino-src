#ifndef _UTIL_ADAFRUITIO_H_
#define _UTIL_ADAFRUITIO_H_

#include "AdafruitIO_WiFi.h"

class utilAdafruitIo
{
public:
  utilAdafruitIo(void);
  ~utilAdafruitIo(void);

  void Setup(void);
  void Loop(void);

  bool WaitForMessage(void);
  
  void sendTemperature(float);
  void sendHumidity(float);

  void setMessageState(bool);
  void requestCommandUpdate(void);
  bool getCommandUpdate(void);
  bool getCommandSleep(void);

  String messageString;

private:
  bool messageReceived;

};

#endif // _UTIL_ADAFRUITIO_H_
