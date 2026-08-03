/*
  StartupSequence - SpiritN20

  The startup procedure from the Spirit manual, with nothing else in the way.
  Three presses, and the robot does nothing until you say so:

    press BTN1  ->  LED1 on, setup step runs, LED1 off
    press BTN1  ->  impeller spins up, both LEDs on
    press BTN2  ->  robot drives

  waitFor() is what makes this work. It parks your sketch on that line until
  the button is pressed, then carries on.

  The order matters. The impeller comes up to speed and gets a moment to
  build vacuum BEFORE the wheels turn. Roll off early and you have no
  downforce for the first corner, which is exactly where you wanted it.

  setImpeller() climbs to the speed you ask for on its own, a step at a
  time, so there is nothing to ramp by hand. It does wait while it climbs.

  This is where a sensor calibration sweep would go, using the ARC8 or
  ARC16 library. Here the setup step just spins the robot on the spot and
  blinks, so the sketch stands on its own.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <SpiritN20.h>

SpiritN20 Robot;

const int IMPELLER_SPEED   = 40;   // start here, raise in steps of 5-10
const int ROBOT_SPEED      = 180;
const int CALIBRATION_SPIN = 50;   // how fast it turns during setup

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Robot.setBattery(3);   // <-- CHANGE THIS to match your pack

  // --- press 1: setup ---
  Serial.println("Press BTN1 to start setup.");
  Robot.waitFor(BTN1);
  delay(1000);                    // let go of the button before we move off

  Robot.ledOn(LED1);
  Serial.println("Setting up...");

  // Your real setup step goes here. A sensor calibration sweep would spin
  // the robot like this while reading the array over both surfaces.
  Robot.drive(CALIBRATION_SPIN, -CALIBRATION_SPIN);
  delay(2000);
  Robot.stop();

  Robot.ledOff(LED1);
  Serial.println("Setup done.");

  // --- press 2: impeller ---
  Serial.println("Press BTN1 again to start the impeller.");
  Robot.waitFor(BTN1);
  delay(1000);

  Serial.println("Spinning up...");
  Robot.setImpeller(IMPELLER_SPEED);   // climbs there on its own
  Robot.ledOn(LED1);
  Robot.ledOn(LED2);

  // --- press 3: go ---
  Serial.println("Press BTN2 to run.");
  Robot.waitFor(BTN2);
  delay(1000);

  Serial.println("Going!");
}

void loop() {
  // Drive forward. This is where your line following would go.
  Robot.drive(ROBOT_SPEED, ROBOT_SPEED);
}
