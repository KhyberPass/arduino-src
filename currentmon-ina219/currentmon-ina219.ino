#define ENABLE_DISPLAY 1

#include <Wire.h>
#include <Adafruit_INA219.h>

#ifdef ENABLE_DISPLAY
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
#endif

Adafruit_INA219 ina219;

int paddingTextLarge;
int paddingTextSmall;

void drawFloatEx(float floatNumber, uint8_t decimal, int32_t x, int32_t y, uint8_t font);

void setup(void) 
{
  Serial.begin(115200);
  while (!Serial) {
      delay(1);
  }
  Serial.println("Hello!");
  
#ifdef ENABLE_DISPLAY
  uint16_t calData[5] = { 476, 3159, 524, 3015, 7 };
  tft.setTouch(calData);

  tft.init();
  tft.setRotation(3);  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Current Meter\n");
#endif  

  // Set the I2C port pins
  Wire.setPins(13, 14);
  
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (! ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
#ifdef ENABLE_DISPLAY
    tft.println("Failed to find INA219 chip");
#endif
    while (1) { delay(10); }
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");

#ifdef ENABLE_DISPLAY
  tft.setTextDatum(BC_DATUM);
  tft.drawString("mA", 440, 180);

  // Draw the table header
  tft.setTextDatum(MC_DATUM);

  // Calculate the text padding for the worst case
  paddingTextLarge = tft.textWidth("88.8", 8);
  paddingTextSmall = tft.textWidth("8.888", 4);

  int xpos = 0;
  int i = 0;
  for (xpos = tft.width()/4; xpos < tft.width(); xpos += tft.width()/4)
  {
    tft.drawString("|", xpos, 275);
    tft.drawString("|", xpos, 300);
  }
  
  String headerMeasure[4] = {"Bus", "Shunt", "Load", "Power"};
  String headerUnits[4] = {"V", "mV", "V", "mW"};
  for (i = 0, xpos = tft.width()/4/2; xpos < tft.width(); i++, xpos += tft.width()/4)
  {
    tft.setTextDatum(MC_DATUM);
    tft.drawString(headerMeasure[i], xpos, 275);
    tft.setTextDatum(BL_DATUM);
    tft.drawString(headerUnits[i], xpos + 2 + paddingTextSmall/2, 320 - 7 + 2, 2);
  }

  // Font 4 h=26, 19 + 7
  // Font 2 h=16, 11.7 + 4.3
  // To make base align of font 2 to font 4
  // xpos-base - 7 + 4
/*  
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.drawString("-", 10, 60, 4);
  tft.drawString(" ", 10, 70, 4);
  int w = tft.textWidth(" ", 4);
  tft.drawNumber(w, 10, 120, 4);
*/
#endif
}

void loop(void) 
{
  float shuntvoltage_mV = 0;
  float busvoltage_V = 0;
  float current_mA = 0;
  float loadvoltage_V = 0;
  float power_mW = 0;

  shuntvoltage_mV = ina219.getShuntVoltage_mV();
  busvoltage_V = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage_V = busvoltage_V + (shuntvoltage_mV / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage_V, 5); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage_mV, 5); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage_V, 5); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA, 5); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW, 5); Serial.println(" mW");
  Serial.println("");

#ifdef ENABLE_DISPLAY
  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(paddingTextLarge);  
  drawFloatEx(current_mA, 1, tft.width()/2, 180, 8);

  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(paddingTextSmall);
  
  int xpos = tft.width()/4/2;
  drawFloatEx(busvoltage_V, 3, xpos, 320, 4);
  xpos += tft.width()/4;
  drawFloatEx(shuntvoltage_mV, 1, xpos, 320, 4);
  xpos += tft.width()/4;
  drawFloatEx(loadvoltage_V, 3, xpos, 320, 4);
  xpos += tft.width()/4;
  drawFloatEx(power_mW, 3, xpos, 320, 4);

  uint16_t x = 0, y = 0; // To store the touch coordinates

  bool pressed = tft.getTouch(&x, &y);

  // Draw a white spot at the detected coordinates
  if (pressed) {
    //tft.fillCircle(x, y, 2, TFT_WHITE);
    Serial.print("x,y = ");
    Serial.print(x);
    Serial.print(",");
    Serial.println(y);
  }
#endif

  delay(2000);
}

#ifdef ENABLE_DISPLAY
void drawFloatEx(float floatNumber, uint8_t decimal, int32_t x, int32_t y, uint8_t font)
{
  uint16_t pad = tft.getTextPadding();
  uint8_t datum = tft.getTextDatum();
  
  tft.setTextDatum(BR_DATUM);
  tft.setTextPadding(8);
  // If the number is not negative add a space so it does not jump around
  if (floatNumber < 0)
    tft.drawString("-", x - pad/2, y, font);
  else
    tft.drawString(" ", x - pad/2, y, font);
  tft.setTextDatum(datum);
  tft.setTextPadding(pad);
  tft.drawFloat(abs(floatNumber), decimal, x, y, font);
}
#endif
