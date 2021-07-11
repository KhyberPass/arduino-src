#ifndef _UTIL_DHT_H_
#define _UTIL_DHT_H_

class utilDht
{
public:
  utilDht(void);
  ~utilDht(void);

  void Setup(void);
  void Loop(void);

  float getTemperature(void);
  float getHumidity(void);

private:
  
};

#endif // _UTIL_DHT_H_
