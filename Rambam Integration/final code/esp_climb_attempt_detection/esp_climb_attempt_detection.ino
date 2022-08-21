//************************************************//
//******************  INCLUDES  ******************//
//************************************************//

// wifi libraries
#if defined(ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

// HX711 load cell module libraries
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// firesbase communication libraries
#include <Firebase_ESP_Client.h>

// provide the token generation process info
#include <addons/TokenHelper.h>

// microSD card libraries 
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// NTP server & time libraries
#include "time.h"

// JSON libraries
#include <ArduinoJson.h>

//*******************************************************//
//******************  PINS DEFINITION  ******************//
//*******************************************************//

// led visual indicator
const byte led_gpio = 2;

// HX711 module
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 22; //mcu > HX711 sck pin

// flexible force sensors
const int ffs1 = 32;
const int ffs2 = 33;
const int ffs3 = 34;
const int ffs4 = 35;

// transmitter & reciever
const int RXD2 = 16;
const int TXD2 = 17;


//************************************************************************//
//******************  VARIABLE AND CONSTANT DEFINITION  ******************//
//************************************************************************//

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

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// ued for delay when writing to fb
unsigned long dataMillis = 0;

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
unsigned long start_time = 0;
float maxv = 0;

// flexible force sensors
int fsrADC1, fsrADC12, fsrADC13, fsrADC2, fsrADC22, fsrADC23, fsrADC3, fsrADC32, fsrADC33, fsrADC4, fsrADC42, fsrADC43;
float fsrV1, fsrV2, fsrV3, fsrV4;
const float VCC = 3.3;

// timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// variable to save current epoch time
char* epochTime; 

// variable to hold sensor readings
String dataMessage;



//*********************************************************//
//******************  HELPER FUNCATIONS  ******************//
//*********************************************************//

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


//***************************************************//
//******************  BEGIN SETUP  ******************//
//***************************************************//

void setup() {
  
  Serial.begin(57600); delay(10);

  //============================  ASSIGN PINS ============================//
  pinMode(ffs1, INPUT); 
  pinMode(ffs2, INPUT); 
  pinMode(ffs3, INPUT); 
  pinMode(ffs4, INPUT); 

  pinMode(led_gpio, OUTPUT);

  //============================  CONNECT TO WIFI AND SET WIFI MODE ============================//
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

  
  //============================  CONNECT TO FIREBASE CLIENT ============================//
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


  //============================  CONNECT LOAD CELL ============================//
  LoadCell.begin();
  LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }

  //============================  INITIALIZE TRANSMITTER ============================//
  Serial2.begin(1200,SERIAL_8N1, RXD2, TXD2);

  //============================  INITIALIZE MICRO-SD ============================//
  initSDCard();
  configTime(0, 0, ntpServer);
  
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt"); // data.txt may be changed to another name/file type
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    // file structure
    writeFile(SD, "/data.txt", "Epoch Time \nFSR1, FSR2, FSR3, FSR4, Load-Cell \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
  
}

void loop() {

  //====================================================================================================//
  //============================             VARIABLE DEFINITIONS           ============================//
  //====================================================================================================//
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
  float lc_data;

  //====================================================================================================//
  //============================  GET, PROCESS AND PRINT LOAD CELL READINGS ============================//
  //====================================================================================================//
   
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
      if (millis() > start_time + serialPrintInterval) {
        float lc_data1 = LoadCell.getData();
        lc_data1 > 0 ? lc_data1*(-1) : lc_data1;
        if(lc_data1>maxv) maxv = lc_data1;
  
        float lc_data2 = LoadCell.getData();
        lc_data2 > 0 ? lc_data2*(-1) : lc_data2;
        if(lc_data2>maxv) maxv = lc_data2;
  
        float lc_data3 = LoadCell.getData();
        lc_data3 > 0 ? lc_data3*(-1) : lc_data3;
        if(lc_data3>maxv) maxv = lc_data3;
  
        lc_data = lc_data1 + lc_data2 + lc_data3;
  
        lc_data = lc_data/3;
        if(maxv !=0) lc_data = lc_data/maxv;
  
        Serial.print("Load_cell output val: ");
        Serial.println(lc_data,3);
        
        newDataReady = 0;
        start_time = millis();
      }
   }

  //====================================================================================================//
  //============================                GET FSR READINGS            ============================//
  //====================================================================================================//
  
  fsrADC1 = analogRead(ffs1);
  fsrADC12 = analogRead(ffs1);
  fsrADC13 = analogRead(ffs1);

  fsrADC2 = analogRead(ffs2);
  fsrADC22 = analogRead(ffs2);
  fsrADC23 = analogRead(ffs2);

  fsrADC3 = analogRead(ffs3);
  fsrADC32 = analogRead(ffs3);
  fsrADC33 = analogRead(ffs3);

  fsrADC4 = analogRead(ffs4);
  fsrADC42 = analogRead(ffs4);
  fsrADC43 = analogRead(ffs4);


  //====================================================================================================//
  //============================            PROCESS FSR READINGS            ============================//
  //====================================================================================================//

  fsrADC1 = fsrADC1 + fsrADC12 + fsrADC13;
  fsrADC1 = fsrADC1 / 3;
  
  fsrADC2 = fsrADC2 + fsrADC22 + fsrADC23;
  fsrADC2 = fsrADC2 / 3;

  fsrADC3 = fsrADC3 + fsrADC32 + fsrADC33;
  fsrADC3 = fsrADC3 / 3;

  fsrADC4 = fsrADC4 + fsrADC42 + fsrADC43;
  fsrADC4 = fsrADC4 / 3;

  //============================  PRINT FSR READINGS ============================//

  fsrV1 = (fsrADC1 * VCC) / 1023.0; 
  Serial.print("FSR sensor 1: "); 
  Serial.println(fsrV1,3); 
  Serial.println(""); 

  fsrV2 = (fsrADC2 * VCC) / 1023.0; 
  Serial.print("Flexi Force sensor2: "); 
  Serial.print(fsrV2,3); 
  Serial.println(""); 

  fsrV3 = (fsrADC3 * VCC) / 1023.0; 
  Serial.print("Flexi Force sensor3: "); 
  Serial.println(fsrV3,3); 
  Serial.println(""); 

  
  fsrV4 = (fsrADC4 * VCC) / 1023.0; 
  Serial.print("Flexi Force sensor4: "); 
  Serial.println(fsrV4,3); 
  Serial.println(""); 


  //====================================================================================================//
  //============================        COMPARE DATA WITH THRESHOLDS        ============================//
  //====================================================================================================//  
  
  if(lc_data<0.600) {
    
    if(fsrV1 >= 80 || fsrV2 >= 80 || fsrV3 >= 80 || fsrV4 >= 80) {
                
        digitalWrite(led_gpio, HIGH);   // turn the LED on (HIGH is the voltage level)
        
        //===============================================================//
        // write to firebsae to notify upon climb attempt identification //
        //===============================================================//
        
        if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0)) {
          dataMillis = millis();
  
          //For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
          FirebaseJson content;
  
          //We will create the nested document in the parent path "a0/b0/c0
          //a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
          //and d? is the document id in the document collection id c0 which we will create.

          String documentPath = "Fall-Detections/21";
  
          //If the document path contains space e.g. "a b c/d e f"
          //It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

          // string
          content.set("fields/Fall-Detected/booleanValue", true);
  
          String doc_path = "projects/";
          doc_path += FIREBASE_PROJECT_ID;
          doc_path += "/databases/(default)/documents/coll_id/doc_id"; //coll_id and doc_id are your collection id and document id
    
          Serial.print("Create a document... ");
  
          if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
              Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
          else
              Serial.println(fbdo.errorReason());
      }

        //===============================================================//
        // device numbers are stored in firebase collection Devices.
        // upon climbing attempt detection, use devices unique id to
        // identify in which room & bed the device is located.
        //===============================================================//       

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
            StaticJsonDocument<512> doc; // allocate enough space for string

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
          delay(500);
         }    
    } // end if(check fsr values)
    
    else
        digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
        
  } // end if(check lc value)
  
  else {
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
  }


  //====================================================================================================//
  //============================   PERIODICALLY UPDATE LOG ON MICROSD CARD  ============================//
  //====================================================================================================// 
  
  if ((millis() - lastTime) > timerDelay) {
    //Get epoch time
    epochTime = getTime();

    //Concatenate all info separated by commas
    dataMessage = String(epochTime) + String(fsrV1) + "," + String(fsrV2) + "," + String(fsrV3) + "," + String(fsrV3) + "," + String(lc_data) + "\r\n";

    //Append the data to file
    appendFile(SD, "/data.txt", dataMessage.c_str());

    lastTime = millis();
  }

  //====================================================================================================//
  //============================      PRINT DATA TO JSON SERIAL-STUDIO      ============================//
  //====================================================================================================// 
  
  Serial.print("/*");        // Frame start sequence  [/*]
  Serial.print(fsrV1);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(fsrV2);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(fsrV3);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(fsrV4);       // Add pressure          [bar]
  Serial.print(",");         // Separator character   [,]
  Serial.print(lc_data);           // Add pressure          [bar]
  Serial.print("*/");        // Frame finish sequence [*/]

}
