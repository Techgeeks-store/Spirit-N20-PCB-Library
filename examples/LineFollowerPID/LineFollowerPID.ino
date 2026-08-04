/*
  LineFollowerPID - SpiritN20

  A complete suction line follower, using an ARC8 sensor array on the front
  connector.

  The two libraries split the job between them:

    the ARC8 library  reads the track and tells you how far off you are
    this library      turns that into steering, drives the motors, and
                      runs the impeller

  So the sensor array has no PID of its own. getError() gives you the
  position, and Robot.getCorrection() does the maths.

  A NEGATIVE correction means the line went right, because sensor 0 is at
  the right-hand end of the array and carries a negative weight. That is why
  the correction is subtracted from the left wheel and added to the right.

  Startup follows the manual: BTN1 to calibrate, BTN1 again for the
  impeller, BTN2 to run. The impeller comes up to speed and builds vacuum
  BEFORE the wheels turn, so there is downforce for the first corner.

  Needs the ARC8 library installed as well as this one.
*/

#include <SpiritN20.h>
#include <ARC8.h>

SpiritN20 Robot;
ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

const int ROBOT_SPEED    = 180;
const int IMPELLER_SPEED = 40;   // start here, raise in steps of 5-10
const int CALIB_SPIN     = 50;

void setup() {
  Serial.begin(9600);
  Robot.begin();
  Robot.setBattery(3);            // <-- CHANGE THIS to match your pack
  Robot.setPID(0.03, 0.0, 0.8);   // kp, ki, kd

  sensors.begin(pins);
  sensors.setLine(BLACK);

  // --- press 1: calibrate ---
  Serial.println(F("Press BTN1 to calibrate."));
  Robot.waitFor(BTN1);
  delay(1000);

  Robot.ledOn(LED1);
  Robot.drive(CALIB_SPIN, -CALIB_SPIN);   // spin on the spot over the line
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Robot.stop();
  Robot.ledOff(LED1);

  // --- press 2: impeller ---
  Serial.println(F("Press BTN1 again for the impeller."));
  Robot.waitFor(BTN1);
  delay(1000);

  Robot.setImpeller(IMPELLER_SPEED);   // climbs there on its own
  Robot.ledOn(LED1);
  delay(1500);                         // let the vacuum build

  // --- press 3: go ---
  Serial.println(F("Press BTN2 to run."));
  Robot.waitFor(BTN2);
  delay(1000);
  Robot.ledOn(LED2);
}

void loop() {
  sensors.read(values);

  int correction = Robot.getCorrection(sensors.getError());

  Robot.drive(ROBOT_SPEED - correction, ROBOT_SPEED + correction);
}
