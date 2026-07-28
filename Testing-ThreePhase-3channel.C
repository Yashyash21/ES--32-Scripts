// Relay Pins
const int relayPins[] = {23, 25, 26};
const int relayCount = sizeof(relayPins) / sizeof(relayPins[0]);

void setup()
{
    Serial.begin(115200);

    for (int i = 0; i < relayCount; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW); // Relay OFF
    }

    Serial.println("Sequential Relay Test Started...");
}

void loop()
{

    // Turn ON one by one
    for (int i = 0; i < relayCount; i++)
    {
        Serial.print("GPIO ");
        Serial.print(relayPins[i]);
        Serial.println(" ON");

        digitalWrite(relayPins[i], HIGH);
        delay(5000);
    }

    // Turn OFF one by one
    for (int i = 0; i < relayCount; i++)
    {
        Serial.print("GPIO ");
        Serial.print(relayPins[i]);
        Serial.println(" OFF");

        digitalWrite(relayPins[i], LOW);
        delay(5000);
    }
}