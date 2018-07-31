#ifndef __SLOT_MOTOR__H_
#define __SLOT_MOTOR__H_

#include "StepMotor.h"

const unsigned int MOTOR_DELAY = 15;
const unsigned int OFF_DELAY = 500;

struct StepMotorCommand {
  public:
    virtual bool exec(StepMotor &motor) = 0;
};

struct CalibrateCommand : public StepMotorCommand {
  private:
    unsigned int reed_pin;
    unsigned int *base_step;
  
  public:
    CalibrateCommand(unsigned int reed_pin, unsigned int *base_step) :
      reed_pin(reed_pin),
      base_step(base_step) {}
  
    bool exec(StepMotor &motor) {
      if (digitalRead(reed_pin) != LOW) {
        motor.forward();
        return false;
      } else {
        *base_step = motor.get_step();
        return true;
      }
    }
};

struct SeekCommand : public StepMotorCommand {
  private:
    unsigned int target;
  
  public:
    SeekCommand(unsigned int target) :
      target(target) {}
  
    bool exec(StepMotor &motor) {
      if (motor.get_step() != target) {
        motor.forward();
        return false;
      } else
        return true;
    }
};

struct RotateCommand : public StepMotorCommand {
  private:
    unsigned int remaining;
  
  public:
    RotateCommand(unsigned int turns) :
      remaining(turns * STEP_MOTOR_MAX_STEPS) {}
  
    bool exec(StepMotor &motor) {
      if (remaining > 0) {
        motor.forward();
        remaining -= 1;
      }
      return (remaining == 0);
    }
};

struct ChainedCommand : public StepMotorCommand {
  private:
    bool first_running;
    StepMotorCommand *first;
    StepMotorCommand *second;
  
  public:
    ChainedCommand(StepMotorCommand *first, StepMotorCommand *second) :
      first_running(true),
      first(first),
      second(second) {}
    
    ~ChainedCommand() {
      delete first;
      delete second;
    }
  
    bool exec(StepMotor &motor) {
      if (first_running) {
        first_running = !first->exec(motor);
        return false;
      } else {
        return second->exec(motor);
      }
    }
};

class SlotMotor : public StepMotor {
  private:
    unsigned int reed_pin;
    unsigned int base_step;
    unsigned long off_timer;

    StepMotorCommand *command;

  public:
    SlotMotor(int a, int b, int c, int d, unsigned int reed_pin) :
      StepMotor(a, b, c, d),
      reed_pin(reed_pin),
      base_step(0),
      off_timer(-1),
      command(NULL) {}
    
    virtual void setup() {
      StepMotor::setup();
      pinMode(reed_pin, INPUT_PULLUP);
      calibrate();
    }
    
    void calibrate() {
      if (command == NULL) {
        command = new ChainedCommand(new RotateCommand(1), new CalibrateCommand(reed_pin, &base_step));
      } else {
        Serial.println("Error: Tried to calibrate while other command is running.");
      }
    }

    void seek(int location) {
      if (command == NULL) {
        if (location < 0 || location >= STEP_MOTOR_MAX_STEPS) {
          Serial.println("Error: Location must be within [0; STEP_MOTOR_MAX_STEPS)");
          return;
        }
        command = new SeekCommand((location + base_step) % STEP_MOTOR_MAX_STEPS);
      } else {
        Serial.println("Error: Tried to seek while other command is running.");
      }
    }

    void next() {
      seek((get_step() + 1) % STEP_MOTOR_MAX_STEPS);
    }
    
    void previous() {
      seek((get_step() + STEP_MOTOR_MAX_STEPS - 1) % STEP_MOTOR_MAX_STEPS);
    }

    void rotate(int turns) {
      if (command == NULL) {
        if (turns < 1 || turns > 10) {
          Serial.println("Error: Cannot turn less than 1 or more than 10 turns");
          return;
        }
        command = new RotateCommand(turns);
      } else {
        Serial.println("Error: Tried to rotate while other command is running.");
      }
    }

    void rotate_seek(int turns, int location) {
      if (command == NULL) {
        if (turns < 1 || turns > 10) {
          Serial.println("Error: Cannot turn less than 1 or more than 10 turns");
          return;
        }
        if (location < 0 || location >= STEP_MOTOR_MAX_STEPS) {
          Serial.println("Error: Location must be within [0; STEP_MOTOR_MAX_STEPS)");
          return;
        }
        command = new ChainedCommand(new CalibrateCommand(reed_pin, &base_step), new ChainedCommand(new RotateCommand(turns), new SeekCommand((location + base_step) % STEP_MOTOR_MAX_STEPS)));
      } else {
        Serial.println("Error: Tried to rotate while other command is running.");
      }
    }

    void update() {
      if (command != NULL) {
        if (command->exec(*this)) {
          delete command;
          command = NULL;
        }
        off_timer = millis() + OFF_DELAY;
      }
      if (off_timer <= millis())
        off();
    }

    bool has_command() {
      return (command != NULL);
    }
};

#endif

