// =====================================================
// ESP32 - Blynk IoT Monitoring Only (No Bluetooth)
// Experimental version - testing Blynk connection
// 3x Soil Moisture Sensors + DHT22
// =====================================================

#include "secrets/secrets.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

// Pin - 3 Soil Moisture Sensors
#define SOIL1_PIN 34
#define SOIL2_PIN 35
#define SOIL3_PIN 32

// DHT22 Pin Definition
#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

int readSoilMoisture(int pin)
{
    int value = analogRead(pin);
    int percentage = map(value, 4095, 1500, 0, 100);
    return constrain(percentage, 0, 100);
}

void sendSensorData()
{
    int soil1 = readSoilMoisture(SOIL1_PIN);
    int soil2 = readSoilMoisture(SOIL2_PIN);
    int soil3 = readSoilMoisture(SOIL3_PIN);

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    Serial.println("==========================================");
    Serial.print("Soil Sensor 1: ");
    Serial.print(soil1);
    Serial.println("%");
    Serial.print("Soil Sensor 2: ");
    Serial.print(soil2);
    Serial.println("%");
    Serial.print("Soil Sensor 3: ");
    Serial.print(soil3);
    Serial.println("%");

    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("Warning: Failed to read from DHT sensor!");
    }
    else
    {
        Serial.print("Air Humidity: ");
        Serial.print(humidity, 1);
        Serial.println("%");
        Serial.print("Temperature : ");
        Serial.print(temperature, 1);
        Serial.println(" C");
    }

    if (Blynk.connected())
    {
        Blynk.virtualWrite(V0, soil1);
        Blynk.virtualWrite(V3, soil2);
        Blynk.virtualWrite(V4, soil3);

        if (!isnan(humidity) && !isnan(temperature))
        {
            Blynk.virtualWrite(V1, temperature);
            Blynk.virtualWrite(V2, humidity);
        }
    }
}

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    delay(1000);
    dht.begin();
    WiFi.begin(ssid, pass);
    Blynk.config(auth);
    Blynk.connect(10000);
    timer.setInterval(2500L, sendSensorData);
}

void loop()
{
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
}