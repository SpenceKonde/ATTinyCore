/* This example demonstrates use of the internal temperature sensor.
|  Be sure to read the datasheet, particularly the part about how
\  the sensor is basically uncalibrated. */

void setup(){
  analogReference(INTERNAL); //You must use the internal 1.1v bandgap reference when measuring temperature
  Serial.begin(9600);
}

void loop() {
  int temperature=analogRead(ADC_TEMPERATURE); //ADC_TEMPERATURE is #defined to be the channel for reading the temperature; this varies between parts.
  Serial.print("ADC: ");
  Serial.println(temperature);
  delay(500);
}
