int photocellPin = A5;
int photocellReading;

void setup() {
  Serial.begin(9600);
  photocellReading = analogRead(A5);
}

void loop() {

  photocellReading = analogRead(A5);

  Serial.print("Light = ");
  Serial.println(photocellReading);

  delay(10);

  if (photocellReading > 130) {
    theaterChase(strip.Color(200, 60, 127), 50);
  } else {
    strip.Color(0, 0, 0);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

