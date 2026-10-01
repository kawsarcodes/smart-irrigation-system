// =====================================================
// ESP32 #1 - Master Station (Monitoring)
// Combined: Blynk IoT (WiFi) + Bluetooth Sender
// Reads 3x Soil Moisture + DHT22 + LDR
// Sends data to Blynk Cloud AND vehicle ESP32 via Bluetooth
// =====================================================

#include "secrets/secrets.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include "BluetoothSerial.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

BluetoothSerial SerialBT;
const char *VEHICLE_NAME = "ESP32_Vehicle";

// 3 Soil Moisture Sensors
#define SOIL1_PIN 34
#define SOIL2_PIN 35
#define SOIL3_PIN 32

// DHT22
#define DHT_PIN 4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// LDR
#define LDR_PIN 33

BlynkTimer timer;

// Sensor values (global so both Blynk and BT can use)
int soil1 = 0, soil2 = 0, soil3 = 0;
float temperature = 0.0;
float humidity = 0.0;
int lightLevel = 0;

// ---------- Sensor Reading ----------
int readSoilMoisture(int pin)
{
    int value = analogRead(pin);
    int percentage = map(value, 4095, 1500, 0, 100);
    return constrain(percentage, 0, 100);
}

int readLight()
{
    int raw = analogRead(LDR_PIN);
    return constrain(map(raw, 4095, 0, 0, 100), 0, 100);
}

// ---------- Read All Sensors ----------
void readAllSensors()
{
    soil1 = readSoilMoisture(SOIL1_PIN);
    soil2 = readSoilMoisture(SOIL2_PIN);
    soil3 = readSoilMoisture(SOIL3_PIN);

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature))
    {
        temperature = 0;
        humidity = 0;
    }

    lightLevel = readLight();

    // Serial output
    Serial.println("==========================================");
    Serial.print("Soil 1: ");
    Serial.print(soil1);
    Serial.println("%");
    Serial.print("Soil 2: ");
    Serial.print(soil2);
    Serial.println("%");
    Serial.print("Soil 3: ");
    Serial.print(soil3);
    Serial.println("%");
    Serial.print("Temp  : ");
    Serial.print(temperature, 1);
    Serial.println(" C");
    Serial.print("Humid : ");
    Serial.print(humidity, 1);
    Serial.println("%");
    Serial.print("Light : ");
    Serial.print(lightLevel);
    Serial.println("%");
}

// ---------- Send to Blynk ----------
void sendToBlynk()
{
    readAllSensors();

    if (Blynk.connected())
    {
        Blynk.virtualWrite(V0, soil1);
        Blynk.virtualWrite(V3, soil2);
        Blynk.virtualWrite(V4, soil3);
        Blynk.virtualWrite(V1, temperature);
        Blynk.virtualWrite(V2, humidity);
    }
}

// ---------- Send to Vehicle via Bluetooth ----------
void sendToVehicle()
{
    if (!SerialBT.connected(500))
    {
        Serial.println("Connecting to vehicle...");
        SerialBT.connect(VEHICLE_NAME);
        return;
    }

    // Format: S,<soil1>,<soil2>,<soil3>,<temp>,<hum>,<light>
    SerialBT.printf("S,%d,%d,%d,%.1f,%.0f,%d\n",
                    soil1, soil2, soil3,
                    temperature, humidity, lightLevel);
    Serial.println("Data sent to vehicle via Bluetooth");
}

// ---------- Setup ----------
void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    delay(1000);

    analogReadResolution(12);
    dht.begin();

    // Start Bluetooth (master mode)
    SerialBT.begin("ESP32_Station", true);
    Serial.println("Bluetooth master started: ESP32_Station");

    // Start WiFi + Blynk
    WiFi.begin(ssid, pass);
    Blynk.config(auth);
    Blynk.connect(10000);

    // Timer: read + send every 2.5 seconds
    timer.setInterval(2500L, sendToBlynk);
}

// ---------- Loop ----------
void loop()
{
    // Keep Blynk connection alive
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!Blynk.connected())
        {
            Blynk.connect(1000);
        }
        else
        {
            Blynk.run();
        }
    }
    timer.run();

    // Send to vehicle via Bluetooth every 2 seconds
    static unsigned long lastBT = 0;
    if (millis() - lastBT >= 2000)
    {
        lastBT = millis();
        sendToVehicle();
    }
}