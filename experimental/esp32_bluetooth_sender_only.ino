// =====================================================
// ESP32 - Bluetooth Sender Only (No Blynk)
// Experimental version - testing Bluetooth sending
// =====================================================

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
const char *VEHICLE_NAME = "ESP32_Vehicle";

// 3 Soil Moisture Sensors
const int SOIL_PINS[3] = {34, 35, 32};
const int DRY_VALUE = 3200;
const int WET_VALUE = 1400;

int readMoisture(int pin)
{
    int raw = analogRead(pin);
    return constrain(map(raw, DRY_VALUE, WET_VALUE, 0, 100), 0, 100);
}

void setup()
{
    Serial.begin(115200);
    SerialBT.begin("ESP32_Station", true); // master mode
    Serial.println("Sender ready. Looking for vehicle...");
}

void loop()
{
    if (!SerialBT.connected(500))
    {
        Serial.println("Connecting to vehicle...");
        SerialBT.connect(VEHICLE_NAME);
    }
    else
    {
        int soil1 = readMoisture(SOIL_PINS[0]);
        int soil2 = readMoisture(SOIL_PINS[1]);
        int soil3 = readMoisture(SOIL_PINS[2]);

        // Format: S,<soil1>,<soil2>,<soil3>,<temp>,<hum>,<light>
        SerialBT.printf("S,%d,%d,%d,%.1f,%.0f,%d\n",
                        soil1, soil2, soil3, 28.5, 65, 70);
        Serial.println("Data sent to vehicle");
        delay(2000);
    }
}