#include "header.h"
#include "config/device_config.h"
#include "storage/flash_config.h"
#include "esp_bt.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` and enable it
#endif

BluetoothSerial SerialBT;
String receivedData = "";
bool credentialsReceived = false;
bool configModeActive = false;


bool checkBluetoothStatus()
{
  if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
  {
    return false; // Bluetooth is disabled
  }
  else
  {
    return true; // Bluetooth is enabled
  }
}

void configMode()
{
  Serial.println("Entering Config Mode...");
  SerialBT.begin("IET-a223"); // Enable Bluetooth only in config mode
  configModeActive = true;
  receivedData = "";

  while (configModeActive)
  {
    if (SerialBT.available())
    {
      char c = SerialBT.read();
      if (c == '\n')
      {
        int separatorIndex = receivedData.indexOf(',');
        if (separatorIndex != -1)
        {
          String ssid = receivedData.substring(0, separatorIndex);
          String password = receivedData.substring(separatorIndex + 1);

          Serial.printf("✔ Received SSID: %s Password: %s\n", ssid, password);

          setSSID(ssid);
          setPassword(password);
          Serial.println("✔ SSID & Password Received & Set Successfully...");

          // Serial.print("↺ Connecting to WiFi.");
          WiFi.begin(ssid.c_str(), password.c_str());

          SerialBT.print("DID:");
          SerialBT.println(DID);
          Serial.println("✔ Device ID Send Successfully...");

          delay(500);

          SerialBT.end();
          configModeActive = false;
          if(!checkBluetoothStatus()){
            Serial.println("✔ Bluetooth END Successfully...");
          }
        }
      }
      else
      {
        receivedData += c;
      }
    }
  }
  Serial.println("Exiting Config Mode...");
  ESP.restart();
}

void resetDevice()
{
  Serial.println("Resetting device credentials...");
  preferences.clear();
  preferences.end();
  delay(500);
  ESP.restart();
}

void bootButtonListener()
{
  if (digitalRead(BOOT_BUTTON) == LOW)
  {
    delay(50); // Debounce
    unsigned long startPress = millis();
    while (digitalRead(BOOT_BUTTON) == LOW)
    {
      if (millis() - startPress >= 2000)
      {
        resetDevice();
      }
    }
  }
}
