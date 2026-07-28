// ================= FAN TEST =================

const int triacPin = 27;
const int zeroCrossPin = 34;

volatile int currentFanStage = 0;

const uint16_t gatePulseUs = 100;

hw_timer_t *triacTimer = NULL;

const uint16_t dimmingDelays[6] =
    {
        0,
        7000,
        6200,
        4600,
        2800,
        1200};

// ================= RELAYS =================

const int relayPins[] = {5, 18, 19, 21, 22};

#define RELAY_ON HIGH
#define RELAY_OFF LOW

// ================= TRIAC =================

void IRAM_ATTR fireTriac()
{
    digitalWrite(triacPin, HIGH);
    delayMicroseconds(gatePulseUs);
    digitalWrite(triacPin, LOW);
}

void IRAM_ATTR onTriacTimer()
{
    fireTriac();
}

void IRAM_ATTR zeroCrossISR()
{
    static uint32_t lastInterruptTime = 0;

    uint32_t now = micros();

    if ((now - lastInterruptTime) < 3000)
        return;

    lastInterruptTime = now;

    if (currentFanStage <= 0)
    {
        digitalWrite(triacPin, LOW);
        return;
    }

    timerWrite(triacTimer, 0);

    timerAlarm(
        triacTimer,
        dimmingDelays[currentFanStage],
        false,
        0);
}

void setFanStage(int stage)
{
    if (stage < 0)
        stage = 0;
    if (stage > 5)
        stage = 5;

    noInterrupts();
    currentFanStage = stage;
    interrupts();

    Serial.print("Fan Speed -> ");
    Serial.println(stage);
}

// ================= SETUP =================

void setup()
{
    Serial.begin(115200);

    // Relay setup
    for (int i = 0; i < 5; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], RELAY_OFF);
    }

    // Fan setup
    pinMode(triacPin, OUTPUT);
    digitalWrite(triacPin, LOW);

    pinMode(zeroCrossPin, INPUT);

    triacTimer = timerBegin(1000000);

    timerAttachInterrupt(
        triacTimer,
        &onTriacTimer);

    attachInterrupt(
        digitalPinToInterrupt(zeroCrossPin),
        zeroCrossISR,
        CHANGE);
    setFanStage(5);

    Serial.println("Hardware Test Started...");
    Serial.println("Fan Running at Speed 5");
}

// ================= LOOP =================

void loop()
{
    // Keep fan running at full speed

    // Turn ON relays one by one
    digitalWrite(5, HIGH);
    Serial.println("GPIO 5 ON");
    delay(3000);

    digitalWrite(18, HIGH);
    Serial.println("GPIO 18 ON");
    delay(3000);

    digitalWrite(19, HIGH);
    Serial.println("GPIO 19 ON");
    delay(3000);

    digitalWrite(21, HIGH);
    Serial.println("GPIO 21 ON");
    delay(3000);

    digitalWrite(22, HIGH);
    Serial.println("GPIO 22 ON");
    delay(3000);

    // Turn OFF relays one by one
    digitalWrite(5, LOW);
    Serial.println("GPIO 5 OFF");
    delay(3000);

    digitalWrite(18, LOW);
    Serial.println("GPIO 18 OFF");
    delay(3000);

    digitalWrite(19, LOW);
    Serial.println("GPIO 19 OFF");
    delay(3000);

    digitalWrite(21, LOW);
    Serial.println("GPIO 21 OFF");
    delay(3000);

    digitalWrite(22, LOW);
    Serial.println("GPIO 22 OFF");
    delay(3000);
}