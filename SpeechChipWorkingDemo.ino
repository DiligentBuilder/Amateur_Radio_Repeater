const byte ledPin = 13;
const byte interruptPin = 5;
volatile byte state = LOW;

void speak(char* msg) {
  Serial.write(0xFD);
  Serial.write((byte)0x0);
  Serial.write(2 + strlen(msg));
  Serial.write(0x01);
  Serial.write((byte)0x0);
  Serial.write(msg);
}

void busy() {
  state = !state;
  digitalWrite(ledPin, state);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), busy, CHANGE);
  Serial.begin(9600);
}

void loop() {
  speak("[x0][t6][v8][s6][m51][g2][h2][n1]K 3 A U K Testing.");
  delay(5000);
}