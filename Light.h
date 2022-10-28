
int sensorPin = 12; // Declaration of the input pin
float read_light_data()
{    
  // Current voltage will be measured...
  int rawValue = analogRead(sensorPin);
  float voltage = rawValue * (5.0 / 1023) * 1000;
  Serial.print("Voltage value:"); Serial.print(voltage); Serial.println("mV");
  return voltage;
}
