#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#if defined(ESP32)
#include <WiFi.h>
//#include <NTPClient.h>
#include <WiFiUdp.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <ArduinoJson.h>

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Libraries to get time from NTP Server
#include <WiFi.h>
#include "time.h"


/* 1. Define the WiFi credentials */
//#define WIFI_SSID "Sea_Cucumber_2.4"
//#define WIFI_PASSWORD "sagi3141"
#define WIFI_SSID "Rambam-Free"
#define WIFI_PASSWORD "1234578"

/* 2. Define the API Key */
#define API_KEY "AIzaSyA3dnNeMSdqlhAeAWLUKSnXxcBonbWq-ck"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smartbed-c3b9e"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "elinor.g12@gmail.com"
#define USER_PASSWORD "@Tkhbur#96"


// transmitter & reciever
#define RXD2 16
#define TXD2 17

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

unsigned long dataMillis = 0;
int count = 0;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;


// Variables to hold sensor readings
String dataMessage;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
char* epochTime; 

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 22; //mcu > HX711 sck pin

float cf = 19.5; // caliberation factor

int ffs1 = 33; // FlexiForce sensor #1 is connected to analog pin of esp32.
int ffs2 = 32; // FlexiForce sensor #2 is connected to analog pin of esp32.
int ffs3 = 34;
int ffs4 = 35;
float vout1, vout11, vout12, vout13, vout2, vout21, vout22, vout23, vout3, vout31, vout32, vout33, vout4, vout41, vout42, vout43;

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;
float maxv = 0;
float value = 0;

int ffsdata11 = 0; 
int ffsdata12 = 0; 
int ffsdata13 = 0; 
int ffsdata1 = 0; 
int ffsdata21 = 0; 
int ffsdata22 = 0; 
int ffsdata23 = 0; 
int ffsdata2 = 0; 
int ffsdata31 = 0; 
int ffsdata32 = 0; 
int ffsdata33 = 0; 
int ffsdata3 = 0;
int ffsdata41 = 0; 
int ffsdata42 = 0; 
int ffsdata43 = 0; 
int ffsdata4 = 0;  
int max_data = 0;

const byte led_gpio = 2;

//The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

// Function that gets current epoch time
// Function that gets current epoch time
char* getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  Serial.println("seconds: ");
  time(&now);
  Serial.println(now);
  struct tm* current_time = gmtime(&now);
  current_time->tm_hour = (current_time->tm_hour+3)%24;
  return asctime(current_time);
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
  Serial.begin(57600); delay(10);
  
  pinMode(ffs1, INPUT); 
  pinMode(ffs2, INPUT); 
  pinMode(ffs3, INPUT); 
  pinMode(ffs4, INPUT); 

  // initialize digital pin led_gpio as an output.
  pinMode(led_gpio, OUTPUT);

  Serial2.begin(1200,SERIAL_8N1, RXD2, TXD2);

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
  
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update()){
    if(LoadCell.getCalFactor() == 1.0) {
        calibrate(); //start calibration procedure
      }
  }

  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  #if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
  #endif

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  //For sending payload callback
  //config.cfs.upload_callback = fcsUploadCallback;
}

void loop() {
  
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
  float i; 
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i1 = LoadCell.getData();
      if(i1<0) i1 = i1*(-1);
      if(i1>maxv) maxv = i1;

      float i2 = LoadCell.getData();
      if(i2<0) i2 = i2*(-1);
      if(i2>maxv) maxv = i2;

      float i3 = LoadCell.getData();
      if(i3<0) i3 = i3*(-1);
      if(i3>maxv) maxv = i3;

      i = i1 + i2 + i3;

      i = i / 3;
      if(maxv !=0) value = i/maxv;

//      Serial.print(0); // To freeze the lower limit
//      Serial.print(" ");
//      Serial.print(1); // To freeze the upper limit
//      Serial.print(" ");
      //Serial.println(sensorValue); // To send all three 'data' points to the plotter
      Serial.print("Load_cell output val: ");
      Serial.println(value,3);
      
      newDataReady = 0;
      t = millis();
    }
   }

  ffsdata11 = analogRead(ffs1);
  ffsdata12 = analogRead(ffs1);
  ffsdata13 = analogRead(ffs1);
  //if(ffsdata11 > max_data) max_data = ffsdata11;
  //if(ffsdata12 > max_data) max_data = ffsdata12;
  //if(ffsdata13 > max_data) max_data = ffsdata13;
  //if(max_data > 0) ffsdata11 = ffsdata11 / max_data;
  //if(max_data > 0) ffsdata12 = ffsdata12 / max_data;
  //if(max_data > 0) ffsdata13 = ffsdata13 / max_data;


  ffsdata1 = ffsdata11 + ffsdata12 + ffsdata13;
  ffsdata1 = ffsdata1 / 3;
  vout1 = (ffsdata1 * 5.0) / 1023.0; 
  vout1 = vout1 * cf ;
  Serial.print("Flexi Force sensor1: "); 
  Serial.println(vout1,3); 
  Serial.println(""); 


  ffsdata21 = analogRead(ffs2);
//  vout21 = (ffsdata21 * 5.0) / 1023.0; 
//  vout21 = vout21 * cf ;

  ffsdata22 = analogRead(ffs2);
//  vout22 = (ffsdata22 * 5.0) / 1023.0; 
//  vout22 = vout22 * cf ;

  ffsdata23 = analogRead(ffs2);
//  vout23 = (ffsdata23 * 5.0) / 1023.0; 
//  vout23 = vout23 * cf ;

//  vout2 = vout21 + vout22 + vout23;
//  vout2 = vout2 / 3;

  ffsdata2 = ffsdata21 + ffsdata22 + ffsdata23;
  ffsdata2 = ffsdata2 / 3;

  vout2 = (ffsdata2 * 5.0) / 1023.0; 
  vout2 = vout2 * cf ;
  
  Serial.print("Flexi Force sensor2: "); 
  Serial.print(vout2,3); 
  Serial.println(""); 

  //int start_timer = millis();
  //int end_timer = 0;

  ffsdata31 = analogRead(ffs3);
  ffsdata32 = analogRead(ffs3);
  ffsdata33 = analogRead(ffs3);

  ffsdata3 = ffsdata31 + ffsdata32 + ffsdata33;
  ffsdata3 = ffsdata3 / 3;
  vout3 = (ffsdata3 * 5.0) / 1023.0; 
  vout3 = vout3 * cf ;
  Serial.print("Flexi Force sensor3: "); 
  Serial.println(vout3,3); 
  Serial.println(""); 

  ffsdata41 = analogRead(ffs4);
  ffsdata42 = analogRead(ffs4);
  ffsdata43 = analogRead(ffs4);

  ffsdata4 = ffsdata41 + ffsdata42 + ffsdata4;
  ffsdata4 = ffsdata4 / 3;
  vout4 = (ffsdata4 * 5.0) / 1023.0; 
  vout4 = vout4 * cf ;
  Serial.print("Flexi Force sensor4: "); 
  Serial.println(vout4,3); 
  Serial.println(""); 

  
  
  if(value<0.600)
  {
    if(vout1 >= 80 || vout2 >= 80 || vout3 >= 80 || vout4 >= 80)
	{
        
        //end_timer = millis();
        
        digitalWrite(led_gpio, HIGH);   // turn the LED on (HIGH is the voltage level)
        //delay(500);
        //digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
       
        Serial.println("Led is on"); 

          if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
          {
            dataMillis = millis();
    
            //For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
            FirebaseJson content;
    
            //We will create the nested document in the parent path "a0/b0/c0
            //a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
            //and d? is the document id in the document collection id c0 which we will create.
            //String documentPath = "FallDetection/FallDetection/FallDetection/doc" + String(count);
            String documentPath = "Fall-Detections/21";
    
            //If the document path contains space e.g. "a b c/d e f"
            //It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"
    
            // string
            //content.set("fields/bed number/stringValue", "2");

            // string
            content.set("fields/Fall-Detected/booleanValue", true);
    
            String doc_path = "projects/";
            doc_path += FIREBASE_PROJECT_ID;
            doc_path += "/databases/(default)/documents/coll_id/doc_id"; //coll_id and doc_id are your collection id and document id

            //String time_stamp = getLocalTime();
            
            //timestamp
            //content.set("fields/timeStamp/stringValue", time_stamp);     
    
            count++;
    
            Serial.print("Create a document... ");
    
            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
        } 

        // read room & bed number from fb
        String documentPath = "Devices/21";
        String mask = "Device";
        String room = "";
        String bed = "";

        // If the document path contains space e.g. "a b c/d e f"
        // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

        Serial.print("Get a document... ");

        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str()))
        {
            StaticJsonDocument<512> doc;

            // Deserialize the JSON document
            DeserializationError error = deserializeJson(doc, fbdo.payload().c_str());

            // Test if parsing succeeds.
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

            // serialize Json so it'll be easier to get room & bed number
            String output;
            serializeJson(doc,output);
            Serial.printf("ok\n%s\n\n", output.c_str());

            // get room number
            int room_index = output.indexOf("room number");
            room = output.substring(room_index+29,room_index+30);
            Serial.printf("ok\n%s\n\n", room);

            // get bed number
            int bed_index = output.indexOf("bed number");
            bed = output.substring(bed_index+28,bed_index+29);
            Serial.printf("ok\n%s\n\n", bed);

        }
        else
            Serial.println(fbdo.errorReason());
            
         // transmit
         for(int i = 0; i < 50; i++) {
          Serial2.flush();
          String msg = String("ABCD-R") + room + String("B") + bed;
          Serial2.println(msg);
          Serial.println("Sending msg...");
          delay(500);
         }    
    }
	else
	{
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
          Serial.println("Led is off"); 
    }
  } 
  else
  {
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
          Serial.println("Led is off"); 
  }
  
  if ((millis() - lastTime) > timerDelay)
  {
    //Get epoch time
    epochTime = getTime();
    Serial.print("epoch time: ");
    Serial.print(epochTime);
    //Concatenate all info separated by commas
    dataMessage = String(epochTime) + String(vout1) + "," + String(vout2) + "," + String(vout3) + "," + String(vout4) + "," + String(i) + "\r\n";
    Serial.print("Saving data: ");
    Serial.println(dataMessage);

    //Append the data to file
    appendFile(SD, "/data.txt", dataMessage.c_str());

    lastTime = millis();
  }

  Serial.print("/*");        // Frame start sequence  [/*]
  Serial.print(vout1);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(vout2);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(vout3);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(vout4);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(i);           // Add pressure          [bar]
  Serial.print("*/");        // Frame finish sequence [*/]
		
}

void calibrate() {
 
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    LoadCell.tareNoDelay();
    /*
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        //char inByte = Serial.read();
        //char inByte = 't';
        //if (inByte == 't') LoadCell.tareNoDelay();
        
      }
    }
    */
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
    _resume = true;
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0.01; // TODO: insert known mass of mounting board
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    _resume = true;
    /*
    if (Serial.available() > 0) { TODO: clean this section
      //known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }*/
  }

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() == 0) {
      //char inByte = Serial.read();
      char inByte = 'n'; //TODO: remove line above & uncomment this one
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
}
