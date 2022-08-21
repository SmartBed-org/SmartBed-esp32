
/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 * 
 * Copyright (c) 2022 mobizt
 *
*/

//This example shows how to create a document in a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

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

unsigned long dataMillis = 0;
int count = 0;

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

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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

void loop()
{

    if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();

        //For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson content;

        //We will create the nested document in the parent path "a0/b0/c0
        //a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
        //and d? is the document id in the document collection id c0 which we will create.
        String documentPath = "a0/b0/c0/d" + String(count);

        //If the document path contains space e.g. "a b c/d e f"
        //It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

        //double
        content.set("fields/myDouble/doubleValue", 123.45678);

        count++;

        Serial.print("Create a document... ");

        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());

        Serial.print("Query a Firestore database... ");

        // If you have run the Create_Documents example, the document b0 (in collection a0) contains the document collection c0, and
        // c0 contains the collections d?.

        // The following query will query at collection c0 to get the 3 documents in the payload result with descending order.

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
        FirebaseJson query;

        query.set("select/fields/[0]/fieldPath", "myDouble");
        // query.set("d0/myLatLng/geoPointValue/latitude", "latitude");
        // query.set("select/fields/[2]/fieldPath", "status");

        query.set("from/collectionId", "c0");
        query.set("from/allDescendants", false);
        query.set("orderBy/field/fieldPath", "count");
        query.set("orderBy/direction", "DESCENDING");
        query.set("limit", 1);

        // The consistencyMode and consistency arguments are not assigned
        // The consistencyMode is set to fb_esp_firestore_consistency_mode_undefined by default.
        // The arguments is the consistencyMode value, see the function description at
        // https://github.com/mobizt/Firebase-ESP-Client/tree/main/src#runs-a-query

        if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, "a0/b0" /* The document path */, &query /* The FirebaseJson object holds the StructuredQuery data */))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());
    }
}
