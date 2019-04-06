/**
 * @file debugging_utilities.cpp
 * @brief Serial println abstraction with DEBUG configuration switch.
 * This is so all printlns can be traced.
*/

#include <Arduino.h>
#include "configuration.h"
#ifndef RADIOTYPE1
#include <mosquitto.h>
#endif
#include "debugging_utilities.h"

// branches/functions to be removed.

void Debugging_Utilities_DebugLog(String inLine)
{
	if (DEBUG)
	{
		Serial.println(inLine);
	}
#ifndef RADIOTYPE1
  if (DEBUG_MQTT)
  {
    publishMQTT("/fossasat-1/logging", "gnd:" + inLine);
  }
#endif
}
