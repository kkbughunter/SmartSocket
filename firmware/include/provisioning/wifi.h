#include "header.h"
#include "provisioning/bluetooth.h"

void connectToWiFi(const String &ssid, const String &password)
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        bootButtonListener();
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}
