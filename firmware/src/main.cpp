
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <FirebaseClient.h>

#define DEVICE_ID "IET-a-7021"
#define WIFI_SSID "loop"
#define WIFI_PASSWORD "cyber@123"

#include <configs/FirebaseConfig.h>

void asyncCB(AsyncResult &aResult);
void asyncCBJust(AsyncResult &aResult);

// Firebase Auth
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);

//
DefaultNetwork network;
WiFiClientSecure ssl_client1, ssl_client2;
AsyncClientClass aClient1(ssl_client1, getNetwork(network)), aClient2(ssl_client2, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;

unsigned long tmo = 0;

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    Serial.println("Initializing app...");
    ssl_client1.setInsecure();
    ssl_client2.setInsecure();

    // Initialize the FirebaseApp or auth task handler.
    // To deinitialize, use deinitializeApp(app).
    initializeApp(aClient2, app, getAuth(user_auth), asyncCB, "authTask");
    initializeApp(aClient1, app, getAuth(user_auth), asyncCB, "authTask");

    // Binding the FirebaseApp for authentication handler.
    // To unbind, use Database.resetApp();
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");
    Database.get(aClient1, "/devices/IET-a-7021", asyncCB, true, "ListenerTask");
}

void loop()
{
    app.loop();
    Database.loop();

    if (millis() - tmo > 20000 && app.ready())
    {
        tmo = millis();

        JsonWriter writer;

        object_t json, obj1, obj2;

        writer.create(obj1, "ms", tmo);
        writer.create(obj2, "rand", random(10000, 30000));
        writer.join(json, 2, obj1, obj2);

        Database.set<object_t>(aClient2, "/test/stream/number", json, asyncCBJust);
    }
}

void asyncCB(AsyncResult &aResult)
{
    // WARNING!
    // Do not put your codes inside the callback.

    if (aResult.isEvent())
        Firebase.printf("-> Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());

    if (aResult.isDebug())
        Firebase.printf("-> Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

    if (aResult.isError())
        Firebase.printf("-> Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

    if (aResult.available())
        Firebase.printf("-> task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}

void asyncCBJust(AsyncResult &aResult)
{
    // WARNING!
    // Do not put your codes inside the callback.

    if (aResult.isEvent())
        Firebase.printf("--> Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    if (aResult.available())
        Firebase.printf("--> task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}
