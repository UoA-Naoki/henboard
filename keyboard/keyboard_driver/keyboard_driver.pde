import processing.serial.*;
import java.awt.Robot;
import java.awt.event.KeyEvent;

String port = "/dev/cu.usbserial-110";
Serial serial;
Robot robot;
int i = 0;

void setup() {
  println("Error opening serial port " + port + ": Port busy =~=^=~-~=~=^=^=~=~-~> close serial monitor on arduino ide ");
  println("Error, disabling serialEvent() for " + port + " =~=-~=~=^-~-~-> retry");
  serial = new Serial(this, port, 115200);
  try {
    robot = new Robot();
  } catch(Exception ignored) {}
}

void draw() {
}

void serialEvent(Serial p) {
  String data = p.readStringUntil('\n');
  if (data == null) return;
  int input = Integer.parseInt(data.substring(0, data.length() - 2));
  println(input);
  int keycode=KeyEvent.getExtendedKeyCodeForChar(input);
  println(keycode);
  switch(input){
    case 33:
    keycode=49;
    break;
    case 34:
    keycode=50;
    break;
    case 35:
    keycode=51;
    break;
    case 36:
    keycode=52;
    break;
    case 37:
    keycode=53;
    break;
    case 38:
    keycode=54;
    break;
    case 39:
    keycode=55;
    break;
    case 40:
    keycode=56;
    break;
    case 41:
    keycode=57;
    break;
    case 42:
    keycode=222;
    break;
    case 43:
    keycode=59;
    break;
    case 58://no shift
    keycode=222;
    break;
    case 60:
    keycode=44;
    break;
    case 61:
    keycode=45;
    break;
    case 62:
    keycode=46;
    break;
    case 63:
    keycode=47;
    break;
    case 64://no shift
    keycode=91;
    break;
    case 91://no shift
    keycode=93;
    break;
    case 92://?????????????????????????????????
    keycode=0;
    break;
    case 93://no shift
    keycode=92;
    break;
    case 94://no shift
    keycode=61;
    break;
    case 95://?????????????????????????????????
    keycode=0;
    break;
    case 123:
    keycode=93;
    break;
    case 124://????????????????????????????????
    keycode=0;
    break;
    case 125:
    keycode=92;
    break;
    case 126:
    keycode=61;
    break;
  }
  if(keycode==0) return;
  
  if((input>=65&&input<=90)||(input>=33&&input<=43)||(input>=60&&input<=63)||input==123||input==125||input==126){
    robot.keyPress(16);
  }
  
  robot.keyPress(keycode);
  robot.keyRelease(keycode);
  
  if((input>=65&&input<=90)||(input>=33&&input<=43)||(input>=60&&input<=63)||input==123||input==125||input==126){
    robot.keyRelease(16);
  }
}
