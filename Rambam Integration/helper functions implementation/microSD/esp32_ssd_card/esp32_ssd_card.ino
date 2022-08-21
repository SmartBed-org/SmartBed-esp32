/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-microsd-card-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Libraries to get time from NTP Server
#include <WiFi.h>
#include "time.h"

// Replace with your network credentials
const char* ssid     = "Sea_Cucumber_2.4";
const char* password = "sagi3141";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;


// Variables to hold sensor readings
float fsr1, fsr2, fsr3, fsr4, load_cell;
String dataMessage;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
char* epochTime; 

// Function that gets current epoch time
char* getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  struct tm* current_time = gmtime(&now);
  current_time->tm_hour = (current_time->tm_hour+3)%24;
  return asctime(current_time);
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}


// Initialize SD card
void initSDCard(){
   if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void setup() {
  Serial.begin(115200);
  
  initWiFi();
  initSDCard();
  configTime(0, 0, ntpServer);
  
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Epoch Time \nFSR1, FSR2, FSR3, FSR4, Load-Cell \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    //Get epoch time
    epochTime = getTime();
    
    //Get sensor readings
    fsr1 = 1;
    fsr2 = 2;
    fsr3 = 3;
    fsr4 = 4;
    load_cell = 5;

    //Concatenate all info separated by commas
    dataMessage = String(epochTime) + String(fsr1) + "," + String(fsr2) + "," + String(fsr3) + "," + String(fsr4) + "," + String(load_cell) + "\r\n";
    Serial.print("Saving data: ");
    Serial.println(dataMessage);

    //Append the data to file
    appendFile(SD, "/data.txt", dataMessage.c_str());

    lastTime = millis();
  }
}
