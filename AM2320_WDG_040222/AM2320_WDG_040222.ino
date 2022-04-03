// April 2nd 2022 W. Gysi code based on standard AM2320 library example
//
//  AM2320 PIN layout             AM2315 COLOR
//  ============================================
//   bottom view  DESCRIPTION     COLOR
//       +---+
//       |o  |       1 VDD          RED
//       |o  |       2 SDA          YELLOW
//       |o  |       3 GND          BLACK
//       |o  |       4 SCL          GREY
//       +---+
//
// include 2-10k ohm pull up resistors between SDA, SCL and VDD.

#include "AM232X.h"

AM232X AM2320;

void setup()
{
  Serial.begin(9600);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(AM232X_LIB_VERSION);
  Serial.println();

  if (! AM2320.begin() )
  {
    Serial.println("Sensor not found");
    while (1);
  }
  AM2320.wakeUp();
  delay(100);

  Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (C)");
}


void loop()
{
  // READ DATA
  Serial.print("AM2320, \t");
  int status = AM2320.read();
  switch (status)
  {
    case AM232X_OK:
      Serial.print("OK,\t");
      break;
    default:
      Serial.print(status);
      Serial.print("\t");
      break;
  }
  // DISPLAY DATA, sensor only returns one decimal.
  Serial.print(AM2320.getHumidity(), 1);
  Serial.print(",\t");
  Serial.println(AM2320.getTemperature(), 1);

  delay(1000);
}


// -- END OF FILE --
