//#include <HardwareSerial.h>

#define RXD2 16
#define TXD2 17

//HardwareSerial TxSerial(1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(1200,SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial2.flush();
  String msg = "ABC-R1B3";
  
  Serial2.println(msg);
  delay(500);
}
