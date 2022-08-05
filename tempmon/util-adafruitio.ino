#ifdef ADAFRUIT_IO_ENABLE

#include "AdafruitIO_WiFi.h"

#include "tempmon-cred.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Set up the feed
AdafruitIO_Feed *bedrooftemp = io.feed("first-floor-roof-temperature");
AdafruitIO_Feed *feedFirstFloorRoofHumidity = io.feed("first-floor-roof-humidity");
AdafruitIO_Feed *feedControlCommand = io.feed("control-command");


utilAdafruitIo::utilAdafruitIo()
{
  messageReceived = false;
}
utilAdafruitIo::~utilAdafruitIo()
{
}

void utilAdafruitIo::Setup(void)
{
  // Connect to io.adafruit.com
  //Serial.print("Connecting to Adafruit IO");
  io.connect();

  // Wait for a connection
  while (io.status() < AIO_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }

  // We are connected
  //Serial.println();
  //Serial.println(io.statusText());

  // Setup the command message handler
  feedControlCommand->onMessage(feedControlCommandMessageHandler);
}

void utilAdafruitIo::Loop(void)
{

}

void utilAdafruitIo::sendTemperature(float temperature)
{
  // io.adafruit.com processes any incoming data
  io.run();

  // Save to feed on Adafruit IO
  bedrooftemp->save(temperature);  
}

void utilAdafruitIo::sendHumidity(float humidity)
{
  // io.adafruit.com processes any incoming data
  io.run();

  // Save to feed on Adafruit IO
  feedFirstFloorRoofHumidity->save(humidity);  
}

void utilAdafruitIo::requestCommandUpdate(void)
{
  messageReceived = false;
  feedControlCommand->get();

  Serial.println("requestCommandUpdate after get");
}

bool utilAdafruitIo::getCommandUpdate(void)
{
  Serial.println("getCommandUpdate");
  if (adafruitIo.messageString == "first-floor-roof-update") {
    return true;
  }
  return false;
}

bool utilAdafruitIo::getCommandSleep(void)
{
  Serial.println("getCommandSleep");
  if (adafruitIo.messageString == "first-floor-roof-nosleep") {
    return false;
  }
  return true;
}

void utilAdafruitIo::setMessageState(bool state)
{
  messageReceived = state;
}

bool utilAdafruitIo::WaitForMessage(void)
{
  // io.adafruit.com processes any incoming data
  io.run();

  if (messageReceived) {
    messageReceived = false;
    return true;
  }
  return false;
}

void feedControlCommandMessageHandler(AdafruitIO_Data *data)
{
  Serial.println("feedControlCommandMessageHandler");
  // Get the command string
  adafruitIo.messageString = data->toString();
  Serial.println(adafruitIo.messageString);

  // Signal that we received a command
  adafruitIo.setMessageState(true);
}
#endif
