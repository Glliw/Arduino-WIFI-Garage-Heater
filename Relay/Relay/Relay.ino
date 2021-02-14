// This code's purpose is to fire a relay using a 5VDC source to control a 24VAC output (thermostat)
// This code is using a SRD-05VDC-SL-C relay

#define ENABLE 9

void setup() {
  pinMode(ENABLE, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  Serial.println("Fire the relay");
  digitalWrite(ENABLE, HIGH);  // enables the relay
  delay(5000); // wait 5 seconds
  Serial.println("Turn off the relay");
  digitalWrite(ENABLE, LOW); // disables the relay
  delay(5000); // wait 5 seconds

}
