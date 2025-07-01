#define TOUCH_PINS 14

uint8_t pins[TOUCH_PINS] = {14,12,10,7,6,4,1,2,5,9,11,13,8,3};
long thresholds[TOUCH_PINS] = {
  98000,  // 1
  83000,  // 2
  120000, // 3
  91000,  // 4
  90000,  // 5
  85000,  // 6
  85000,  // 7
  110000, // 8
  70000,  // 9
  85000,  //10
  95000,  //11
  95000,  //12
  88000,  //13
  88000   //14
};

void setup() {
  Serial.begin(115200);
}

uint8_t current=0;
uint8_t previous=0;
uint8_t big=0;

void loop() {
  int i = 0;
  previous=current;
  current=0;
  for (; i < 7; ++i) {
    current |= (thresholds[pins[i]-1] < touchRead(pins[i])) << i;
  }
  if(current==0&&previous!=0){
    Serial.println(big);
    big=0;
  }else{
    if(current>big){
      big=current;
    }
  }
}
