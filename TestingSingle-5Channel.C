// Relay Pins
const int relayPins[] = {5, 18, 19, 21, 22};
const int relayCount = sizeof(relayPins) / sizeof(relayPins[0]);

void setup()
{
    Serial.begin(115200);

    for (int i = 0; i < relayCount; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW); // OFF
    }

    Serial.println("Sequential Relay Test Started...");
}

void loop()
{

    // Turn ON one by one
    for (int i = 0; i < relayCount; i++)
    {
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.println(" ON");

        digitalWrite(relayPins[i], HIGH);
        delay(5000);
    }

    // Turn OFF one by one
    for (int i = 0; i < relayCount; i++)
    {
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.println(" OFF");

        digitalWrite(relayPins[i], LOW);
        delay(5000);
    }
}