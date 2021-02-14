//The DHT11 code portion of this is heavily based on the sample code, "DHTtester".
//The LCD display code is borrowed from a random site download and adapted herein.


#include <LiquidCrystal.h> 
#include <DHT.h>

#define ENABLE 9
#define DHTPIN 8
#define DHTTYPE DHT11
#define Contrast 75
unsigned long Previous_Relay_Millis = 0;  // this variable will store the last time the relay was fired
const long Relay_Interval = 180000; // sets the delay before opening / closing the relay again to prevent nuisance signals.  180,000 = 3 minutes

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
DHT dht(DHTPIN,DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT11 test beginning"));
  analogWrite(6, Contrast);
  dht.begin();
  lcd.begin(16,2);
  pinMode(ENABLE, OUTPUT);
}

void loop() {
  delay (2500); //sensor is slow to react; readings take ~0.25s each and data is ~2s old
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

  //print the temperature in degrees Fahrenheit to the first line of the LCD display.
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(tempF,1);    
  lcd.print((char)223);  //This is required to correctly print the degrees symbol on the display.
  lcd.print("F");
  
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(int_humidity);    
  lcd.print("%");

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
