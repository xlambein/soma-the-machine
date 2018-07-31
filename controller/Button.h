#ifndef __BUTTON__H_
#define __BUTTON__H_

class Button {
  private:
    int pin;
    bool pressed;

  public:
    Button(int pin) :
      pin(pin),
      pressed(false) {}

    void setup() {
      pinMode(pin, INPUT_PULLUP);
    }

    bool is_pressed() {
      if (digitalRead(pin) == LOW) {
        if (!pressed) {
          pressed = true;
          return true;
        } else {
          return false;
        }
      } else {
        pressed = false;
        return false;
      }
    }
};

#endif

