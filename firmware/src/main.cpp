#include "Arduino.h"

#include "provisioning/wifi.h"
#include "cloud_comm/firebase_crud_inti.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("----------------------------");
  Serial.println("Setup START...");

  pinModeSetterForDevice();
  initPreferences();

  String ssid = getSSID();
  String password = getPassword();
  
  if (ssid != "" && password != "")
  {
    connectToWiFi(ssid.c_str(), password.c_str());
  }
  else
  {
    configMode();
  }

  firebaseSetup();

  Serial.println("Setup END...");
  Serial.println("----------------------------");
}

void loop() { 
  delay(100); 
  app.loop();
  Database.loop();
  bootButtonListener();
}
