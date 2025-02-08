#include <FirebaseClient.h>

#include "config/firebase_config.h"

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;
RealtimeDatabase Database;

WiFiClient basic_client1;
WiFiClient basic_client2;
WiFiClient basic_client3;

// The ESP_SSLClient uses PSRAM by default (if it is available), for PSRAM usage, see https://github.com/mobizt/FirebaseClient#memory-options
// For ESP_SSLClient documentation, see https://github.com/mobizt/ESP_SSLClient
ESP_SSLClient ssl_client1;
ESP_SSLClient ssl_client2;
ESP_SSLClient ssl_client3;

using AsyncClient = AsyncClientClass;

AsyncClient aClient1(ssl_client1, getNetwork(network));
AsyncClient aClient2(ssl_client2, getNetwork(network));
AsyncClient aClient3(ssl_client3, getNetwork(network));

// AsyncResult aResult_no_callback1;
// AsyncResult aResult_no_callback2;
// AsyncResult aResult_no_callback3;

unsigned long ms = 0;

void asyncCBinit(AsyncResult &aResult);
void asyncCB1(AsyncResult &aResult);
void printResult(AsyncResult &aResult);

void printFirebaseLibVersion()
{
    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
}

void clientSetup()
{
    ssl_client1.setClient(&basic_client1);
    ssl_client2.setClient(&basic_client2);
    ssl_client3.setClient(&basic_client3);

    ssl_client1.setInsecure();
    ssl_client2.setInsecure();
    ssl_client3.setInsecure();

    ssl_client1.setBufferSizes(2048, 1024);
    ssl_client2.setBufferSizes(2048, 1024);
    ssl_client3.setBufferSizes(2048, 1024);

    // In case using ESP8266 without PSRAM and you want to reduce the memory usage,
    // you can use WiFiClientSecure instead of ESP_SSLClient with minimum receive and transmit buffer size setting as following.
    // ssl_client1.setBufferSizes(1024, 512);
    // ssl_client2.setBufferSizes(1024, 512);
    // ssl_client3.setBufferSizes(1024, 512);
    // Note that, because the receive buffer size was set to minimum safe value, 1024, the large server response may not be able to handle.
    // The WiFiClientSecure uses 1k less memory than ESP_SSLClient.

    ssl_client1.setDebugLevel(1);
    ssl_client2.setDebugLevel(1);
    ssl_client3.setDebugLevel(1);

    // In ESP32, when using WiFiClient with ESP_SSLClient, the WiFiClient was unable to detect
    // the server disconnection in case server session timed out and the TCP session was kept alive for reusage.
    // The TCP session timeout in seconds (>= 60 seconds) can be set via `ESP_SSLClient::setSessionTimeout`.
    ssl_client1.setSessionTimeout(150);
    ssl_client1.setSessionTimeout(150);
    ssl_client3.setSessionTimeout(150);
    Serial.println("✔ Client Setup Successfully...");
}

void initFirebaseApp()
{
    Serial.println("Initializing the app...");
    initializeApp(aClient3, app, getAuth(user_auth), asyncCBinit, "authTask");

    // Binding the FirebaseApp for authentication handler.
    // To unbind, use Database.resetApp();
    app.getApp<RealtimeDatabase>(Database);

    // Set your database URL (requires only for Realtime Database)
    Database.url(DATABASE_URL);

    // Since v1.2.1, in SSE mode (HTTP Streaming) task, you can filter the Stream events by using RealtimeDatabase::setSSEFilters(<keywords>),
    // which the <keywords> is the comma separated events.
    // The event keywords supported are:
    // get - To allow the http get response (first put event since stream connected).
    // put - To allow the put event.
    // patch - To allow the patch event.
    // keep-alive - To allow the keep-alive event.
    // cancel - To allow the cancel event.
    // auth_revoked - To allow the auth_revoked event.
    // To clear all prevousely set filter to allow all Stream events, use RealtimeDatabase::setSSEFilters().
    Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");
    Serial.println("✔ Firebase App initialized Successfully...");
}

void startFirebaseStream()
{
    char parentPath[50];
    sprintf(parentPath, "%s%s", "/devices/", DID);

    Database.get(aClient1, "/devices/d1/node", asyncCB1, true, "node");
    Serial.println("✔ Firebase Stream Started Successfully...");
}

void asyncCBinit(AsyncResult &aResult)
{
    printResult(aResult);
}

void asyncCB1(AsyncResult &aResult)
{
    printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
        if (RTDB.type() != 0)
        {
            if (RTDB.isStream())
            {
                // Firebase.printf("task: %s event: %s\n", aResult.uid().c_str(), RTDB.event().c_str());
                // Firebase.printf("type: %d\n", RTDB.type());

                // The stream event from RealtimeDatabaseResult can be converted to the values as following.
                bool v1 = RTDB.to<bool>();
                int v2 = RTDB.to<int>();
                float v3 = RTDB.to<float>();
                double v4 = RTDB.to<double>();
                String v5 = RTDB.to<String>();
                
                // if the path mathcn the case print the path and data 
                String path = RTDB.dataPath().c_str();
                if(path.equals("/node1/status")){
                    Firebase.printf("1-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node2/status")){
                    Firebase.printf("2-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node3/status")){
                    Firebase.printf("3-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node4/status")){
                    Firebase.printf("4-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node5/status")){
                    Firebase.printf("5-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node6/status")){
                    Firebase.printf("6-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node7/status")){
                    Firebase.printf("7-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else if(path.equals("/node8/status")){
                    Firebase.printf("9-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                else {
                    Firebase.printf("0-path: %s data: %s\n", RTDB.dataPath().c_str(), RTDB.to<const char *>());
                }
                
                Serial.println("----------------------------");
                
            }
            else
            {
                Serial.println("----------------------------");
                Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
            }
        }
        // else
        // {
        //     Serial.println("Keep-alive....");
        // }
        // Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
    }
}


// =================================================================================================

void firebaseSetup(){
    printFirebaseLibVersion();
    clientSetup();
    initFirebaseApp();
    startFirebaseStream();
}
