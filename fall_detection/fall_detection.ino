/**************************************************************************************************************/
/************************************************* LIBRARIES  *************************************************/
/**************************************************************************************************************/


/************ WEIGHT SENSORS & HX711 MODULE ************/
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

/****************** WIFI AND FIREBASE ******************/
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

/**************************************************************************************************************/
/******************************************** VARIABLES & DEFINES  ********************************************/
/**************************************************************************************************************/


/****************** WIFI AND FIREBASE ******************/

/* 1. Define the WiFi credentials */
//#define WIFI_SSID "Sea_Cucumber_2.4"
//#define WIFI_PASSWORD "sagi3141"
#define WIFI_SSID "TechPublic"
#define WIFI_PASSWORD "NULL"

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

// auxiliary variables
unsigned long dataMillis = 0;
int count = 0;

/****************** WEIGHT SENSORS & HX711 MODULE ******************/
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;
float maxv = 0;
float weight_sensor_normalized = 0;

/****************** PRESSURE SENSORS ******************/
float cf = 19.5; // caliberation factor

int ffs1 = 33; // FlexiForce sensor #1 is connected to analog pin GPIO of esp32.
int ffs2 = 32; // FlexiForce sensor #2 is connected to analog pin GPIO of esp32.
float vout1, vout2;
int ffsdata11 = 0; 
int ffsdata12 = 0; 
int ffsdata13 = 0; 
int ffsdata1 = 0; 
int ffsdata21 = 0; 
int ffsdata22 = 0; 
int ffsdata23 = 0; 
int ffsdata2 = 0; 

/****************** LED ******************/
const byte led_gpio = 2;

/****************** DEBUG ******************/
// write to serial monitor when debugging
bool DEBUG = false;


/****************************************************************************************************/
/******************************************** FUNCTIONS  ********************************************/
/****************************************************************************************************/

void setup() {
  Serial.begin(57600); delay(10);

  // initialize analog pins for flexible force sensors
  pinMode(ffs1, INPUT); 
  pinMode(ffs2, INPUT); 

  // initialize digital pin led_gpio as an output.
  pinMode(led_gpio, OUTPUT);

  // initialize load cell & HX711 module
  LoadCell.begin();
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = false; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(1.0); // user set calibration value (float), initial value 1.0 may be used for this sketch
    if(DEBUG) Serial.println("Startup is complete");
  }

  // start wifi communication & connect to Firebase
  //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.begin(WIFI_SSID);
  if(DEBUG) Serial.print("Connecting to Wi-Fi");

  if(DEBUG) {
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
  }
  
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  /* In case Wifi connection is lose, try to reconnect automatically */
  Firebase.reconnectWiFi(true);
}

void loop() {

  /*      GET DATA FROM WEIGHT SENSOR & HX711 MODULE      */
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; // increase value to slow down serial print activity
   
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {

      // read data from sensor
      float weight_sensor1 = LoadCell.getData();

      // to avoid switch polarity problem, just make sure the values are always positive
      if(weight_sensor1<0) weight_sensor1 = weight_sensor1*(-1); 

      /* Since we care only about the pressure delta to detect climbing
       * We want to detect when a the maximal value (when the person is on the bed)
       * drops to ~60% of it's value. For this pupose, we want to avoid scaling issues
       * due to calibration factor. To achieve this, we want to normalize the values
       * into a smooth range between [0,1] & to detect when the reading is below 0.650 
       * (for safety we chose a bit higher value). Note that the common value, even when
       * patient is rolling in the bed, are between 0.8-1 so we have some error margin. */
      if(weight_sensor1 > maxv) maxv = weight_sensor1;
	  
	  // next sample
	  float weight_sensor2 = LoadCell.getData();
	  if(weight_sensor2<0) weight_sensor2 = weight_sensor2*(-1);
	  if(weight_sensor2 > maxv) maxv = weight_sensor2;
	  
	  // next sample
	  float weight_sensor3 = LoadCell.getData();
	  if(weight_sensor3<0) weight_sensor3 = weight_sensor3*(-1);
	  if(weight_sensor3 > maxv) maxv = weight_sensor3;
	  
	  float weight_sensor_normalized, weight_sensor_normalized1, weight_sensor_normalized2, weight_sensor_normalized3;
	  
	  // make sure to avoid kernel panic - sometimes may reach 0 due to percision
	  if(maxv != 0) weight_sensor_normalized1 = weight_sensor1/maxv; 
	  if(maxv != 0) weight_sensor_normalized2 = weight_sensor2/maxv;
	  if(maxv != 0) weight_sensor_normalized3 = weight_sensor3/maxv;
	  
	  
	  // moving average method: average other 3 last samples
	  weight_sensor_normalized = weight_sensor_normalized1 + weight_sensor_normalized2 + weight_sensor_normalized3;
	  weight_sensor_normalized = weight_sensor_normalized / 3;
	  
      if(DEBUG) {
        Serial.print("Load_cell output val: ");
        Serial.println(weight_sensor_normalized,3);
      }
      
      newDataReady = 0;
      t = millis();
    }
   }


  /*      GET DATA FROM FLEXIBLE FORCE SENSORS      */

  /* Note: The reason we use this funny conversion is that at first,
   * the conversion from the original guide produced more uniformly spread
   * values which was easier to read. After we found the adequate threshold
   * value, we just left the conversion so it would be easier to debug in
   * the future if neccessary. */
  ffsdata11 = analogRead(ffs1);
  ffsdata12 = analogRead(ffs1);
  ffsdata13 = analogRead(ffs1);
  
  // moving average method: average other 3 last samples
  ffsdata1 = ffsdata11 + ffsdata12 + ffsdata13;
  ffsdata1 = ffsdata1 / 3;
  vout1 = (ffsdata1 * 5.0) / 1023.0; 
  vout1 = vout1 * cf ;

  if(DEBUG) {
    Serial.print("Flexi Force sensor #1: "); 
    Serial.print(vout1,3); 
    Serial.println(""); 
  }
  
  ffsdata21 = analogRead(ffs2);
  ffsdata22 = analogRead(ffs2);
  ffsdata23 = analogRead(ffs2);
  
  // moving average method: average other 3 last samples
  ffsdata2 = ffsdata21 + ffsdata22 + ffsdata23;
  ffsdata2 = ffsdata2 / 3;
  vout2 = (ffsdata2 * 5.0) / 1023.0; 
  vout2 = vout2 * cf ;

  if(DEBUG) {
    Serial.print("Flexi Force sensor #2: "); 
    Serial.print(vout2,3); 
    Serial.println(""); 
  }
  
  /* This is where the magic happends. We compare the values we got from the sensors
   * and compare them to our threshold values. If we detect climbing attempt, i.e.,
   * if the newely arrived data crosses the thresholds, we will write to firestore database
   * which in turn will notify the medical stuff about the room & bed where the climbing
   * attempt was detected. */
  if(weight_sensor_normalized < 0.650) {
    if(vout1 >= 70 || vout2 >= 70){
      
        digitalWrite(led_gpio, HIGH);   // turn the LED on (HIGH is the voltage level)
        
        if(DEBUG) Serial.println("Led is on"); 

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
            content.set("fields/fallDetection/booleanValue", true);
    
            String doc_path = "projects/";
            doc_path += FIREBASE_PROJECT_ID;
            doc_path += "/databases/(default)/documents/coll_id/doc_id"; //coll_id and doc_id are your collection id and document id
    
            count++;
        
            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
        }  
      } else {
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
          if(DEBUG) Serial.println("Led is off"); 
        }
    } else {
          digitalWrite(led_gpio, LOW);   // turn the LED off (LOW is the voltage level)
          if(DEBUG) Serial.println("Led is off"); 
        }

  
}
