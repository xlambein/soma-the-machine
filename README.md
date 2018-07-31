# Introduction

This repository contains the Arduino code for the project "The Machine"
realised during the last week of School of Ma's "Winner and Losers" course, by
Maryam Aghajani, Anna Brynskov and Xavier Lambein.  The project is an
"alternative slot machine" that generates random sentences.  Instead of the
classical numbers and images, the three reels of this slot machine each have 21
phrases.  When activated, the Machine's reels rotate and stop randomly on one
phrase, generating a full sentence.  On each reel, the 21 phrases are split
into three categories (with 7 phrases each): "Poetry", "Question" and
"Challenge".  When operating the Machine, the user can choose the category they
want, and the Machine will draw phrases from the specified category to create a
sentence.

The Machine's input are a coin slot, which detects when a user inserts a coin,
and a set of three buttons, labeled "Poetry", "Question" and "Challenge", to
select a category.  The Machine is operated in the following way: First, the
user picks up a coin and inserts it into the coin slot.  Then, they press one
of the three buttons.  Finally, the reels start spinning, and stop on random
phrases, creating a sentence.

# Technical Implementation

The Machine is made of the following components:

- A painted wooden box, housing all the components
- A wooden frame, holding the reels and the Arduino microcontrollers
- A coin slot, attached to the wooden box, made with an LED and a light
  detector
- Two Arduino Uno, one serving as a motor driver, and the other handling the
  user interface and the random sentence generation
- Three cardboard reels with phrases on them, each powered by a stepper motor
- A set of four Neo-Pixels, acting as "eyes" for the Machine
- A light with changing hue, attached in the box, creating ambiant lighting
- An LED strip, attached on the inside near the reels, lighting up the current
  sentence
- An additional set of Neo-Pixels around the coin slot, controlled by a third
  Arduino Uno

## The Coin Slot

The coin slot is a cardboard tube, attached to the front of the machine.
Within this tube are a blue LED facing a light sensor.  While falling, a coin
inserted into the slot will block the light for a few miliseconds, which is
detected by the light sensor.

## The Cardboard Reels

The reels have three main components:

- A stepper motor
- A cardboard frame on which the phrases are attached, and which is rotated by
  the motor
- A magnet, attached to the inside of the cardboard frame, and a Reed switch,
  attached to the static part of stepper motor

The magnet and Reed switch are used for calibrating the motor.  When the motor
rotates, the (rotating) magnet passes in front of the (static) Reed switch,
closing the circuit.

# Description of the Source Code

The code provided in this repository is the Arduino code for the motor driver
and master controller handling the UI and logic.  The driver code is given in
the `driver/` directory, while the master controller is given in the
`controller/` directory.

## The Motor Driver

The driver code is structured in the following way.  The `StepMotor` class (in
`StepMotor.h`) describes a stepper motor controlled by four input pins.  It
provides an interface that allows to rotate it forward or backward, get its
current position (its step counter), and turn it off.

The `SlotMotor` class (in `SlotMotor.h`) describes a slot machine motor.  It
inherits `StepMotor`.  It manages calibration with the Reed switch, and
provides abstractions for performing complex actions such as rotating a full
turn and selecting a specific location on the reel.  In addition, it controls
the automatic shutdown of the motor when it is not being used.  This is done to
avoid overheating.

To perform complex actions `SlotMotor` can take "commands" which are pieces of
code executed at each time step (by calling `update`) until they reach
completion.  Commands are child classes of the `StepMotorCommand` class, and
use the common ["command" design
pattern](https://en.wikipedia.org/wiki/Command_pattern).

- `CalibrateCommand` rotates forward until the Reed switch is closed, and then
  records the current position of the motor.
- `SeekCommand` takes a target position as argument, and rotates forward until
  that target is reached.
- `RotateCommand` takes a number of turns as argument, and rotates forward for
  that number of turns.
- `ChainedCommand` takes two commands as arguments, and executes them one after
  the other.

The chained command is useful because `SlotMotor` can only have a single
command running at once.  Hence, if we want to rotate 3 times and then stop at
position 54, this can be modeled with a single chained command, itself
containing a rotate command and a seek command.

The `SlotMotor` class comes with a few methods that encapsulate the running of
commands.  `calibrate`, `seek` and `rotate` match the corresponding commands.
`rotate_seek` corresponds to a chained calibration, rotation and seek.  The
other methods are never used.

Finally, `driver.ino` describes the logic for the motor driver Arduino.
Basically, the driver is a slave I2C device controlled by another Arduino.  It
is constantly listening for messages telling it to execute commands on the
motors.  It can also notify its master of the current state of each motor
(running a command or idle).

It expects messages with 2 or more bytes, corresponding to a command code, a
motor ID, and possibly arguments.  For example, to tell motor 2 to rotate 3
turns and seek position 54, the master Arduino would send the four bytes `6`
(rotate-seek command code), `2` (motor ID), `3` (number of turns) and `54`
(position).

When requested by the master, the driver sends three bytes of data,
corresponding to the state of each motor (`0` being idle, `1` being running a
command).

## The Controller

The main code for the controller is given in `controller.ino`.  Due to the time
constraints of the project, its code is not the cleanest.  Two helper classes
are given in `Button.h` and `PhotoCell.h`.  They provide abstractions for the
buttons and coin slot's light sensor, respectively.

The code in `loop()` is the most important.  It checks if a coin has been
inserted, and if a button has been pressed.  If the user had previously
inserted a coin and a button is pressed, the coin is "consumed", a random
sentence is generated, and the motor driver is told to rotate each motor a few
times before stopping at the selected phrases.

Every 300 miliseconds, the controller asks the driver for the motor state.  If
the motors have stopped rotating, the "eyes" Neo-Pixels are made to go
off-and-on a few times.

The position of each label for each category and motor is listed in the array
`targets`.  Hence, when generating a sentence from, say, the second category,
three random numbers between 0 and 6 are generated, and then the motors are
told to go to positions `targets[1][0][r1]`, `targets[1][1][r1]`,
`targets[1][2][r3]`, where `r1` to `r3` are the random numbers.

The function `rotate_seek` is used to tell the driver to rotate a given motor
and then seek the position corresponding to a specific phrase.  Because phrase
labels are bigger than individual steps of the motor, the position of a label
is first converted into a step position.  Unfortunately, a printing mistake
made the first and third reels have smaller labels than the second, and so each
motor has its own, different label size.

