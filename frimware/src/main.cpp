/*
Version 03:
  * USer can controle using the local switch also.
*/
#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LittleFS.h>

#define H_SSID "IET-y223"
#define H_PASSWORD "password"
#define EXCHANGE_PORT 20002
#define BROADCAST_PORT 20001
#define CTRL_PORT 20000
#define BUFF_SIZE 8

WiFiUDP broadcast, control;
WiFiServer tcp(EXCHANGE_PORT);
WiFiClient conn;

bool wifi_setup = false;
bool wifi_conn = false;
String ssid;
String password;

const int buttonPin = 5;     // D1
bool buttonState = HIGH;     // Current state of the button
bool lastButtonState = HIGH; // Previous state of the button
bool hotspotActive = false;  // Flag to track if the hotspot is already on
String s1;                   // Data
unsigned long sendDataPrevMillis = 0;
int count = 0;
volatile bool dataChanged = false;
const int D4_PIN = 2;
const int light_switch = 3;
volatile bool state = false;

#define API_KEY "AIzaSyB3usuxJqpEtKSl6i8eRmYAFip_fR-T_No"
#define DATABASE_URL "https://final-year-project-400d2-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "karthikeyan.aas@gmail.com"
#define USER_PASSWORD "1234567890"

FirebaseData stream;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Config - Function definitions
String read_data(String file_name);
void write_data(String file_name, String data);
void remove_data(String file_name);
void wifi_connect(String ssid, String password);
void wifi_mode();
void parse_data(String data);
void enable_hotspot();
void get_wifi_creds();
void disable_wifi();
void setup_device();
void restart_device();
void ListenD1();
void disable_hotspot();

String read_data(String file_name)
{
  File file = LittleFS.open("/" + file_name, "r");
  if (!file)
  {
    return "";
  }
  while (file.available())
  {
    return file.readString();
  }
  file.close();
  return "";
}

void write_data(String file_name, String data)
{
  File file = LittleFS.open("/" + file_name, "w");
  file.print(data);
  delay(1);
  file.close();
  Serial.println("Data saved");
}

void remove_data(String file_name)
{
  LittleFS.remove("/" + file_name);
  Serial.println("Data removed");
}

void restart_device()
{
  Serial.println("Device is restarting...");
  ESP.restart();
}

void disable_wifi()
{
  WiFi.mode(WIFI_OFF);
  Serial.println("Wi-Fi scanning stopped and Wi-Fi turned off.");
}

void enable_hotspot()
{
  if (!hotspotActive)
  {
    WiFi.softAP(H_SSID, H_PASSWORD);
    Serial.println("---Hotspot Enabled---");
    Serial.print("SoftAP IP Address: ");
    Serial.println(WiFi.softAPIP());
    hotspotActive = true;
  }
  else
  {
    Serial.println("Hotspot is already on.");
  }
}

void disable_hotspot()
{
  if (hotspotActive)
  {
    WiFi.softAPdisconnect(true);
    Serial.println("---Hotspot Disabled---");
    hotspotActive = false;
  }
  else
  {
    Serial.println("Hotspot is already off.");
  }
}

void wifi_connect(String ssid, String password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Wifi Connecting #");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
    // ^ Listen for D1 pin
    ListenD1();
  }
  // ^ if new SSID & password receive
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("\nConnected as ");
    Serial.print(WiFi.localIP());
    Serial.print(" | ");
    Serial.println(WiFi.subnetMask());
    return;
  }
}

void get_wifi_creds()
{
  delay(1000);

  tcp.begin();
  Serial.print("TCP Connection opened: ");

  while (!conn)
  {
    conn = tcp.accept();
    if (conn && conn.connected())
    {
      Serial.print("\nConnection established with ");
      Serial.println(conn.remoteIP());
    }
    Serial.print("~");
    delay(100);
  }

  conn.write("CRED-REQ");
  String msg, data;
  while (conn.connected())
  {
    while (conn.available())
    {
      msg = conn.readString();
      Serial.println(msg);
      conn.write("END");
      if (msg == "END")
      {
        File file = LittleFS.open("/data", "w");
        file.print(data);
        Serial.println("Writing data");
        conn.stop();
        break;
      }
      else
        data = msg;
    }
    if (msg == "END")
      break;
  }
  parse_data(data);

  // write the new SSID and Password.
  write_data("data", ssid + "\n" + password);
  data = read_data("data");
  Serial.println("New credentials saved. - SSID: " + ssid + " Password: " + password + " Data: " + data + " Data len:" + data.length());

  // Attempt to connect to Wi-Fi with the new credentials
  wifi_connect(ssid, password);

  // Set wifi_setup to true if the connection is successful
  if (WiFi.status() == WL_CONNECTED)
  {
    wifi_setup = true;
  }
}

void parse_data(String data)
{
  Serial.println("Data: " + data);
  int splitIndex = data.indexOf('\n');
  if (splitIndex == -1)
  {
    ssid = data;
    password = "";
  }
  else
  {
    ssid = data.substring(0, splitIndex);
    password = data.substring(splitIndex + 1);
  }
  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);
}

void ListenD1()
{
  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == LOW && lastButtonState == HIGH)
  {
    disable_wifi();
    remove_data("data");
    enable_hotspot();
    get_wifi_creds();
    disable_hotspot();
    restart_device();
  }
  lastButtonState = currentButtonState;
}

void setup_device()
{
  Serial.println("\t---Setup Device START---");

  String data = read_data("data");
  if (data.length() > 5)
  {
    Serial.println("Data len: " + (String)data.length() + " so using old ssid & password.");
    parse_data(data);
    wifi_connect(ssid, password);
  }
  else
  {
    Serial.println("Data len: " + (String)data.length() + " so requesting for new SSID and Password.");
    enable_hotspot();
    get_wifi_creds();
    disable_hotspot();
  }

  Serial.println("\t---Setup Device END---");
}

// Firebase - Function definitions
void streamCallback(FirebaseStream data);
void streamTimeoutCallback(bool timeout);

void streamCallback(FirebaseStream data)
{

  // print path and status of the topic
  Serial.print(data.streamPath().c_str());
  Serial.printf(" - %s\n", data.payload().c_str());
  // Serial.println( data.payload().c_str());
  // Serial.println( "Test");
  s1 = data.payload().c_str();

  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
  // --->wifi checks should be here
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n---SETUP START---");

  // Config - V1
  pinMode(buttonPin, INPUT_PULLUP); // for RESET Device.

  if (!LittleFS.begin())
  {
    Serial.println("File System not accessible");
  }
  else
    Serial.println("Accessing File system");

  Serial.println("Broadcast Connection Started...");
  broadcast.begin(BROADCAST_PORT);
  Serial.println("Control Connection started...");
  control.begin(CTRL_PORT);

  setup_device(); // Call the new setup function

  // Cloud - V2
  // 2) PIN Config
  pinMode(D4_PIN, OUTPUT);
  pinMode(light_switch, INPUT);
  state = digitalRead(light_switch);

  // 3) FIREBASE Config
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(2048, 1024);
  stream.setBSSLBufferSize(2048, 1024);
  Firebase.begin(&config, &auth);

  // Listen to the path
  if (!Firebase.RTDB.beginStream(&stream, "/devices/d1/status")) // change the path
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  Serial.println("---SETUP END---");
}

volatile bool localUpdate = false; // Flag to indicate a local update

void loop()
{
  ListenD1();                // Monitor for D1 input
  Firebase.RTDB.runStream(); // Run the Firebase stream

  // Check for Firebase stream changes (Remote updates)
  if (dataChanged && !localUpdate) // Only process remote changes if not a local update
  {
    dataChanged = false;
    if (s1 == "true")
    {
      Serial.println("S1 Switch ON (Remote)");
      digitalWrite(D4_PIN, LOW); // Turn ON locally
    }
    else
    {
      Serial.println("S1 Switch OFF (Remote)");
      digitalWrite(D4_PIN, HIGH); // Turn OFF locally
    }
  }

  // Local Switch Handling
  bool current_state = digitalRead(light_switch); // Read the local switch state
  if (current_state != state)                     // Check if the state has changed
  {
    localUpdate = true; // Set the flag to indicate local update

    Serial.println("Local state: " + (String)current_state);

    if (current_state == true)
    {
      Serial.println("Light old State: " + s1);

      if (s1 == "false")
      {
        digitalWrite(D4_PIN, HIGH);
        if (Firebase.RTDB.setBool(&fbdo, "/devices/d1/status", true))
        {
          Serial.println("Light New State:(" + s1 + ")");
        }
      }
      else
      {
        digitalWrite(D4_PIN, LOW);
        if (Firebase.RTDB.setBool(&fbdo, "/devices/d1/status", false))
        {
          Serial.println("Light New State:(" + s1 + ")");
        }
      }
    }
    delay(50);             // Debounce delay
    state = current_state; // Update the local state
    localUpdate = false;   // Reset the local update flag
  }

  if (!stream.httpConnected())
  {
    Serial.println("Server was disconnected!");
  }
}
