// =====================================================
// ESP32 #2 - Vehicle Receiver
// Receives sensor data via Bluetooth from master station
// Displays data on OLED (SSD1306)
// =====================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

Adafruit_SSD1306 display(128, 64, &Wire, -1);
BluetoothSerial SerialBT;
String line = "";

// ---------- Parse Comma-Separated Value ----------
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;
    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// ---------- Show Data on OLED ----------
void showData(String l)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("Received data");

    display.setCursor(0, 14);
    display.printf("Soil %%: %s %s %s",
                   getValue(l, ',', 1).c_str(),
                   getValue(l, ',', 2).c_str(),
                   getValue(l, ',', 3).c_str());

    display.setCursor(0, 28);
    display.printf("Temp: %s C", getValue(l, ',', 4).c_str());

    display.setCursor(0, 40);
    display.printf("Humidity: %s %%", getValue(l, ',', 5).c_str());

    display.setCursor(0, 52);
    display.printf("Light: %s %%", getValue(l, ',', 6).c_str());

    display.display();
}

// ---------- Setup ----------
void setup()
{
    Serial.begin(115200);

    // Start Bluetooth (slave mode)
    SerialBT.begin("ESP32_Vehicle");
    Serial.println("Bluetooth slave started: ESP32_Vehicle");

    // OLED init
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("OLED not found");
        while (true)
            ;
    }
    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Waiting for data...");
    display.display();
}

// ---------- Loop ----------
void loop()
{
    while (SerialBT.available())
    {
        char c = SerialBT.read();
        if (c == '\n')
        {
            Serial.println("Received: " + line);
            if (line.startsWith("S,"))
                showData(line);
            line = "";
        }
        else if (c != '\r')
        {
            line += c;
        }
    }
}