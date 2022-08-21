#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
   
}

void loop() {

  // reciever code
  uint8_t buf[3] = {0};
  uint8_t buflen = sizeof(buf);
  buf[0] = 0x69;
  buf[1] = 0x70;
  buf[2] = 0x71;
  buf[3] = 0x72;

  // convert uint8_t to string
  String output = (char*)buf;
  output = output.substring(0,3);

  // display room & bed nubmer
  if(output != "") {
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0, 24);
    // Display static text
    display.println(output); //Write custom text here
    //display.update();
    display.display(); 
    delay(2000);
  }

  // clean variables & display
  output = "";
  display.clearDisplay();
}
