#ifndef __STEP_MOTOR__H_
#define __STEP_MOTOR__H_

const size_t STEP_MOTOR_N_PINS = 4;
const size_t STEP_MOTOR_N_STEPS = STEP_MOTOR_N_PINS * 2; // half-steps
const size_t STEP_MOTOR_MAX_STEPS = 96;

class StepMotor {
  private:
    int pins[4];
    int step;
    bool on;

  public:
    StepMotor(int a, int b, int c, int d) :
      pins({a, b, c, d}),
      step(0),
      on(false) {}
    
    virtual void setup() {
      for (int i = 0; i < STEP_MOTOR_N_PINS; i++)
        pinMode(pins[i], OUTPUT);
    }

    void forward() {
      on = true;
      step = (step + 1) % STEP_MOTOR_MAX_STEPS;
      update();
    }

    void backward() {
      on = true;
      step = (step + STEP_MOTOR_MAX_STEPS - 1) % STEP_MOTOR_MAX_STEPS;
      update();
    }

    int get_step() {
      return step;
    }

    void off() {
      on = false;
      update();
    }

    bool is_on() {
      return on;
    }
    
  protected:
    void update() {
      for (int i = 0; i < STEP_MOTOR_N_PINS; i++)
        digitalWrite(pins[i], LOW);

      if (on) {
        digitalWrite(pins[(step / 2) % STEP_MOTOR_N_PINS], HIGH);
        digitalWrite(pins[((step + 1) / 2) % STEP_MOTOR_N_PINS], HIGH);
      }
    }
};

#endif

