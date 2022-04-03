float cf = 19.5; // caliberation factor

int ffs1 = 36; // FlexiForce sensor is connected to analog pin GPIO 36 (SVP) of esp32.

int ffsdata = 0; 
float vout; 
void setup()
{
  Serial.begin(115200); 
  pinMode(ffs1, INPUT); 
  
}

void loop()
{
 

ffsdata = analogRead(ffs1);
vout = (ffsdata * 5.0) / 1023.0; 
vout = vout * cf ; 
Serial.print("Flexi Force sensor: "); 
Serial.print(vout,3); 
Serial.println(""); 
delay(100); 
  
}
