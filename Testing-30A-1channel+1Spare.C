#define RELAY1 23
#define RELAY2 25

void setup() {
  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  // Initially both OFF
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

  Serial.println("Relay Test Started");
}

void loop() {

  // Step 1: Turn ON Relay 1
  Serial.println("GPIO 23 ON");
  digitalWrite(RELAY1, HIGH);
  delay(3000);

  // Step 2: Turn ON Relay 2
  Serial.println("GPIO 25 ON");
  digitalWrite(RELAY2, HIGH);
  delay(3000);

  // Step 3: Turn OFF Relay 1
  Serial.println("GPIO 23 OFF");
  digitalWrite(RELAY1, LOW);
  delay(3000);

  // Step 4: Turn OFF Relay 2
  Serial.println("GPIO 25 OFF");
  digitalWrite(RELAY2, LOW);
  delay(3000);
}