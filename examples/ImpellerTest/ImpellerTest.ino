/*
  ImpellerTest - SpiritN20

  Brings the suction impeller up to speed slowly, holds it, then winds it
  back down. Press BTN1 at any time to start or stop it.

  ==========================================================================
   READ THIS BEFORE YOU RUN IT
  ==========================================================================

  1. SET YOUR BATTERY. setBattery(2) for a 2 cell LiPo, setBattery(3) for a
     3 cell. This is what limits the impeller speed and protects the motor.
     Leave it out and the library assumes 3 cells, which is the safer guess
     but will feel weak on a 2 cell pack.

  2. RAMP UP, NEVER JUMP. Going straight to full power throws a big current
     spike into the 8520 and heats it immediately. This sketch climbs in
     steps of 5, which is the pattern you want in your own code too.

  3. CHECK THE TEMPERATURE after each run. Touch the impeller motor body.
     Warm is fine. Too hot to hold your finger on for 2 seconds is not.
     Lower the target speed or rest longer between runs.

  4. REST BETWEEN RUNS. Give it 30 to 60 seconds. This matters most when
     the Y Pad is shorted and the motor is seeing full battery voltage.

  5. MAKE SURE THE SKIRT IS SEATED before expecting real downforce. Air
     leaking in from the sides kills the vacuum.

  ==========================================================================

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <SpiritN20.h>

SpiritN20 Robot;

const int TARGET_SPEED = 40;   // start here. Raise in steps of 5-10.
const int STEP         = 5;    // how much to climb each time
const int STEP_DELAY   = 100;  // milliseconds between steps

bool running = false;
int  speedNow = 0;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Robot.setBattery(2);   // <-- CHANGE THIS to match your pack

  Serial.println("Press BTN1 to spin the impeller up.");
}

void loop() {
  if (Robot.isPressed(BTN1, 20)) {
    while (Robot.isPressed(BTN1));   // wait for your finger to come off

    running = !running;

    if (running) {
      Robot.ledOn(LED1);
      Serial.println("ramping up...");
      for (speedNow = 0; speedNow <= TARGET_SPEED; speedNow += STEP) {
        Robot.setImpeller(speedNow);
        Serial.println(speedNow);
        delay(STEP_DELAY);
      }
      Serial.println("at speed. Press BTN1 again to wind down.");
    } else {
      Serial.println("winding down...");
      for (; speedNow >= 0; speedNow -= STEP) {
        Robot.setImpeller(speedNow);
        delay(STEP_DELAY);
      }
      Robot.setImpeller(0);
      speedNow = 0;
      Robot.ledOff(LED1);
      Serial.println("stopped. Let the motor rest before the next run.");
    }
  }
}
