
/*
  Stepper Motor Control - one revolution

  This program drives a unipolar or bipolar stepper motor.
  The motor is attached to digital pins 8 - 11 of the Arduino.

  The motor should revolve one revolution in one direction, then
  one revolution in the other direction.


  Created 11 Mar. 2007
  Modified 30 Nov. 2009
  by Tom Igoe

*/

#include <Stepper.h>

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

int speed = 60;
int steps = 0;
int dir = 1;

void setup() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(speed);
  // initialize the serial port:
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    parse();
  }
}

void execute() {
  // step one revolution  in one direction:
  myStepper.setSpeed(speed);
  if (dir == 1) {
    myStepper.step(steps);
  }
  if (dir == 2) {
    myStepper.step(-steps);
  }

  speed = 0;
  steps = 0;
  dir = 1;
}

void parse() {
  String first  = Serial.readStringUntil(',');
  Serial.read(); //next character is comma, so skip it using this
  String second = Serial.readStringUntil(',');
  Serial.read();
  String third  = Serial.readStringUntil('\n');

  speed = first.toInt();
  steps = second.toInt();
  dir = third.toInt();
  
  Serial.print("Speed: ");
  Serial.println(speed);
  Serial.print("Steps: ");
  Serial.println(steps);
  Serial.print("Dir: ");
  Serial.println(dir);
  
  execute();
}
