/*
  MotorTest - SpiritN20

  Drives both motors through every direction so you can check your wiring.
  Run this first, before anything else. The impeller stays off throughout.

  Watch the robot and note what it actually does:

    - If it drives BACKWARD when it says forward, swap the wires on BOTH
      motor terminals.
    - If it TURNS when it says forward, one motor is mounted the other way
      round. Swap the wires on that motor only.

  No code changes needed for either - it is all in the wiring.

  Put the robot on a stand or hold it up so it cannot drive off the table.
  Motor A is the LEFT motor, Motor B is the RIGHT one.
*/

#include <SpiritN20.h>

SpiritN20 Robot;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Serial.println("Motor test starting in 2 seconds...");
  delay(2000);
}

void loop() {
  Serial.println("forward");
  Robot.drive(150, 150);
  delay(1500);

  Serial.println("brake");
  Robot.stop();
  delay(1000);

  Serial.println("backward");
  Robot.drive(-150, -150);
  delay(1500);

  Robot.stop();
  delay(1000);

  Serial.println("turn right (left motor faster)");
  Robot.drive(150, 50);
  delay(1500);

  Serial.println("turn left (right motor faster)");
  Robot.drive(50, 150);
  delay(1500);

  Serial.println("spin on the spot");
  Robot.drive(-150, 150);
  delay(1500);

  Robot.stop();
  delay(2000);
}
