#include "header.h"
Preferences preferences;


void initPreferences()
{
    preferences.begin("wifiCreds", false);
}

String getSSID()
{
    return preferences.getString("ssid", "");
}
String getPassword()
{
    return preferences.getString("password", "");
}
void setSSID(String ssid)
{
    preferences.putString("ssid", ssid);
}

void setPassword(String password)
{
    preferences.putString("password", password);
}