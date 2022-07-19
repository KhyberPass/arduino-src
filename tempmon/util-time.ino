#include <ezTime.h>

int previousMinute = 0;
bool timeChanged = true;
Timezone myTime;

utilTime::utilTime()
{
}
utilTime::~utilTime()
{
}

void utilTime::Setup(void)
{
  bool status;
  
  setDebug(INFO);
  
  status = waitForSync();
  
  if (status == false) {
    Serial.println("Time sync failed");
  }
  
  myTime.setLocation("Australia/Sydney");
  Serial.println("Time: " + myTime.dateTime());

  previousMinute = minute();
}

void utilTime::Loop(void)
{
  // Call the events system, this will periodically
  // do a NTP update
  events();

  // Check if the time minutes has changed, if it has
  // then flag that the time has changed
  if (previousMinute != minute())
  {
    //Serial.println("Time changed " + myTime.dateTime());
    timeChanged = true;
    previousMinute = minute();
  }
}

bool utilTime::HasChanged() {
  if (timeChanged) 
  {
    timeChanged = false;
    return true;
  }
  return false;
}

String utilTime::Get() {
  return myTime.dateTime("H:i");
}

String utilTime::HoursGet() {
  return myTime.dateTime("H");
}

String utilTime::MinutesGet() {
  return myTime.dateTime("i");
}

uint8_t utilTime::getMinute() {
  return myTime.minute();
}

uint8_t utilTime::getSecond() {
  return myTime.second();
}
