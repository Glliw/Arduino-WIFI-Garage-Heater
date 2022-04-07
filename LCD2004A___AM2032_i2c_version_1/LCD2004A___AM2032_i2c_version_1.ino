// this code combines the LCD2004A i2c display with the output of the AM2032 temp+humidity sensor.

// Vcc / Vdd - > 5v (red)
// GND - > GND (green)
// SCL - > SCL (orange)
// SDA - > SDA (blue)
// For AM2032, a 10k ohm resistor is used between SCL/Vdd and SDA/Vdd.  No extra devices needed for the LCD display.

// Add required libraries
#include <AM232X.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// add Blynk.Cloud Server
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL7DhvBTx-"
#define BLYNK_DEVICE_NAME "Garage Thermostat v2"
#define BLYNK_AUTH_TOKEN "gSWaQosYSG07u8FA8do8pVjY_puexq4Z"
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "UCFKNIGHTS";   // WiFi credentials.
char pass[] = "scooter1";     // Set password to "" for open networks.
BlynkTimer timer;
BLYNK_WRITE(V0)
{
  int value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
}

// Define variable types
float AM2320_TempC;
float AM2320_TempF;
float AM2320_Humidity;
float HeatIndex;
float c1;
float c2;
float c3;
float c4;
float c5;
float c6;
float c7;
float c8;
float c9;

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7);  //0x27 is the I2c bus address of the LCD's "backpack" by default.  Version used is not able to be changed, but some are.
AM232X AM2320;

void setup() {
  Serial.begin(115200);
  Serial.println(auth);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  timer.setInterval(1000L, myTimerEvent);
  
  Serial.println(__FILE__);  //__FILE__ is a preprocessor macro that expands to full path to the current file. __FILE__ is useful when generating log statements, error messages intended for programmers, when throwing exceptions, or when writing debugging code.
  Serial.print("AM232X Library Version: ");
  Serial.println(AM232X_LIB_VERSION);
  Serial.println("");
  delay (100);

// Check the AM2320 sensor.
if (! AM2320.begin() ) {
    Serial.println("Sensor not found");
    while (1); }
  AM2320.wakeUp();
  delay(100);
  Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (F)");

//start the 20x4 LCD screen
  lcd.begin (20,4); // for 20 x 4 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("THERMOSTAT STARTING");
  lcd.setCursor(0,1);
  delay(500);
  lcd.print("PLEASE WAIT.........");
  delay(500);
  lcd.setCursor(0,2);
  lcd.print("....................");
  delay(500);
  lcd.setCursor(0,3);
  lcd.print("....................");
  delay(1000);
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("Set Point: ");
  lcd.setCursor(0,1);
  lcd.print("Temp: "); 
  lcd.setCursor(0,2);
  lcd.print("Heat Index: ");
  lcd.setCursor(0,3);
  lcd.print("Humidity: ");    
  }

void ReadAM2320() {
  Serial.print("AM2320, \t");
  int status = AM2320.read();
  switch (status){
    case AM232X_OK:
      Serial.print("OK,\t");
      break;
    default:
      Serial.print(status);
      Serial.print("\t");
      break;
  }
  // define variables for temp and humidity, convert to Fahrenheit.
  AM2320_TempC = AM2320.getTemperature();
  AM2320_TempF = ((AM2320_TempC * 1.8) + 32) ;
  AM2320_Humidity = AM2320.getHumidity();
  Serial.print(AM2320_Humidity , 1);
  Serial.print(",\t");
  Serial.println(AM2320_TempF , 1);
  
  // print to serial monitor AM2320 data, sensor only returns one decimal.
 // Serial.print(AM2320.getHumidity(), 1);
 // Serial.print(",\t");
 // Serial.println(AM2320.getTemperature(), 1);
  delay(1000);
}

void ComputeHeatIndex(){ //https://en.wikipedia.org/wiki/Heat_index
  if((70 < AM2320_TempF < 115) && (0 < AM2320_Humidity < 80)){
    float c1 = 0.363445176;
    float c2 = 0.988622465;
    float c3 = 4.777114035;
    float c4 = -0.114037667;
    float c5 = -0.000850208;
    float c6 = -0.020716198;
    float c7 = 0.000687678;
    float c8 = 0.000274954;
    float c9 = 0;
    HeatIndex = c1 + (c2 * AM2320_TempF) + (c3 * AM2320_Humidity) + (c4 * AM2320_TempF * AM2320_Humidity) + 
    (c5 * (pow(AM2320_TempF,2)) + (c6 * (pow(AM2320_Humidity,2))) + (c7 * (pow(AM2320_TempF,2))) * AM2320_Humidity) +
    (c8 * AM2320_TempF * (pow(AM2320_Humidity,2))) ;
  }
  else if (AM2320_TempF <=70)
    HeatIndex = AM2320_TempF;
  
}



void UpdateLCD(){
  lcd.setCursor(14,0);
  lcd.print("75.0 F");
  lcd.setCursor(14,1);
  lcd.print(AM2320_TempF, 1);
  lcd.print(" F");
  lcd.setCursor(14,2);
  lcd.print(HeatIndex, 1);
  lcd.print(" F");
  lcd.setCursor(14,3);
  lcd.print(AM2320_Humidity, 1);
  lcd.print(" %");
}

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}



void loop() {
  Blynk.run();
  timer.run();
  
  ReadAM2320();
  ComputeHeatIndex();
  UpdateLCD();
  Blynk.virtualWrite(V6, AM2320_TempF);
  Blynk.virtualWrite(V7, AM2320_Humidity);
}
