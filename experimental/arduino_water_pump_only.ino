// =====================================================
// Arduino Uno - Water Pump Only (No LFR)
// Experimental version - testing pump cycle with delay()
// Pattern: 10s ON -> 2s OFF -> Repeat Continuously
// =====================================================

const int RELAY_PIN = 7;

void setup()
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    delay(1000);
}

void loop()
{
    digitalWrite(RELAY_PIN, LOW);
    delay(10000);

    digitalWrite(RELAY_PIN, HIGH);
    delay(2000);
}