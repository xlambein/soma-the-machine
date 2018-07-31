#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#include "Button.h"
#include "PhotoCell.h"

const size_t STEP_MOTOR_MAX_STEPS = 96;

const unsigned int N_LABELS_MOTOR_1 = 23;
const unsigned int N_LABELS_MOTOR_2 = 21;
const unsigned int N_LABELS_MOTOR_3 = 23;

const float LABEL_SIZE_MOTOR_1 = (float) STEP_MOTOR_MAX_STEPS / (float) N_LABELS_MOTOR_1;
const float LABEL_SIZE_MOTOR_2 = (float) STEP_MOTOR_MAX_STEPS / (float) N_LABELS_MOTOR_2;
const float LABEL_SIZE_MOTOR_3 = (float) STEP_MOTOR_MAX_STEPS / (float) N_LABELS_MOTOR_3;

const int MOTOR_1_OFFSET = 3;
const int MOTOR_2_OFFSET = -6;
const int MOTOR_3_OFFSET = -1;

const unsigned long REQUEST_TIMER_DELAY = 300;

PhotoCell phot(A0, 200);  // exterior: 130, interior: 200

Button btn1(13);
Button btn2(12);
Button btn3(11);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, 3, NEO_GRB + NEO_KHZ800);

size_t n_targets[][3] = {
  {7, 7, 7},
  {7, 7, 7},
  {7, 7, 7}
};

int targets[][3][7] = {
  {
    {0, 1, 6, 7, 12, 13, 19},
    {0, 3, 6, 9, 12, 15, 18},
    {1, 4, 7, 10, 13, 16, 21}
  },
  {
    {2, 3, 8, 9, 14, 15, 20},
    {2, 5, 8, 11, 14, 17, 20},
    {2, 5, 8, 11, 14, 19, 22},
  },
  {
    {4, 5, 10, 11, 12, 21, 22},
    {1, 4, 7, 10, 13, 16, 19},
    {0, 3, 6, 9, 12, 15, 20}
  }
};

void seek_category(int category) {
  for (size_t i = 0; i < 3; i++) {
    int r = random(n_targets[category - 1][i]);
    int target = targets[category - 1][i][r];
    Serial.print("Seeking ");
    Serial.println(target);
    rotate_seek(i+1, i+1, target);  // i+1 turns
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  btn1.setup();
  btn2.setup();
  btn3.setup();
  phot.setup();

  strip.begin();
  reset_pixels();

  Wire.begin();

  randomSeed(analogRead(A1) + analogRead(A2) + analogRead(A3));
}

unsigned long rotate_timer = millis() + 5000;
unsigned long request_timer = 0;

bool motor_state[3] = {false, false, false};

bool has_coin = false;

void loop() {
  delay(10);

  bool motors_were_on = !are_all_motors_off();

  if (request_timer < millis()) {
    request_timer = millis() + REQUEST_TIMER_DELAY;

    if (Wire.available()) {
      for (int i = 0; i < 3; i++)
        motor_state[i] = (bool) Wire.read();
      Serial.print("Received bytes: ");
      for (int i = 0; i < 3; i++)
        Serial.print(motor_state[i]);
      Serial.println();
    } else {
      Serial.println("Sending request");
      Wire.requestFrom(8, 3);
    }
  }

  // Motors stopped running
  if (motors_were_on && are_all_motors_off()) {
    blink(strip.Color(200, 60, 127), 50);
    reset_pixels();
  }

  if (phot.is_activated()) {
    has_coin = true;
    Serial.println("Received coin");
  }

  if (has_coin && are_all_motors_off()) {
    if (btn1.is_pressed()) {
      Serial.println("Button 1");
      seek_category(1);
      has_coin = false;
    }

    if (btn2.is_pressed()) {
      Serial.println("Button 2");
      seek_category(2);
      has_coin = false;
    }

    if (btn3.is_pressed()) {
      Serial.println("Button 3");
      seek_category(3);
      has_coin = false;
    }
  }
}

void reset_pixels() {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(200, 60, 127));  //turn every pixel on
  }
  strip.show();
}

bool are_all_motors_off() {
  return (!motor_state[0] && !motor_state[1] && !motor_state[2]);
}

void rotate_seek(int motor, int turns, int location) {
  float label_size;
  int offset;
  switch (motor) {
    case 1:
      label_size = LABEL_SIZE_MOTOR_1;
      offset = MOTOR_1_OFFSET;
      break;

    case 2:
      label_size = LABEL_SIZE_MOTOR_2;
      offset = MOTOR_2_OFFSET;
      break;

    case 3:
      label_size = LABEL_SIZE_MOTOR_3;
      offset = MOTOR_3_OFFSET;
      break;

    default:
      Serial.println("This motor does not exist");
  }

  Wire.beginTransmission(8);
  Wire.write((byte) 6);
  Wire.write((byte) motor);
  Wire.write((byte) turns);
  Wire.write((byte) (((int)(location * label_size) + offset + STEP_MOTOR_MAX_STEPS) % STEP_MOTOR_MAX_STEPS));
  Wire.endTransmission();
}

void blink(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);  //turn every pixel on
    }
    strip.show();

    delay(wait);

    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0);      //turn every pixel off
    }
    strip.show();

    delay(wait);
  }
}

