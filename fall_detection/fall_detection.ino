#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Sea_Cucumber_2.4"
#define WIFI_PASSWORD "sagi3141"
//#define WIFI_SSID "TechPublic"
//#define WIFI_PASSWORD "NULL"

/* 2. Define the API Key */
#define API_KEY "AIzaSyA3dnNeMSdqlhAeAWLUKSnXxcBonbWq-ck"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smartbed-c3b9e"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "elinor.g12@gmail.com"
#define USER_PASSWORD "@Tkhbur#96"

#define BUTTON_PIN 33 // GIOP33 pin connected to button


//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

float cf = 19.5; // caliberation factor

int ffs1 =33; // FlexiForce sensor #1 is connected to analog pin GPIO 2 of esp32.
int ffs2 = 15; // FlexiForce sensor #2 is connected to analog pin GPIO 15 of esp32.
float vout1, vout2;

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

int ffsdata1 = 0; 
int ffsdata2 = 0; 

const byte led_gpio = 32;

// button variables
int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin

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

void setup() {
  Serial.begin(57600); delay(10);
  
  pinMode(ffs1, INPUT); 
  pinMode(ffs2, INPUT); 

  // initialize digital pin led_gpio as an output.
  pinMode(led_gpio, OUTPUT);

  // initialize the pushbutton pin as an pull-up input
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(-163000); // user set calibration value (float), initial value 1.0 may be used for this sketch
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update()){
    if(LoadCell.getCalFactor() == 1.0) {
        calibrate(); //start calibration procedure
      }
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //WiFi.begin(WIFI_SSID);
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

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  //For sending payload callback
  //config.cfs.upload_callback = fcsUploadCallback;
}

void loop() {

  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH) {
    Serial.println("The state changed from LOW to HIGH");
    calibrate(); //start calibration procedure
    LoadCell.setCalFactor(-163000); // TODO: this should activate calibration. remove this line & the delay
  }
  
  // save the last state
  lastState = currentState;
  
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal
  /*
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay(); //tare
    else if (inByte == 'r') calibrate(); //calibrate
    else if (inByte == 'c') changeSavedCalFactor(); //edit calibration value manually
  }
*/
  // check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  ffsdata1 = analogRead(ffs1);
  vout1 = (ffsdata1 * 5.0) / 1023.0; 
  vout1 = vout1 * cf ;
  Serial.print("Flexi Force sensor #1: "); 
  Serial.print(vout1,3); 
  Serial.println(""); 
  
  ffsdata2 = analogRead(ffs2);
  vout2 = (ffsdata2 * 5.0) / 1023.0; 
  vout2 = vout2 * cf ;
  Serial.print("Flexi Force sensor #2: "); 
  Serial.print(vout2,3); 
  Serial.println(""); 

  float i = LoadCell.getData();
  if(i<=2.7) {
    if(vout1 >= 75 && vout2 >= 75){
        digitalWrite(led_gpio, HIGH);   // turn the LED on (HIGH is the voltage level)
        Serial.println("Led is on"); 

          if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
          {
            dataMillis = millis();
    
            //For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
            FirebaseJson content;
    
            //We will create the nested document in the parent path "a0/b0/c0
            //a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
            //and d? is the document id in the document collection id c0 which we will create.
            String documentPath = "FallDetection/FallDetection/FallDetection/doc" + String(count);
    
            //If the document path contains space e.g. "a b c/d e f"
            //It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"
    
            //boolean
            content.set("fields/FallDetected/booleanValue", true);
    
            String doc_path = "projects/";
            doc_path += FIREBASE_PROJECT_ID;
            doc_path += "/databases/(default)/documents/coll_id/doc_id"; //coll_id and doc_id are your collection id and document id
          
            //timestamp
            content.set("fields/Timestamp/timestampValue", "2014-10-02T15:01:23Z"); //RFC3339 UTC "Zulu" format     
    
            count++;
    
            Serial.print("Create a document... ");
    
            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
        }  
      } else {
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
          Serial.println("Led is off"); 
        }
    } else {
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
          Serial.println("Led is off"); 
        }

  
}

void calibrate() {
  /*
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");
*/
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
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      //char inByte = 'y'; TODO: remove line above & uncomment this one
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
