//The DHT11 code portion of this is heavily based on the sample code, "DHTtester".
//The LCD display code uses an I2C backpack design. https://alselectro.wordpress.com/2018/04/16/esp8266-wemos-d1-with-i2c-serial-lcd/
// Vcc - > 5v
// GND - > GND
// SCL - > SCL
// SDA - > SDA

// WIFI control is through the "Blynk" IOT app, using "wifi provisioning".
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Reference doc: https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
#include <DHT.h>
#include <ESP8266WiFi.h>
                                  
#define EnableHeater 14
#define DHTPIN 3
#define DHTTYPE DHT11
#define BLYNK_TEMPLATE_ID "TMPL7DhvBTx-" //device name for unique ID
#define BLYNK_DEVICE_NAME "Garage Heater" //device name for unique ID
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7); // 0x27 is the I2C bus address of LCD backpack by default
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

unsigned long Previous_Relay_Millis = 0;  // this variable will store the last time the relay was fired
const long Relay_Interval = 60000; // sets the delay before opening / closing the relay again to prevent nuisance signals.  60,000 = 60 seconds (1.00 min)
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
unsigned long Previous_DHT11Timeout_Millis = 0;
const long DHT11Timeout_Interval = 5000;
unsigned long LastHeaterOnTime;
unsigned long TotalHeaterOnTime;
int V8Status;
unsigned long ButtonOnTime;
unsigned long ButtonPressedStartTime;
String ControlStatus;
unsigned long Previous_HTRButton_Millis;
unsigned long Current_HTRButton_Millis;
const long HTRButton_Interval = 3600000; // 1 hour = 3600000 ms
int int_humidity;
float heat_index_F;
float heat_index_C;
int ForceOnButton;

//V0 = Heat Set Point in degrees F, integer
//V1 = Heater Status Message, text
//V2 = Not used
//V3 = Control Method Status Message, text  
//V4 = Not used
//V5 = humidity, integer
//V6 = Temperature in degrees F, double xx.x

void GetDHT11() { // read the temperature and humidity from the DHT11 sensor
  unsigned long Current_DHT11_Millis = millis();
  if (Current_DHT11_Millis - Previous_DHT11_Millis >= DHT11_Interval) {  // take a reading every interval as defined by variable "DHT11_Interval"
    Previous_DHT11_Millis = Current_DHT11_Millis;  // this if statement "saves" the last time the action was taken.
    humidity = dht.readHumidity();
    tempC = dht.readTemperature();
    tempF = dht.readTemperature(true); //(isFahrenheit = true)
//    Blynk.virtualWrite(V0, HeatSetPoint);  //16Jan2023 - unclear to me why i put a data write here....commenting out for the time being
//    Serial.print("Control Status: ");
//    Serial.println(ControlStatus);
  }

  // Check if reading failed and exit early to try again.
  if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
    unsigned long Current_DHT11Timeout_Millis = millis();
    if (Current_DHT11Timeout_Millis - Previous_DHT11Timeout_Millis >= DHT11Timeout_Interval) {
      Previous_DHT11Timeout_Millis = Current_DHT11Timeout_Millis;  // this if statement "saves" the last time the action was taken.
      Serial.println(F("Failed to receive a reading from the DHT11 sensor."));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("NO DATA FROM ");
      lcd.setCursor(0, 1);
      lcd.print("SENSOR. RETRY.");
      HeatSetPoint = 35; // set a default value.
      Blynk.virtualWrite(V1, "Heater Off via slider - No data from sensor."); //sends status of the relay to Blynk
      digitalWrite(EnableHeater, LOW);
      Blynk.syncVirtual(V0);
    }
    return;
  }
  //Compute the heat index in Fahrenheit (the default)
  heat_index_F = dht.computeHeatIndex(tempF, humidity);
  //Compute the heat index in Celsius (isFahrenheit = False)
  heat_index_C = dht.computeHeatIndex(tempC, humidity, false);
  int_humidity = int(humidity);
  return;
}

//uses slider for temperature control
void tempcontrolled() {
  ControlStatus = "Temperature Controlled";
  Serial.println("The heater is now controlled by temperature.");
  Blynk.virtualWrite(V3, "Temperature Controlled");
  ButtonPressedStartTime = millis();
  unsigned long Current_Relay_Millis = millis();
  if (Current_Relay_Millis - Previous_Relay_Millis >= Relay_Interval) { // take a reading every interval as defined by variable "Relay_Interval". prevents excessive relay firing.
    Previous_Relay_Millis = Current_Relay_Millis;  // this if statement "saves" the last time the action was taken.
    
    if (Current_HTRButton_Millis < HTRButton_Interval) {
      return;
    }
    else
      if (tempF < HeatSetPoint) {
        Serial.print("The V8 Button set point is: ");
        Serial.println(V8Status);
        Serial.println("The temp is too low.  Turn on Heater.");
        digitalWrite(EnableHeater, HIGH); //sends status of the relay to Blynk
        Blynk.virtualWrite(V1, "Heater On");
        Blynk.virtualWrite(V8, V8Status);
        BLYNK_CONNECTED();
        return;
      }
      else if (tempF >= HeatSetPoint) {
        Serial.print("The V8 Button set point is: ");
        Serial.println(V8Status);
        Serial.println("The temp is too high.  Turn off the Heater.");
        //      TotalHeaterOnTime = TotalHeaterOnTime + LastHeaterOnTime;
        digitalWrite(EnableHeater, LOW);
        Blynk.virtualWrite(V8, V8Status);
        Blynk.virtualWrite(V1, "Heater Off"); //sends status of the relay to Blynk
        BLYNK_CONNECTED();
        return;
      }
      else {
        return;
      }
    return;
  }
  return;
}
void buttoncontrolled() {
  ControlStatus = "Button Controlled";
  Serial.println("The Heater has been turned on by the forced ON command.");
  Blynk.virtualWrite(V3, "Button Controlled");
  digitalWrite(EnableHeater, HIGH);
  Blynk.virtualWrite(V1, "Heater On");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HEATER ON 1 HR.");
  ButtonOnTime = millis() - ButtonPressedStartTime;
  Serial.print("Button on time: ");
  Serial.println(ButtonOnTime);
  Current_HTRButton_Millis = millis();
  if (Current_HTRButton_Millis - Previous_HTRButton_Millis >= HTRButton_Interval) {
    Previous_HTRButton_Millis = Current_HTRButton_Millis;  // this if statement "saves" the last time the action was taken.
    Serial.println("The Heater was on for 1 hour by the forced ON Command. Turning off heater.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HEATER OFF AFTER");
    lcd.setCursor(0, 1);
    lcd.print("1 HR ON.");
    delay(2000);
    if (tempF > HeatSetPoint) {
      Blynk.virtualWrite(V8, 0);
      Blynk.virtualWrite(V1, "Heater Off");
      digitalWrite(EnableHeater, LOW);
      BLYNK_CONNECTED();
      tempcontrolled();
    }
    else {
      BLYNK_CONNECTED();
      tempcontrolled();
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESUME TEMP ");
    lcd.setCursor(0, 1);
    lcd.print("CONTROLLED");
    delay(3000);
  }
  return;
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0, V8); // syncs the button push for forced on mode and the slider position
}

BLYNK_WRITE(V0) {
  HeatSetPoint = param.asInt();
  Serial.print("V0 Slider value is: ");
  Serial.println(HeatSetPoint);
  V8Status = 0;
  tempcontrolled();
  return;
}

BLYNK_WRITE(V8) {
  int V8Status = param.asInt();
  if (V8Status == 1) {
    buttoncontrolled();
  }
  else if (V8Status == 0) {
    digitalWrite(EnableHeater, LOW); 
    Blynk.virtualWrite(V1, "Heater Off");
    tempcontrolled();
  }
  Serial.print("V8 Button value is: ");
  Serial.println(V8Status);
  return;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  BlynkEdgent.begin();
  Serial.println(F("Starting the thermostat..."));
  lcd.begin(16, 2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  //lcd.clear();
  pinMode(EnableHeater, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print("Starting the");
  lcd.setCursor(0, 1);
  lcd.print("Thermostat...");
  dht.begin();
  
  delay (2000);
  lcd.clear();
  delay(1000);
}

void loop() {
  BlynkEdgent.run();
  GetDHT11();

  timer.run();
  //tempcontrolled();


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
    Serial.print("Heater Relay Status: ");
    Serial.println(digitalRead(EnableHeater));
    Serial.print("Heater Set point: ");
    Serial.print(HeatSetPoint);
    Serial.println("F");
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
    lcd.setCursor(0, 0); // Start in the top left
    lcd.print("Temp: ");
    lcd.print(tempF, 1);
    //lcd.print((char)223);  //This is required to correctly print the degrees symbol on the display.
    lcd.print("F");

    lcd.setCursor(0, 1); // Start in the bottom left
    lcd.print("Humidity: ");
    lcd.print(int_humidity);
    lcd.print("%");

    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Point: ");
    lcd.print(HeatSetPoint);
    lcd.print("F");

    lcd.setCursor(0, 1);
    lcd.print("Heat Index: ");
    lcd.print(heat_index_F);
    lcd.print("F");
  }
}
