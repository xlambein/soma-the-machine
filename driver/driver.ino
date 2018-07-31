#include <Wire.h>

#include "SlotMotor.h"

SlotMotor step3(2, 3, 4, 5, 14);
SlotMotor step2(6, 7, 8, 9, 15);
SlotMotor step1(10, 11, 12, 13, 16);

enum CommandCode {
  Calibrate = 1,
  Next = 2,
  Previous = 3,
  Seek = 4,
  Rotate = 5,
  RotateSeek = 6
};

void setup() {
  Wire.begin(8);
  Wire.onReceive(receive_event);
  Wire.onRequest(notify_motors_state);

  Serial.begin(9600);

  delay(500);

  step1.setup();
  step2.setup();
  step3.setup();

  Serial.println("Started up!");
}

void loop() {
  delay(MOTOR_DELAY);
  
  step1.update();
  step2.update();
  step3.update();
}

void receive_event(int howMany) {
  int c = Wire.read();
  if (c == -1) return;

  int m = Wire.read();
  Serial.print("Motor ");
  Serial.println(m);

  SlotMotor *motor;
  switch (m) {
    case 1:
      motor = &step1;
      break;

    case 2:
      motor = &step2;
      break;

    case 3:
      motor = &step3;
      break;

    default:
      Serial.println("Error: This motor does not exist");
      return;
  }

  int t, l;

  switch (c) {
    case CommandCode::Calibrate:
      Serial.print("Calibrate motor");
      motor->calibrate();
      break;

    case CommandCode::Next:
      motor->next();
      break;

    case CommandCode::Previous:
      motor->previous();
      break;

    case CommandCode::Seek:
      l = Wire.read();
      Serial.print("Seek ");
      Serial.println(l);
      motor->seek(l);
      break;

    case CommandCode::Rotate:
      t = Wire.read();
      Serial.print("Rotate ");
      Serial.print(t);
      Serial.println(" turns");
      motor->rotate(t);
      break;

    case CommandCode::RotateSeek:
      t = Wire.read();
      l = Wire.read();
      Serial.print("Rotate ");
      Serial.print(t);
      Serial.print(" turns then seek ");
      Serial.println(l);
      motor->rotate_seek(t, l);
      break;

    default:
      Serial.print("Unknown command ");
      Serial.println(c);
  }
}

void notify_motors_state() {
  Serial.println("Answering request...");
  Wire.write((byte) step1.has_command());
  Wire.write((byte) step2.has_command());
  Wire.write((byte) step3.has_command());
}

