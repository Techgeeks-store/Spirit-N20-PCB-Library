/*
  ButtonsAndLEDs - SpiritN20

  BTN1 turns both LEDs on. BTN2 turns them off.

  About the buttons: they are wired so that pressing one connects the pin to
  GND. That means a plain digitalRead() gives you 0 when pressed, which is
  backwards from what you would expect. isPressed() flips it round, so:

      if (Robot.isPressed(BTN1))   // true when you are holding it down

  Add a number for a debounce when you are counting presses rather than
  asking whether a button is held:

      if (Robot.isPressed(BTN1, 20))   // ignores contact bounce

  Note on LED2: it sits on a pin that shares a timer with the impeller.
  Turning it on and off is completely safe. Do NOT call analogWrite() on
  that pin to dim it - that would disturb the impeller.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <SpiritN20.h>

SpiritN20 Robot;

void setup() {
  Serial.begin(9600);
  Robot.begin();

  Serial.println("BTN1 for lights on, BTN2 for lights off.");
}

void loop() {
  if (Robot.isPressed(BTN1)) {
    Robot.ledOn(LED1);
    Robot.ledOn(LED2);
    Serial.println("BTN1 - lights on");
  }

  if (Robot.isPressed(BTN2)) {
    Robot.ledOff(LED1);
    Robot.ledOff(LED2);
    Serial.println("BTN2 - lights off");
  }
}
