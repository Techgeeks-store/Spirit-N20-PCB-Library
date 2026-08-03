/*
  ImpellerTest - SpiritN20

  Press BTN1 to spin the suction impeller up, press it again to stop.

  setImpeller() climbs to the speed you ask for one step at a time rather
  than jumping, because a sudden jump to full power throws a big current
  spike into the coreless motor and heats it immediately. You do not have to
  ramp it yourself.

  It does wait while it climbs. Going from nothing to 100 takes about a
  second, and your sketch does nothing else meanwhile. Set the impeller
  before you start driving, not inside a fast loop. Coming back down is
  instant, since slowing has no spike to protect against.

  ==========================================================================
   BEFORE YOU RUN IT
  ==========================================================================

  1. SET YOUR BATTERY. setBattery(2) for a 2 cell LiPo, setBattery(3) for a
     3 cell. This is what limits the impeller speed and protects the motor.
     Leave it out and the library assumes 3 cells, which is the safer guess
     but will feel weak on a 2 cell pack.

  2. START LOW. 30 is a sensible first target. Raise it in steps of 5 to 10
     across runs until you have the grip you want, checking temperature each
     time.

  3. CHECK THE TEMPERATURE after each run. Touch the impeller motor body.
     Warm is fine. Too hot to hold your finger on for 2 seconds is not.

  4. REST BETWEEN RUNS. Give it 30 to 60 seconds. This matters most when
     the Y Pad is shorted and the motor is seeing full battery voltage.

  5. MAKE SURE THE SKIRT IS SEATED before expecting real downforce. Air
     leaking in from the sides kills the vacuum.

  ==========================================================================

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <SpiritN20.h>

SpiritN20 Robot;

const int TARGET_SPEED = 30;   // start here. Raise in steps of 5-10.

bool running = false;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Robot.setBattery(3);   // <-- CHANGE THIS to match your pack

  Serial.println("Press BTN1 to spin the impeller up.");
}

void loop() {
  if (Robot.isPressed(BTN1, 20)) {
    while (Robot.isPressed(BTN1));   // wait for your finger to come off

    running = !running;

    if (running) {
      Robot.ledOn(LED1);
      Serial.println("spinning up...");
      Robot.setImpeller(TARGET_SPEED);
      Serial.println("at speed. Press BTN1 again to stop.");
    } else {
      Robot.setImpeller(0);
      Robot.ledOff(LED1);
      Serial.println("stopped. Let the motor rest before the next run.");
    }
  }
}
