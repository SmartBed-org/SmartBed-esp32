#include <SoftwareSerial.h> // for rx

// for oled
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial mySerial(2,3);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect
  }

  // prepare oled
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  
  // set data rate for the SoftwareSerial
  mySerial.begin(1200);
}

void loop() {
  // put your main code here, to run repeatedly:

  String recieved_data = "";
  String output = "";

  Serial.println("waiting");
  
  if(mySerial.available()) {
    recieved_data = mySerial.read();
    Serial.println(recieved_data);

    // clear output
    int index = output.indexOf("R");
    output = recieved_data.substring(index,index+4);

    // display room & bed nubmer on oled screen
    if(output != "") {
      display.setTextSize(4);
      display.setTextColor(WHITE);
      display.setCursor(0, 24);
      
      // Display static text
      display.println(output); //Write text here
      display.display(); 
      delay(2000);
    }
  
    // clean variables & display
    output = "";
    display.clearDisplay();
  }
}
