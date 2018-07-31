#ifndef __PHOTO_CELL__H_
#define __PHOTO_CELL__H_

const unsigned long PHOTO_CELL_DELAY = 500;

class PhotoCell {
  private:
    int pin;
    int threshold;
    unsigned long millis_timer;

  public:
    PhotoCell(int pin, int threshold) :
      pin(pin),
      threshold(threshold),
      millis_timer(0) {}

    void setup() {
      pinMode(pin, INPUT);
    }

    bool is_activated() {
      if (analogRead(pin) > threshold) {
        if (millis_timer < millis()) {
          millis_timer = millis() + PHOTO_CELL_DELAY;
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
};

#endif

