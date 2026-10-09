// =====================================================
// Arduino Uno - Vehicle (LFR + Water Pump Control)
// Combined: PID Line Following + Relay Water Pump
// =====================================================

// ---------- SENSOR PINS ----------
const int sensorPins[5] = {2, 3, 4, 5, 6};

// ---------- L298N MOTOR DRIVER ----------
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

// ---------- RELAY (WATER PUMP) ----------
const int RELAY_PIN = 7;

// ---------- SENSOR LOGIC ----------
const bool BLACK_IS_HIGH = false;

// ---------- PID SETTINGS ----------
float Kp = 45.0;
float Ki = 0.0;
float Kd = 25.0;
int baseSpeed = 120;
int maxSpeed = 220;

// ---------- PID VARIABLES ----------
float error = 0;
float lastError = 0;
float integral = 0;

// ---------- PUMP TIMING ----------
const unsigned long PUMP_ON_MS = 10000; // 10 seconds
const unsigned long PUMP_OFF_MS = 2000; // 2 seconds
unsigned long pumpTimer = 0;
bool pumpOn = false;

// =====================================================
// SETUP
// =====================================================
void setup()
{
    Serial.begin(9600);

    // Motor pins
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Relay pin
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH); // pump OFF initially

    // IR sensor pins
    for (int i = 0; i < 5; i++)
    {
        pinMode(sensorPins[i], INPUT);
    }

    setMotors(0, 0);

    Serial.println("Vehicle starting...");
    delay(3000);

    // Start pump cycle timer
    pumpTimer = millis();
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop()
{
    // Always run line following
    runLFR();

    // Always run pump cycle in parallel
    runPumpCycle();
}

// =====================================================
// PID LINE FOLLOWING
// =====================================================
void runLFR()
{
    float position = readSensors();

    // Line lost
    if (position == 999)
    {
        if (lastError < 0)
        {
            setMotors(-100, 100);
        }
        else
        {
            setMotors(100, -100);
        }
        return;
    }

    // PID
    error = position;
    integral += error;
    integral = constrain(integral, -100, 100);

    float derivative = error - lastError;
    float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
    lastError = error;

    int leftSpeed = baseSpeed + correction;
    int rightSpeed = baseSpeed - correction;

    leftSpeed = constrain(leftSpeed, -maxSpeed, maxSpeed);
    rightSpeed = constrain(rightSpeed, -maxSpeed, maxSpeed);

    setMotors(leftSpeed, rightSpeed);
}

// =====================================================
// SENSOR READING
// =====================================================
float readSensors()
{
    int sensorValue[5];
    float weights[5] = {-2.0, -1.0, 0.0, 1.0, 2.0};
    float weightedSum = 0;
    int activeSensors = 0;

    for (int i = 0; i < 5; i++)
    {
        sensorValue[i] = digitalRead(sensorPins[i]);
        bool lineDetected = BLACK_IS_HIGH ? (sensorValue[i] == HIGH)
                                          : (sensorValue[i] == LOW);

        if (lineDetected)
        {
            weightedSum += weights[i];
            activeSensors++;
        }
    }

    if (activeSensors == 0)
        return 999;
    return weightedSum / activeSensors;
}

// =====================================================
// MOTOR CONTROL
// =====================================================
void setMotors(int left, int right)
{
    // Left motor
    if (left > 0)
    {
        digitalWrite(IN1, LOW);
        analogWrite(IN2, constrain(left, 0, 255));
    }
    else if (left < 0)
    {
        analogWrite(IN1, constrain(-left, 0, 255));
        digitalWrite(IN2, LOW);
    }
    else
    {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
    }

    // Right motor
    if (right > 0)
    {
        analogWrite(IN3, constrain(right, 0, 255));
        digitalWrite(IN4, LOW);
    }
    else if (right < 0)
    {
        digitalWrite(IN3, LOW);
        analogWrite(IN4, constrain(-right, 0, 255));
    }
    else
    {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
    }
}

// =====================================================
// WATER PUMP CYCLE (10s ON, 2s OFF)
// Runs in parallel with LFR (no blocking delay)
// =====================================================
void runPumpCycle()
{
    unsigned long now = millis();
    unsigned long elapsed = now - pumpTimer;

    if (!pumpOn && elapsed >= PUMP_OFF_MS)
    {
        // Turn pump ON
        digitalWrite(RELAY_PIN, LOW);
        pumpOn = true;
        pumpTimer = now;
        Serial.println("Pump ON");
    }
    else if (pumpOn && elapsed >= PUMP_ON_MS)
    {
        // Turn pump OFF
        digitalWrite(RELAY_PIN, HIGH);
        pumpOn = false;
        pumpTimer = now;
        Serial.println("Pump OFF");
    }
}