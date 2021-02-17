//The DHT11 code portion of this is heavily based on the sample code, "DHTtester".
//The LCD display code uses an I2C backpack design. https://alselectro.wordpress.com/2018/04/16/esp8266-wemos-d1-with-i2c-serial-lcd/
    // Vcc - > 5v
    // GND - > GND
    // SCL - > SCL
    // SDA - > SDA

// WIFI control is through the Blynk IOT app.
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Reference doc: https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define ENABLE 14
#define DHTPIN 3
#define DHTTYPE DHT11
#define BLYNK_PRINT Serial

LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address of LCD backpack by default
DHT dht(DHTPIN,DHTTYPE);
BlynkTimer timer;

char auth[] = "eldr5r4R0yl_7dte5jJpQ8ANvSaaekpy"; // this is the "Auth Token" from Blynk to link to the project.
//WiFi credentials: (Set password to "" for open networks.)
char ssid[] = "UCFKNIGHTS";
char pass[] = "scooter1";


unsigned long Previous_Relay_Millis = 0;  // this variable will store the last time the relay was fired
const long Relay_Interval = 20000; // sets the delay before opening / closing the relay again to prevent nuisance signals.  20,000 = 20 seconds
unsigned long Previous_LCD_Millis = 0; // store the last time the LCD display updated
const long LCD_Interval = 5000;
unsigned long Previous_Serial_Millis = 0; // store the last time the LCD display updated
const long Serial_Interval = 5000;
static int HeatSetPoint;
float humidity;
float tempC;
float tempF;
unsigned long Previous_DHT11_Millis = 0;
const long DHT11_Interval = 2000;

BLYNK_CONNECTED() {
    Blynk.syncVirtual(V0,V8); // syncs the button push for forced on mode and the slider position
}

// define the slider input.
BLYNK_WRITE(V0) {
      HeatSetPoint = param.asInt();
      Serial.print("V0 Slider value is: ");
      Serial.println(HeatSetPoint);   
    }

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting the thermostat..."));
  lcd.begin(16,2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  //lcd.clear();
  pinMode(ENABLE, OUTPUT);
  lcd.setCursor(0,0);
  lcd.print("Starting the");
  lcd.setCursor(0,1);
  lcd.print("Thermostat...");
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  delay (4000);
  lcd.clear();
  delay(1000);
}


void loop() {
  Blynk.run();
  timer.run();
  Blynk.virtualWrite(V0, HeatSetPoint); 
  // delay (500); //sensor is slow to react; readings take ~0.25s each and data is ~2s old
  unsigned long Current_DHT11_Millis = millis();
  if (Current_DHT11_Millis - Previous_DHT11_Millis >= DHT11_Interval) {
    Previous_DHT11_Millis = Current_DHT11_Millis;  // this if statement "saves" the last time the action was taken.
    humidity = dht.readHumidity();
    tempC = dht.readTemperature();
    tempF = dht.readTemperature(true); //(isFahrenheit = true)
  }
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
  
unsigned long Current_Serial_Millis = millis();
  if (Current_Serial_Millis - Previous_Serial_Millis >= Serial_Interval) {
    Previous_Serial_Millis = Current_Serial_Millis;  // this if statement "saves" the last time the action was taken.
    Serial.println("starting the loop...");
    Serial.print(F("Humidity: "));
    Serial.print(int_humidity);
    Serial.println(F("%"));
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.println(F("°F"));
    Serial.print(F("Heat Index: "));
    Serial.print(heat_index_F);
    Serial.println(F("°F"));
    Blynk.virtualWrite(V5, int_humidity);  
    Blynk.virtualWrite(V6, tempF);
    Blynk.virtualWrite(V7, heat_index_F);
    
    
 }

//this section prints to the LCD
unsigned long Current_LCD_Millis = millis();
 if (Current_LCD_Millis - Previous_LCD_Millis >= LCD_Interval) {
  Previous_LCD_Millis = Current_LCD_Millis;  // this if statement "saves" the last time the action was taken.
  //print the temperature in degrees Fahrenheit to the first line of the LCD display.
  lcd.clear();
  lcd.setCursor(0,0); // Start in the top left
  lcd.print("Temp: ");
  lcd.print(tempF,1);    
  //lcd.print((char)223);  //This is required to correctly print the degrees symbol on the display.
  lcd.print("F");
  
  lcd.setCursor(0,1);  // Start in the bottom left
  lcd.print("Humidity: ");
  lcd.print(int_humidity);    
  lcd.print("%");
  
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set Point: ");
  lcd.print(HeatSetPoint);
  lcd.print("F");

  lcd.setCursor(0,1);
  lcd.print("Heat Index: ");
  lcd.print(heat_index_F);
  lcd.print("F");
    }

  // this section fires the relay to enable the heater based on the set point of the Blynk app's V1 slider.
  unsigned long Current_Relay_Millis = millis();
  if (Current_Relay_Millis - Previous_Relay_Millis >= Relay_Interval) {
    Previous_Relay_Millis = Current_Relay_Millis;  // this if statement "saves" the last time the action was taken.
    if (tempF < HeatSetPoint) {
      Serial.println("The temp is too low.  Turn on Heater.");
      digitalWrite(ENABLE,HIGH); //sends status of the relay to Blynk
      Blynk.virtualWrite(V1,"Heater On");
    }
    else if (tempF > HeatSetPoint) {
      Serial.println("The temp is too high.  Turn off the Heater.");
      digitalWrite(ENABLE, LOW);
      Blynk.virtualWrite(V1,"Heater Off");  //sends status of the relay to Blynk
    }
    }
  
 
}
