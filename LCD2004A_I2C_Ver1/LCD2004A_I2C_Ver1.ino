// this code is a good example showing how to use a I2C LCD setup with the Wemos D1 R1 board.
// Vcc - > 5v
// GND - > GND
// SCL - > SCL
// SDA - > SDA


/* Demonstration sketch for PCF8574T I2C LCD Backpack 
Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
//#include <LCD.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address of backpack by default

void setup()
{
  // activate LCD module
  lcd.begin (20,4); // for 20 x 4 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  Serial.begin(9600);
  
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.setCursor(14,1);
  lcd.print("63.0 F");
  
  lcd.setCursor(0,3);
  lcd.print("Humidity: ");
  lcd.setCursor(14,3);
  lcd.print("34.4 %");
  
  lcd.setCursor(0,0);
  lcd.print("Set Point: ");
  lcd.setCursor(14,0);
  lcd.print("65.0 F");
  
  lcd.setCursor(0,2);
  lcd.print("Heat Index: ");
  lcd.setCursor(14,2);
  lcd.print("64.2 F");
}
void loop() {
  // put your main code here, to run repeatedly:

}
