/*https://github.com/esp32beans/ESP32USBMIDI/tree/mainを参考にした*/

#if ARDUINO_USB_MODE//USB MODE is not USB-OTG 
#warning This sketch must be used when USB is in OTG mode
void setup() {}
void loop() {}
#else//USB MODE is USB-OTG

#include "USB.h"
#include "esp32-hal-tinyusb.h"

#define NUM_TOUCH_KEYS 12

// Basic MIDI Messages
#define NOTE_OFF 0x80
#define NOTE_ON 0x90

int status;// = digitalRead(BUTTON[2]);//17

const int BUTTON[4] = {15, 16, 17, 18};//ボタンで機能切り替え用

const char *TAG = "usbdmidi"; //ログを取る時に使う。

const int touchPin[NUM_TOUCH_KEYS] = {
    1, 2, 8, 3, 6, 7, 9, 10, 11, 12, 13, 14// T1, T2, T3, T6, T7, T8, T9, T10, T11, T12, T13, T14を使う
};
/*const int touchPin[NUM_TOUCH_KEYS] = {
    1, 2, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14// T1, T2, T3, T6, T7, T8, T9, T10, T11, T12, T13, T14を使う
};*/

const int touchPin2[NUM_TOUCH_KEYS] = {
    1, 2, 8, 3, 6, 7, 9, 10, 11, 12, 13, 14// T1, T2, T3, T6, T7, T8, T9, T10, T11, T12, T13, T14を使う
};

int midiNote[NUM_TOUCH_KEYS] = {
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71 //ドからシ(C4~B4)
};

//オクターブの増減をするピン
const int OCTAVE_UP_PIN = 3;  // T3
const int OCTAVE_DOWN_PIN   = 8; // T8

int octaveOffset = 0; // MIDIノートへのオフセット値 (+12 or -12)
const int MAX_OCTAVE_SHIFT = 2; // 最大2オクターブまでシフト
const int MIN_OCTAVE_SHIFT = -2; // 最小-2オクターブまでシフト

//機能キーの状態を保存する変数
bool isOctUpPressed = false;
bool isOctDownPressed = false;

//const int TOUCH_THRESHOLD[12] = {95000 ,85000 ,95000 ,90000 ,90000 ,85000 ,70000 ,90000 ,95000 ,100000 ,90000 ,95000};// タッチ検出値。この値より大きくなったらタッチされたと判断
//const int TOUCH_THRESHOLD[12] = {95000 ,85000 ,120000 ,110000 ,90000 ,85000 ,70000 ,90000 ,95000 ,100000 ,90000 ,95000};// タッチ検出値。この値より大きくなったらタッチされたと判断
const int TOUCH_THRESHOLD[12] = {80000 ,105000 ,100000 ,90000 ,79000 ,80000 ,70000 ,90000 ,93000 ,99000 ,88000 ,90000};// タッチ検出値。この値より大きくなったらタッチされたと判断
//触った時の瞬時の値　　88900(T1),115000(T2),125000(T3),134000(T8),85000(T6),86000(T7),75000(T9),98000(T10),98000(T11),97000(T12),96000(T13),97000(T14)


const int touchThreshold3 = 110000;
const int touchThreshold8 = 120000;


bool isKeyPressed[NUM_TOUCH_KEYS] = {false};// 各キーが現在タッチされているかどうかの状態を保存する配列

// グローバルなMIDIパラメータ
uint8_t const cable_num = 0;
uint8_t const channel = 0;

/** TinyUSB descriptors **/
extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf) {
  uint8_t str_index = tinyusb_add_string_descriptor("TinyUSB MIDI");
  uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
  TU_VERIFY(ep_num != 0);
  uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
      TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num), 64)
  };
  *itf += 1;
  memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
  return TUD_MIDI_DESC_LEN;
}

// From usb.org MIDI 1.0 specification. This 4 byte structure is the unit
// of transfer for MIDI data over USB.
typedef struct __attribute__((__packed__)) {
  uint8_t code_index_number : 4;
  uint8_t cable_number : 4;
  uint8_t MIDI_0;
  uint8_t MIDI_1;
  uint8_t MIDI_2;
} USB_MIDI_t;

static void midi_task_read_example(void *arg) {
  // The MIDI interface always creates input and output port/jack descriptors
  // regardless of these being used or not. Therefore incoming traffic should be
  // read (possibly just discarded) to avoid the sender blocking in IO
  uint8_t packet[4];
  for (;;) {
    delay(1);
    while (tud_midi_available()) {
      if (tud_midi_packet_read(packet)) {
        ESP_LOGI(TAG,
                 "Read - Time (ms since boot): %lld, Data: %02hhX %02hhX "
                 "%02hhX %02hhX",
                 esp_timer_get_time(), packet[0], packet[1], packet[2],
                 packet[3]);
        USB_MIDI_t *m = (USB_MIDI_t *)packet;
        Serial.printf(
            "%lld: Cable: %d Code: %01hhX, Data: %02hhX %02hhX %02hhX\n",
            esp_timer_get_time(), m->cable_number, m->code_index_number,
            m->MIDI_0, m->MIDI_1, m->MIDI_2);
      }
    }
  }
}

void app_main(void) {
  // MIDI受信タスクを開始する
  ESP_LOGI(TAG, "MIDI read task init");
  xTaskCreate(midi_task_read_example, "midi_task_read_example", 2 * 1024, NULL, 5, NULL);
}

//対象のタッチセンサーが反応したらオクターブの増減をする
void checkFunctionKeys(int octDownNow, int octUpNow){
  //int octUpValue = touchRead(OCTAVE_UP_PIN);
  //bool octUpNow = (octUpValue >= touchThreshold3);

  if ((octUpNow == LOW) && !(isOctUpPressed == octUpNow)) { // 押された瞬間
  //if(statusUp == LOW) { 
    if (octaveOffset < (MAX_OCTAVE_SHIFT * 12)) {
      octaveOffset += 12;
      Serial.printf("Octave UP! Offset: %d\n", octaveOffset);
      delay(1000);
    } else {
      Serial.println("Max octave reached!");
    }
  }
  isOctUpPressed = octUpNow;

 // int octDownValue = touchRead(OCTAVE_DOWN_PIN);
  //bool octDownNow = (octDownValue >= touchThreshold8);
  if ((octDownNow == LOW) && !(isOctDownPressed == octDownNow)) { // 押された瞬間
  //if(statusDown == LOW) {
    if (octaveOffset > (MIN_OCTAVE_SHIFT * 12)) {
      octaveOffset -= 12;
      Serial.printf("Octave DOWN! Offset: %d\n", octaveOffset);
      delay(1000);
    } else {
      Serial.println("Min octave reached!");
    }
  }
  isOctDownPressed = octDownNow;
}


void checkTouchKeys(){
  // NUM_TOUCH_KEYSの数だけループ
  for (int i = 0; i < NUM_TOUCH_KEYS; i++) {
    int touchValue = touchRead(touchPin[i]);
    
    // 触れているかどうかの判定
    bool isPressedNow = (touchValue >= TOUCH_THRESHOLD[i]);

    // 状態が変化した瞬間だけ処理を実行
    if (isPressedNow != isKeyPressed[i]) {
      int finalNote = midiNote[i] + octaveOffset;//オクターブが変わっていたら、出力する値を変える
      //midiの範囲0~127であることを確認
      if (finalNote >= 0 && finalNote <= 127) {
        if (isPressedNow) {
          // 触れられた瞬間: Note On を送信
          uint8_t note_on[3] = {NOTE_ON | channel, (uint8_t)finalNote, 127};
          tud_midi_stream_write(cable_num, note_on, 3);
          Serial.printf("Pin %d (Note %d) ON - Value: %d\n", touchPin[i], finalNote, touchValue);
        } else {
          // 離された瞬間: Note Off を送信
          uint8_t note_off[3] = {NOTE_OFF | channel, (uint8_t)finalNote, 0};
          tud_midi_stream_write(cable_num, note_off, 3);
          Serial.printf("Pin %d (Note %d) OFF - Value: %d\n", touchPin[i], finalNote, touchValue);
        }
      }
      
      //現在の状態を保存する
      isKeyPressed[i] = isPressedNow; 
      delay(1);
    }
  }
}

void checkButton(){
  for(int i=0;i<4;i++){
    int status = digitalRead(BUTTON[i]);
    if(status == LOW){
      Serial.print("Pushed Button GPIO ");
      Serial.println(BUTTON[i]);
    }
  }
  delay(10);
}


static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id) {
    case ARDUINO_USB_STARTED_EVENT:
      Serial.println("USB PLUGGED");
      break;
    case ARDUINO_USB_STOPPED_EVENT:
      Serial.println("USB UNPLUGGED");
      break;
    case ARDUINO_USB_SUSPEND_EVENT:
      Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n",
                    data->suspend.remote_wakeup_en);
      break;
    case ARDUINO_USB_RESUME_EVENT:
      Serial.println("USB RESUMED");
      break;

    default:
      break;
    }
  }
}

void checkReadTouch(int status) {
  if(status == LOW){
    for(int i=0;i<3;i++){
      Serial.print(touchPin2[i]);
      Serial.print(":\t");
    //if(touchRead(touchPin[i])>80000){
      //Serial.println("touched");
    //}else{
      //Serial.println("released");
     //}
      Serial.println(touchRead(touchPin2[i]));
    }
    for(int i=5;i<14;i++){
      Serial.print(touchPin2[i]);
      Serial.print(":\t");
    //if(touchRead(touchPin[i])>80000){
      //Serial.println("touched");
    //}else{
      //Serial.println("released");
     //}
      Serial.println(touchRead(touchPin2[i]));
    }
  }

}

void setup() {
  Serial.begin(115200);
  for(int i=0;i<4;i++){
    pinMode(BUTTON[i],INPUT_PULLUP);
  }

  USB.onEvent(usbEventCallback);
  tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, tusb_midi_load_descriptor);
  USB.begin();
  while (!Serial && millis() < 5000) delay(10);
  app_main();
}

void loop() {
  //checkButton();
  status = digitalRead(BUTTON[2]);//17
  int octUpNow = digitalRead(BUTTON[0]);//15
  int octDownNow = digitalRead(BUTTON[1]);//16
  checkReadTouch(status);
  checkFunctionKeys(octUpNow, octDownNow);
  checkTouchKeys();
  delay(1);
}
#endif /* ARDUINO_USB_MODE */