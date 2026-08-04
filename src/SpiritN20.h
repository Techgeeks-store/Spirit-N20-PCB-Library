/*
  SpiritN20 - Techgeeks Spirit PCB, N20 Model

  One library for everything built into the board: two N20 drive motors
  through the TB6612FNG, the suction impeller, two push buttons and two
  status LEDs.

  Every pin is fixed in the PCB itself, so begin() takes no arguments.

  The impeller runs on Timer2 at 62.5 kHz, set up by hand rather than with
  analogWrite(). That keeps it silent and easy on the coreless motor, but it
  does tie this library to the Arduino Nano's chip.
*/

#ifndef SPIRITN20_H
#define SPIRITN20_H

#include <Arduino.h>

#if !defined(__AVR_ATmega328P__) && !defined(__AVR_ATmega328__) && \
    !defined(__AVR_ATmega168P__) && !defined(__AVR_ATmega168__)
  #error "SpiritN20 is for the Arduino Nano (ATmega328P). The impeller uses Timer2 and pin D11 = OC2A, which belong to that chip."
#endif

// Motor driver pins - wired into the PCB, do not change these.
#define AIN1  7    // Motor A is the LEFT motor
#define AIN2  6
#define PWMA  5    // Timer0
#define BIN1  8    // Motor B is the RIGHT motor
#define BIN2  9
#define PWMB  10   // Timer1

// The impeller. D11 is OC2A, which is why Timer2 is set up by hand.
#define IMPELLER 11

// Buttons and LEDs.
#define BTN1 12
#define BTN2 4
#define LED1 2
#define LED2 3     // also OC2B - only ever use digitalWrite on this pin

class SpiritN20 {
  public:
    // Gets the library ready with sensible starting settings.
    SpiritN20();

    // Sets up every pin, starts the impeller timer, and stops everything.
    void begin();

    // Tells the library how many cells your battery has, 2 or 3.
    void setBattery(int cells);

    // Runs the impeller. 0 is off, higher is more suction.
    void setImpeller(int speed);

    // Raises or lowers the highest impeller speed the library will allow.
    void setImpellerMAX(int maxSpeed);

    // Drives both motors. -255 to 255 each. 0 brakes that motor.
    void drive(int speedA, int speedB);

    // Brakes both drive motors. The impeller keeps running.
    void stop();

    // True while the button is held down. Pass BTN1 or BTN2.
    bool isPressed(int button, unsigned int debounceMs = 0);

    // Pauses your sketch until the button is pressed. Pass BTN1 or BTN2.
    void waitFor(int button);

    // Sets the three numbers that decide how hard the robot steers.
    void setPID(float kp, float ki, float kd);

    // Turns how far off the line you are into a steering number.
    int getCorrection(int error);

    // Turns an LED on. Pass LED1 or LED2.
    void ledOn(int led);

    // Turns an LED off. Pass LED1 or LED2.
    void ledOff(int led);

    // Flips an LED: on becomes off, off becomes on. Pass LED1 or LED2.
    void ledToggle(int led);

  private:
    bool _began;             // has begin() been called?

    int  _cells;             // 2 or 3, however many your battery has
    int  _cap;               // the highest impeller speed allowed right now
    int  _impeller;          // the impeller speed actually being run

    bool _ledState[2];       // what we last told each LED to do

    float _kp, _ki, _kd;     // the three PID steering numbers
    int   _lastError;        // the error from the previous getCorrection()
    long  _integral;         // every error so far, added up. A long, because
                             // an int fills up after only a few loops

    // Only used when you ask isPressed() for a debounce.
    bool          _lastRaw[2];     // the reading we saw last time
    bool          _stable[2];      // the reading that has settled down
    unsigned long _lastChange[2];  // when the reading last flipped

    // Warnings print only once, so one mistake cannot flood the
    // Serial Monitor thousands of times a second.
    bool _warnedNoBegin;
    bool _warnedButton;
    bool _warnedLed;
    bool _warnedCells;
    bool _warnedCapped;
    bool _warnedReverse;
    bool _warnedCapRaised;

    // The safe ceiling for the battery you told us about.
    int  _safeCap();

    // Writes the impeller speed out to the timer.
    void _applyImpeller(int speed);

    // Sends one motor forwards, backwards, or into a brake.
    void _driveMotor(int in1, int in2, int pwm, int speed);

    // Turns BTN1 into 0 and BTN2 into 1. Gives -1 and warns otherwise.
    int _buttonIndex(int button);

    // Turns LED1 into 0 and LED2 into 1. Gives -1 and warns otherwise.
    int _ledIndex(int led);

    // Reads a button right now. True means pressed.
    bool _readButton(int button);

    // True if begin() has run. Warns once if it has not.
    bool _checkBegan();
};

#endif
