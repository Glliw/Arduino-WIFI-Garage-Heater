//The DHT11 code portion of this is heavily based on the sample code, "DHTtester".
//The LCD display code uses an I2C backpack design. https://alselectro.wordpress.com/2018/04/16/esp8266-wemos-d1-with-i2c-serial-lcd/
    // Vcc - > 5v
    // GND - > GND
    // SCL - > SCL
    // SDA - > SDA

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Reference doc: https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define ENABLE 14
#define DHTPIN 3
#define DHTTYPE DHT11
unsigned long Previous_Relay_Millis = 0;  // this variable will store the last time the relay was fired
const long Relay_Interval = 18000; // sets the delay before opening / closing the relay again to prevent nuisance signals.  180,000 = 3 minutes
unsigned long Previous_LCD_Millis = 0; // store the last time the LCD display updated
const long LCD_Interval = 2500;

LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address of backpack by default
DHT dht(DHTPIN,DHTTYPE);

void setup() {
  //lcd.clear();
  Serial.begin(9600);
  Serial.println(F("Starting the thermostat..."));
  
  lcd.begin(16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  //lcd.clear();
  pinMode(ENABLE, OUTPUT);
  lcd.print("Hello");
  dht.begin();
}


void loop() {
  Serial.println("starting the loop...");
  delay (500); //sensor is slow to react; readings take ~0.25s each and data is ~2s old
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  float tempF = dht.readTemperature(true); //(isFahrenheit = true)

  // Check if reading failed and exit early to try again.

  if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
    Serial.println(F("Failed to receive a reading from the DHT11 sensor."));
    return;
   
  }
    //Compute the heat index in Fahrenheit (the default)
  float heat_index_F = dht.computeHeatIndex(tempF, humidity);
  //Compute the heat index in Celsius (isFahrenheit = False)
  float heat_index_C = dht.computeHeatIndex(tempC, humidity, false);
  int int_humidity = int(humidity);
  
  
  Serial.print(F("Humidity: "));
  Serial.print(int_humidity);
  Serial.println(F("%"));
  Serial.print("Temperature: ");
  Serial.print(tempF);
  Serial.println(F("°F"));
  Serial.print(F("Heat Index: "));
  Serial.print(heat_index_F);
  Serial.println(F("°F"));


unsigned long Current_LCD_Millis = millis();
 if (Current_LCD_Millis - Previous_LCD_Millis >= LCD_Interval) {
  Previous_LCD_Millis = Current_LCD_Millis;  // this if statement "saves" the last time the action wast taken.
  //print the temperature in degrees Fahrenheit to the first line of the LCD display.
  
  lcd.setCursor(0,0); // Start in the top left
  lcd.print("Temp: ");
  lcd.print(tempF,1);    
  //lcd.print((char)223);  //This is required to correctly print the degrees symbol on the display.
  lcd.print("F");
  
  lcd.setCursor(0,1);  // Start in the bottom left
  lcd.print("Humidity: ");
  lcd.print(int_humidity);    
  lcd.print("%");
    }

  unsigned long Current_Relay_Millis = millis();

  if (Current_Relay_Millis - Previous_Relay_Millis >= Relay_Interval) {
    Previous_Relay_Millis = Current_Relay_Millis;  // this if statement "saves" the last time the action wast taken.
    if (tempF < 72) {
      Serial.println("The temp is too low.  Turn on Heater.");
      digitalWrite(ENABLE,HIGH);
    }
    else if (tempF > 72) {
      Serial.println("The temp is too high.  Turn off the Heater.");
      digitalWrite(ENABLE, LOW);
    }
    }
  
 
//  Serial.println("Fire the relay");
//  digitalWrite(ENABLE, HIGH);  // enables the relay
//  delay(5000); // wait 5 seconds
//  Serial.println("Turn off the relay");
//  digitalWrite(ENABLE, LOW); // disables the relay
//  delay(5000); // wait 5 seconds  

    
}
