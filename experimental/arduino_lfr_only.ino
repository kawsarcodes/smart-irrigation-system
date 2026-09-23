// =====================================================
// Arduino Uno - Line Following Robot Only (No Pump)
// Experimental version - testing PID LFR
// =====================================================

// SENSOR PINS (Digital Pins 2, 3, 4, 5, 6)
const int sensorPins[5] = {2, 3, 4, 5, 6};

// L298N MOTOR CONTROL PINS
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

// SENSOR LOGIC (Active-LOW: LOW/0 = Black line)
const bool BLACK_IS_HIGH = false;

// PID SETTINGS
float Kp = 45.0;
float Ki = 0.0;
float Kd = 25.0;
int baseSpeed = 120;
int maxSpeed = 220;

// PID VARIABLES
float error = 0;
float lastError = 0;
float integral = 0;

void setup()
{
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    for (int i = 0; i < 5; i++)
    {
        pinMode(sensorPins[i], INPUT);
    }

    setMotors(0, 0);
    delay(3000);
}

void loop()
{
    runLFR();
}

void runLFR()
{
    float position = readSensors();

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

float readSensors()
{
    int sensorValue[5];
    float weights[5] = {-2.0, -1.0, 0.0, 1.0, 2.0};
    float weightedSum = 0;
    int activeSensors = 0;

    for (int i = 0; i < 5; i++)
    {
        sensorValue[i] = digitalRead(sensorPins[i]);
        bool lineDetected;

        if (BLACK_IS_HIGH)
        {
            lineDetected = (sensorValue[i] == HIGH);
        }
        else
        {
            lineDetected = (sensorValue[i] == LOW);
        }

        if (lineDetected)
        {
            weightedSum += weights[i];
            activeSensors++;
        }
    }

    if (activeSensors == 0)
    {
        return 999;
    }

    return weightedSum / activeSensors;
}

void setMotors(int left, int right)
{
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