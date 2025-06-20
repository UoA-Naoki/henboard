#define TOUCH_PINS 14

uint8_t pins[TOUCH_PINS] = {14,12,10,7,6,4,1,2,5,9,11,13,8,3};
long thresholds[TOUCH_PINS] = {
  95000,  // 1
  85000,  // 2
  120000, // 3
  95000,  // 4
  90000,  // 5
  90000,  // 6
  85000,  // 7
  110000, // 8
  70000,  // 9
  90000,  //10
  95000,  //11
  100000, //12
  90000,  //13
  95000   //14
};
bool previous_pressed[TOUCH_PINS];
bool pressed[TOUCH_PINS];
bool send;

void setup() {
  for (int i = 0; i < TOUCH_PINS; ++i) {
    previous_pressed[i] = false;
    pressed[i] = false;
  }
  send = false;
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
    previous_pressed[i] = pressed[i];
    pressed[i] = thresholds[pins[i]-1] < touchRead(pins[i]);
    if (previous_pressed[i] == pressed[i] || !pressed[i]) continue;
    send = true;
  }
  if (send) {
    for (i = 0; i < 7; ++i) output |= pressed[i] << i;
    Serial.println(output);
    send = false;
  }
}
