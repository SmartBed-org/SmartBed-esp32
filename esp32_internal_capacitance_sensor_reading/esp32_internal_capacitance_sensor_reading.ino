// ESP32 Touch Test
// Just test touch pin - Touch0 is T0 which is on GPIO 4.
// In NodeMCU ESP32-S it is pin # P4 (7th pin from clk)

void setup()
{
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
}

void loop()
{
  Serial.println(touchRead(T0));  // get value using T0
  delay(1000);
}