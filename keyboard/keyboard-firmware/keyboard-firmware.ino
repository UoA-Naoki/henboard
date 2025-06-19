#define TOUCH_PINS 14

uint8_t pins[TOUCH_PINS] = {        1,      2,      3,      4,     5,     6,      7,      8,      9,     10,     11,     12,     13,     14};
long thresholds[TOUCH_PINS] = { 80000,  80000,  80000,  80000, 80000, 80000,  80000,  60000,  70000,  80000,  80000,  70000,  80000,  70000};


void setup() {
  Serial.begin(115200);
}

void loop() {
  uint8_t output = 0;
  int i = 0;
  for (; i < 7; ++i) {
    // Serial.print(i);
    // Serial.print(":\t");
    // Serial.print(thresholds[i] < touchRead(pins[i]));
    // Serial.print(" ");
    output |= (thresholds[i] < touchRead(pins[i])) << i;
  }
  // Serial.println();
  if (output == 0) return;
  if (90 - 64 < output) return;
  Serial.println(64 + output);
  delay(200);
}
