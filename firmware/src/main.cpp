#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// Unique Device ID
#define DEVICE_ID "IET-a-7021"

/* WiFi credentials */
#define WIFI_SSID "loop"
#define WIFI_PASSWORD "cyber@123"

/* Firebase credentials */
#define API_KEY "AIzaSyDgwZKG93XVmjSwBvUfbIB96FE5gtbO_V8"
#define DATABASE_URL "https://iet-office-tsy-e1e18-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* Firebase credentials project-1*/
// #define API_KEY "AIzaSyBm_I3tnxfiSi8sd-jhiFNZpO5XgEj8NJ4"
// #define DATABASE_URL "https://project1-ac5cd-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define USER_EMAIL "device@gmail.com"
#define USER_PASSWORD "device"

void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);

DefaultNetwork network; // Network handler
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
WiFiClientSecure ssl_client1, ssl_client2;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client1, getNetwork(network)), aClient2(ssl_client2, getNetwork(network));
RealtimeDatabase Database;

unsigned long ms = 0;

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

  Serial.println("✔ Device Connected to Internet Successfully...");
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  Serial.println("Initializing app...");

  ssl_client1.setInsecure();
  ssl_client2.setInsecure();

  initializeApp(aClient2, app, getAuth(user_auth), asyncCB, "authTask");

  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

  Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");
  Database.get(aClient, "/devices/IET-a-7021", asyncCB, true /* SSE mode */, "streamTask");
  Serial.println("✔ Device Connected to Firebase Successfully...");
}

void loop()
{
  app.loop();
  Database.loop();
}

void asyncCB(AsyncResult &aResult)
{
  if (aResult.available())
  {
    printResult(aResult);
  }
  else
  {
    Serial.println("No data found");
  }
}

void printResult(AsyncResult &aResult)
{
  if (aResult.available())
  {
    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
    if (RTDB.isStream())
    {
      Firebase.printf("task: %s - ", aResult.uid().c_str());
      Firebase.printf("event: %s - ", RTDB.event().c_str());
      Firebase.printf("path: %s - ", RTDB.dataPath().c_str());
      Firebase.printf("data: %s\n", RTDB.to<const char *>());
      Firebase.printf("type: %d\n", RTDB.type());
      Serial.println("----------------------------");
    }
    else
    {
      Serial.println("----------------------------");
      Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
  }
}
