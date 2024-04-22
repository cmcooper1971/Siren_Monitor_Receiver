
// Local declations

#include "sensorFunctions.h"    // Sensor functions
#include "drawBitmap.h"         // Draw drawBitmap
#include "global.h"             // Global

// Debug serial prints

#define DEBUG 0

#if DEBUG==1
#define outputDebug(x); Serial.print(x);
#define outputDebugLn(x); Serial.println(x);
#else
#define outputDebug(x); 
#define outputDebugLn(x); 
#endif

/*---------------------------------------------------------------- */

void sensorCheckCall(boolean interruptDetected, const unsigned long interruptCheckPeriod, const byte interruptCount, byte nanoResetPin) {

    static unsigned long lastCheckTime = 0;
    static byte interruptIntervals = 0;

    if (millis() - lastCheckTime >= interruptCheckPeriod) {

        lastCheckTime = millis();

        if (!interruptDetected) {
            outputDebugLn("");
            outputDebug("Arduino BLE Sense not responding #");
            outputDebug(interruptIntervals);
            outputDebugLn("");

            interruptIntervals++;

            drawBitmap(tft, PULSE_ICON_Y, PULSE_ICON_X, pulseAmber, PULSE_ICON_W, PULSE_ICON_H);

            if (interruptIntervals > interruptCount) {

                drawBitmap(tft, PULSE_ICON_Y, PULSE_ICON_X, pulseRed, PULSE_ICON_W, PULSE_ICON_H);

                delay(1500);

                playTone(buzzerP, buzzerF, buzzerD);

                outputDebugLn("");
                outputDebug("Rebooting Arduino BLE Sense...");
                outputDebugLn("");

                digitalWrite(nanoResetPin, LOW);
                delay(10);
                digitalWrite(nanoResetPin, HIGH);

                interruptIntervals = 0;

            }
        }

        else {

            outputDebugLn("");
            outputDebug("Interrupt signal detected...");
            outputDebug(interruptIntervals);
            outputDebugLn("");

            interruptDetected = false;
            interruptIntervals = 0;

            drawBitmap(tft, PULSE_ICON_Y, PULSE_ICON_X, pulseGreen, PULSE_ICON_W, PULSE_ICON_H);

        }
    }

} // Close function

/*---------------------------------------------------------------- */