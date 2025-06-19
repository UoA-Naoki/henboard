import processing.serial.*;
import java.awt.Robot;
import java.awt.event.KeyEvent;

String port = "COM3";
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
  print(data);
  int keyCode = Integer.parseInt(data.substring(0, data.length() - 2));
  robot.keyPress(keyCode);
  robot.delay(1);
  robot.keyRelease(keyCode);
}
