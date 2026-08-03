/*
  StartupSequence - SpiritN20

  The pattern almost every competition robot uses: nothing happens until a
  human says so.

    press BTN1  ->  LED1 on, setup step runs, LED1 off
    press BTN2  ->  impeller spins up, LED2 on, robot drives

  waitFor() is what makes this work. It parks your sketch on that line until
  the button is pressed, then carries on.

  The important bit for a suction robot is the order. The impeller is
  brought up to speed and given a moment to build vacuum BEFORE the wheels
  start turning. Roll off too early and you have no downforce for the first
  corner, which is exactly where you need it.

  This is where a sensor calibration sweep would go, using the ARC8 or
  ARC16 library. Here the setup step is just a blink, so the sketch stands
  on its own.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <SpiritN20.h>

SpiritN20 Robot;

const int IMPELLER_SPEED = 40;   // start here, raise in steps of 5-10
const int BASE_SPEED     = 120;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Robot.setBattery(2);   // <-- CHANGE THIS to match your pack

  Serial.println("Press BTN1 to start setup.");
  Robot.waitFor(BTN1);            // sketch pauses right here

  Robot.ledOn(LED1);
  Serial.println("Setting up...");

  // Your real setup step goes here - a sensor calibration sweep, say.
  for (int i = 0; i < 10; i++) {
    Robot.ledToggle(LED1);
    delay(200);
  }

  Robot.ledOff(LED1);
  Serial.println("Setup done. Press BTN2 to run.");
  Robot.waitFor(BTN2);            // and pauses again here

  // Impeller first, wheels second. Climb in steps rather than jumping.
  Serial.println("Spinning the impeller up...");
  for (int s = 0; s <= IMPELLER_SPEED; s += 5) {
    Robot.setImpeller(s);
    delay(100);
  }

  // Give the vacuum a moment to build under the skirt before moving off.
  delay(1500);

  Robot.ledOn(LED2);
  Serial.println("Going!");
}

void loop() {
  // Drive forward. This is where your line following would go.
  Robot.drive(BASE_SPEED, BASE_SPEED);

  // BTN1 stops everything, wheels and impeller both.
  if (Robot.isPressed(BTN1, 20)) {
    Robot.stop();                 // stop() is drive motors only...
    Robot.setImpeller(0);         // ...so switch the impeller off yourself
    Robot.ledOff(LED2);
    Serial.println("Stopped.");
    while (true) { }              // stay stopped until you reset the board
  }
}
