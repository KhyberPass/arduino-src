#ifndef _SCLOCK_TIME_H_
#define _SCLOCK_TIME_H_

class utilTime
{
public:
  utilTime(void);
  ~utilTime(void);

  void Setup(void);
  void Loop(void);
  bool HasChanged();
  String Get();  
  String HoursGet();
  String MinutesGet();
  uint8_t getMinute();
  uint8_t getSecond();
  
private:
  
};

#endif // _SCLOCK_TIME_H_
