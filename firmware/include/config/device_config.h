#include "header.h"

String DID = "d1";

#define BOOT_BUTTON 0

void pinModeSetterForDevice();

void pinModeSetterForDevice()
{
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
}