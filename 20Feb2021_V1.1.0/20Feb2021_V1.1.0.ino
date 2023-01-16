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

LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7); // 0x27 is the I2C bus address of LCD backpack by default
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

char auth[] = "eldr5r4R0yl_7dte5jJpQ8ANvSaaekpy"; // this is the "Auth Token" from Blynk to link to the project.
//WiFi credentials: (Set password to "" for open networks.)
char ssid[] = "UCFKNIGHTS";
char pass[] = "scooter1";


unsigned long Previous_Relay_Millis = 0;  // this variable will store the last time the relay was fired
const long Relay_Interval = 1000; // sets the delay before opening / closing the relay again to prevent nuisance signals.  20,000 = 5 seconds
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
const long HTRButton_Interval = 3600000; // 1 hour

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0, V8); // syncs the button push for forced on mode and the slider position
}

// define the slider input.

void tempcontrolled() {
  ControlStatus = "Temperature Controlled";
  Serial.print("The heater is now controlled by temperature.");
  ButtonPressedStartTime = millis();
  unsigned long Current_Relay_Millis = millis();
  if (Current_Relay_Millis - Previous_Relay_Millis >= Relay_Interval) {
    Previous_Relay_Millis = Current_Relay_Millis;  // this if statement "saves" the last time the action was taken.
    if (tempF < HeatSetPoint) {
      Serial.print("The V8 Button set point is: ");
      Serial.println(V8Status);
      Serial.println("The temp is too low.  Turn on Heater.");
      digitalWrite(ENABLE, HIGH); //sends status of the relay to Blynk
      Blynk.virtualWrite(V1, "Heater On");
      Blynk.virtualWrite(V8, 0);
      tempcontrolled();
    }
    else if (tempF > HeatSetPoint) {
      Serial.print("The V8 Button set point is: ");
      Serial.println(V8Status);
      Serial.println("The temp is too high.  Turn off the Heater.");
      //      TotalHeaterOnTime = TotalHeaterOnTime + LastHeaterOnTime;
      digitalWrite(ENABLE, LOW);
      Blynk.virtualWrite(V8, 0);
      Blynk.virtualWrite(V1, "Heater Off"); //sends status of the relay to Blynk
      tempcontrolled();
    }
  }

}
void buttoncontrolled() {
  ControlStatus = "Button Controlled";
  Serial.println("The Heater has been turned on by the forced ON command.");
  digitalWrite(ENABLE, HIGH);
  Blynk.virtualWrite(V1, "Heater On");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HEATER ON 1 HR.");
  ButtonOnTime = millis() - ButtonPressedStartTime;
  Serial.print("Button on time: ");
  Serial.println(ButtonOnTime);
  unsigned long Current_HTRButton_Millis = millis();
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
      digitalWrite(ENABLE, LOW);
      tempcontrolled();
    }
    else {
      tempcontrolled();
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESUME TEMP ");
    lcd.setCursor(0, 1);
    lcd.print("CONTROLLED");
    delay(3000);
  }
}

BLYNK_WRITE(V0) {
  HeatSetPoint = param.asInt();
  Serial.print("V0 Slider value is: ");
  Serial.println(HeatSetPoint);
  tempcontrolled();
}

BLYNK_WRITE(V8) {
  int V8Status = param.asInt();
  if (V8Status == 1) {
    buttoncontrolled();
  }
  else if (V8Status == 0) {
    tempcontrolled();
  }
  Serial.print("V8 Button value is: ");
  Serial.println(V8Status);

}


void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting the thermostat..."));
  lcd.begin(16, 2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  //lcd.clear();
  pinMode(ENABLE, OUTPUT);
  lcd.setCursor(0, 0);
  lcd.print("Starting the");
  lcd.setCursor(0, 1);
  lcd.print("Thermostat...");
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  delay (2000);
  lcd.clear();
  delay(1000);
}


void loop() {
  Blynk.run();
  timer.run();

  // delay (500); //sensor is slow to react; readings take ~0.25s each and data is ~2s old
  unsigned long Current_DHT11_Millis = millis();
  if (Current_DHT11_Millis - Previous_DHT11_Millis >= DHT11_Interval) {
    Previous_DHT11_Millis = Current_DHT11_Millis;  // this if statement "saves" the last time the action was taken.
    humidity = dht.readHumidity();
    tempC = dht.readTemperature();
    tempF = dht.readTemperature(true); //(isFahrenheit = true)
    Blynk.virtualWrite(V0, HeatSetPoint);
    Serial.print("Control Status: ");
    Serial.println(ControlStatus);
  }

  // Check if reading failed and exit early to try again.
  if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
    unsigned long Current_DHT11Timeout_Millis = millis();
    Serial.println(F("Failed to receive a reading from the DHT11 sensor."));
    if (Current_DHT11Timeout_Millis - Previous_DHT11Timeout_Millis >= DHT11Timeout_Interval) {
      Previous_DHT11Timeout_Millis = Current_DHT11Timeout_Millis;  // this if statement "saves" the last time the action was taken.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("NO DATA FROM ");
      lcd.setCursor(0, 1);
      lcd.print("SENSOR. RETRY.");
      HeatSetPoint = 40;
      Blynk.virtualWrite(V1, "Heater Off via slider - No data from sensor."); //sends status of the relay to Blynk
    }
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

  // this section fires the relay to enable the heater based on the set point of the Blynk app's V1 slider.


}
