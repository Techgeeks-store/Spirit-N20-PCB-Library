#include "SpiritN20.h"

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

SpiritN20::SpiritN20() {
  _began = false;

  // Start on the safest assumption. If your battery is really a 2S you get a
  // weaker impeller until you say so, which is the right way round to be
  // wrong - the other way round cooks the motor.
  _cells    = 3;
  _cap      = 100;
  _impeller = 0;

  for (int i = 0; i < 2; i++) {
    _ledState[i]   = false;
    _lastRaw[i]    = false;
    _stable[i]     = false;
    _lastChange[i] = 0;
  }

  _warnedNoBegin   = false;
  _warnedButton    = false;
  _warnedLed       = false;
  _warnedCells     = false;
  _warnedCapped    = false;
  _warnedReverse   = false;
  _warnedCapRaised = false;
}

void SpiritN20::begin() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  // INPUT_PULLUP holds the pin HIGH on its own, and pressing the button
  // pulls it down to GND. That is why a pressed button reads LOW, and why
  // isPressed() flips it round for you.
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  _ledState[0] = false;
  _ledState[1] = false;

  // ---- Impeller: Timer2, by hand ----
  //
  // analogWrite() would drive this pin at about 490 Hz. You can hear that as
  // a whine from the coreless motor, and the current through the coil keeps
  // jumping up and down, which wastes power and makes heat.
  //
  // Setting the timer directly with no prescaler gives 16 MHz / 256, which
  // is 62,500 Hz. Far above hearing, and the current stays smooth.
  //
  // This is the one place the library talks to the chip's registers, and it
  // is why the library only runs on the Nano.
  DDRB  |=  (1 << PB3);    // D11 as an output
  PORTB &= ~(1 << PB3);    // and start it LOW

  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM21) | (1 << WGM20);  // fast PWM, 8 bit
  TCCR2A |= (1 << COM2A1);                // drive OC2A (D11), not inverted
  TCCR2B |= (1 << CS20);                  // no prescaler -> 62.5 kHz

  OCR2A = 0;               // impeller off to begin with
  _impeller = 0;

  _began = true;
  stop();                  // brake both drive motors, known starting state
}

// ---------------------------------------------------------------------------
// Battery and the impeller ceiling
// ---------------------------------------------------------------------------

int SpiritN20::_safeCap() {
  // Both work out to roughly 5V average at the motor, which is what the
  // 8520 is happy running at.
  //   2 cells: 8.4V  x (150 / 255)  = about 5V
  //   3 cells: 12.6V x (100 / 255)  = about 5V
  return (_cells == 2) ? 150 : 100;
}

void SpiritN20::setBattery(int cells) {
  if (!_checkBegan()) return;

  if (cells != 2 && cells != 3) {
    if (!_warnedCells) {
      Serial.println(F("SpiritN20: battery must be 2 or 3"));
      _warnedCells = true;
    }
    return;  // keep whatever was set before
  }

  _cells = cells;
  _cap   = _safeCap();

  // If the impeller is already spinning faster than the new battery allows,
  // bring it down right now rather than waiting for the next setImpeller().
  if (_impeller > _cap) _applyImpeller(_cap);
}

void SpiritN20::setImpellerMAX(int maxSpeed) {
  if (!_checkBegan()) return;

  maxSpeed = constrain(maxSpeed, 0, 255);

  // Going past the battery's safe ceiling is allowed, because sometimes you
  // need it. Say so once, so nobody does it by accident and then wonders
  // why the impeller motor is too hot to touch.
  if (maxSpeed > _safeCap() && !_warnedCapRaised) {
    Serial.println(F("SpiritN20: impeller cap raised above the safe limit"));
    _warnedCapRaised = true;
  }

  _cap = maxSpeed;
  if (_impeller > _cap) _applyImpeller(_cap);
}

void SpiritN20::setImpeller(int speed) {
  if (!_checkBegan()) return;

  // The speed controller can only spin the impeller one way, which is all an
  // impeller ever needs. A negative number means someone expected it to
  // reverse like a drive motor.
  if (speed < 0) {
    if (!_warnedReverse) {
      Serial.println(F("SpiritN20: impeller only runs one way"));
      _warnedReverse = true;
    }
    speed = 0;
  }

  if (speed > _cap) {
    if (!_warnedCapped) {
      Serial.println(F("SpiritN20: impeller limited by the cap"));
      _warnedCapped = true;
    }
    speed = _cap;
  }

  _applyImpeller(speed);
}

void SpiritN20::_applyImpeller(int speed) {
  _impeller = speed;
  OCR2A = (uint8_t)speed;   // the duty cycle register for D11
}

// ---------------------------------------------------------------------------
// Drive motors
// ---------------------------------------------------------------------------

void SpiritN20::drive(int speedA, int speedB) {
  if (!_checkBegan()) return;

  // Anything past full speed is simply full speed. Quietly trimmed so a
  // stray big number never reaches analogWrite().
  speedA = constrain(speedA, -255, 255);
  speedB = constrain(speedB, -255, 255);

  _driveMotor(AIN1, AIN2, PWMA, speedA);
  _driveMotor(BIN1, BIN2, PWMB, speedB);
}

void SpiritN20::stop() {
  drive(0, 0);
}

// Sends one motor forwards, backwards, or into a brake.
//
// The driver chip decides direction from the two direction pins:
//   one HIGH, one LOW  -> spin, whichever way round they are
//   both HIGH          -> brake, the motor actively resists turning
//   both LOW           -> coast, the motor free-wheels to a halt
//
// A speed of 0 brakes rather than coasts. On a suction robot the wheels have
// a lot of grip, and braking is what makes a tight corner stop where you
// aimed it instead of sliding on.
void SpiritN20::_driveMotor(int in1, int in2, int pwm, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(pwm, speed);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(pwm, -speed);   // -speed is how fast, without the minus
  } else {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, HIGH);
    analogWrite(pwm, 0);
  }
}

// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------

bool SpiritN20::_readButton(int button) {
  // Pressed pulls the pin down to GND, so LOW means pressed.
  return digitalRead(button) == LOW;
}

bool SpiritN20::isPressed(int button, unsigned int debounceMs) {
  if (!_checkBegan()) return false;

  int i = _buttonIndex(button);
  if (i < 0) return false;

  bool reading = _readButton(button);

  // No debounce asked for: hand back exactly what the pin says, right now.
  if (debounceMs == 0) return reading;

  // Debounce asked for. A real push button does not switch cleanly - the
  // metal contacts bounce apart and back together for a few milliseconds,
  // so one press can look like several. The fix is to ignore a new reading
  // until it has stayed the same for long enough to be believable.
  //
  // Every time the reading flips we restart the clock. Only once it has
  // held steady for debounceMs do we accept it as the real answer. That
  // means this needs calling regularly - inside loop() is ideal.
  if (reading != _lastRaw[i]) {
    _lastRaw[i]    = reading;
    _lastChange[i] = millis();
  }

  if (millis() - _lastChange[i] >= debounceMs) {
    _stable[i] = reading;
  }

  return _stable[i];
}

void SpiritN20::waitFor(int button) {
  if (!_checkBegan()) return;

  // Check the button is real first. Without this a typo would hang the
  // sketch forever on a pin that is never going to go low.
  if (_buttonIndex(button) < 0) return;

  while (!_readButton(button)) {
    // Wait here doing nothing until someone presses it.
  }
}

int SpiritN20::_buttonIndex(int button) {
  if (button == BTN1) return 0;
  if (button == BTN2) return 1;

  if (!_warnedButton) {
    Serial.println(F("SpiritN20: unknown button"));
    _warnedButton = true;
  }
  return -1;
}

// ---------------------------------------------------------------------------
// LEDs
// ---------------------------------------------------------------------------

void SpiritN20::ledOn(int led) {
  if (!_checkBegan()) return;

  int i = _ledIndex(led);
  if (i < 0) return;

  _ledState[i] = true;
  digitalWrite(led, HIGH);
}

void SpiritN20::ledOff(int led) {
  if (!_checkBegan()) return;

  int i = _ledIndex(led);
  if (i < 0) return;

  _ledState[i] = false;
  digitalWrite(led, LOW);
}

void SpiritN20::ledToggle(int led) {
  if (!_checkBegan()) return;

  int i = _ledIndex(led);
  if (i < 0) return;

  // We remember what we last told the LED to do, rather than asking the pin.
  _ledState[i] = !_ledState[i];
  digitalWrite(led, _ledState[i] ? HIGH : LOW);
}

int SpiritN20::_ledIndex(int led) {
  if (led == LED1) return 0;
  if (led == LED2) return 1;

  if (!_warnedLed) {
    Serial.println(F("SpiritN20: unknown LED"));
    _warnedLed = true;
  }
  return -1;
}

// ---------------------------------------------------------------------------
// Private checks
// ---------------------------------------------------------------------------

bool SpiritN20::_checkBegan() {
  if (_began) return true;

  if (!_warnedNoBegin) {
    Serial.println(F("SpiritN20: call begin() first"));
    _warnedNoBegin = true;
  }
  return false;
}
