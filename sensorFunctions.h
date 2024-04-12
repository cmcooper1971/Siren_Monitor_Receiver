#pragma once

#ifndef SENSOR_FUNCTIONS_H
#define SENSOR_FUNCTIONS_H

// Main libraries

#include <Arduino.h>
#include <TFT_eSPI.h>

// Local declarations

#include "global.h"
#include "icons.h"
#include "screenLayout.h"

void sensorCheckCall(boolean interruptDetected,const unsigned long interruptCheckPeriod, const byte interruptCount,byte nanoResetPin);

#endif
