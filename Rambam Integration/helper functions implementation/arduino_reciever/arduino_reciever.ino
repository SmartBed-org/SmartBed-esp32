#include <SoftwareSerial.h>

SoftwareSerial mySerial(2,3);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect
  }

  // set data rate for the SoftwareSerial
  mySerial.begin(2400);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(mySerial.available()) {
    Serial.write(mySerial.read());
  }
}
