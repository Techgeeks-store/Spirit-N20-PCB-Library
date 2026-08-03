# SpiritN20

The official Arduino library for the **Spirit PCB — N20 Model** by TECHGEEKS.

One library, one class, for everything built into the board: two N20 drive motors through the TB6612FNG, the suction impeller, two push buttons and two status LEDs.

Every pin is fixed in the PCB itself, so **`begin()` takes no arguments**.

---

## Contents

- [What the board is](#what-the-board-is)
- [Which board this runs on](#which-board-this-runs-on)
- [Installing the library](#installing-the-library)
- [Your first sketch](#your-first-sketch)
- [Pin names](#pin-names)
- [The impeller](#the-impeller)
- [Battery setting and the speed cap](#battery-setting-and-the-speed-cap)
- [Power: the X Pad and Y Pad](#power-the-x-pad-and-y-pad)
- [Drive motors](#drive-motors)
- [Buttons](#buttons)
- [LEDs](#leds)
- [Full function reference](#full-function-reference)
- [Example sketches](#example-sketches)
- [Warning messages and what to do about them](#warning-messages-and-what-to-do-about-them)
- [Using it with a sensor array](#using-it-with-a-sensor-array)
- [Things to know](#things-to-know)
- [License](#license)

---

## What the board is

The Spirit is a PCB that *is* the chassis. There is no separate frame and no standoffs stacking boards over a base plate — the motors, wheels, suction system and all the electronics mount straight onto the circuit board.

What makes it different from an ordinary line follower is the **impeller**. A small fan under the robot pulls air out from underneath, creating low pressure between the robot and the track. Atmospheric pressure above then pushes the robot down. That is downforce, and the Spirit generates well over 700 g of it at speed. For a robot weighing 120–140 g, that means it presses onto the track with five or six times its own weight.

The practical effect: the wheels grip far harder. Corners that would normally break traction at speed stay planted.

---

## Which board this runs on

**The Arduino Nano, and only the Arduino Nano.**

This is a deliberate exception. The ARC8 and ARC16 sensor libraries are written to be portable — no register access, so they can move to an ESP32 later. This one cannot be, because the impeller depends on the ATmega328P's Timer2 registers and on pin D11 being OC2A. On any other chip that pin mapping is wrong.

The library refuses to compile on anything else rather than silently driving the wrong pin. If you see an error mentioning the ATmega328P, check that **Tools → Board** is set to Arduino Nano.

---

## Installing the library

### Option 1 — Add .ZIP Library (recommended)

1. Download **`SpiritN20.zip`** from the [Releases page](https://github.com/Techgeeks-store/Spirit-N20-PCB-Library/releases).
2. In the Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library…**
3. Select the file you downloaded.

Restart the IDE, and the examples appear under **File → Examples → SpiritN20**.

### Option 2 — Download the whole repository

1. Click the green **Code** button at the top of this page, then **Download ZIP**.
2. Unzip it. You will get a folder called **`Spirit-N20-PCB-Library-main`**.
3. **Rename that folder to just `SpiritN20`.** This matters — the Arduino IDE wants the folder name to match the library name, and it does not like the dashes.
4. Move the `SpiritN20` folder into your sketchbook's `libraries` folder:
   - **Windows:** `Documents\Arduino\libraries\`
   - **macOS:** `~/Documents/Arduino/libraries/`
   - **Linux:** `~/Arduino/libraries/`
5. Restart the Arduino IDE.

### Option 3 — Git clone

```bash
git clone https://github.com/Techgeeks-store/Spirit-N20-PCB-Library.git SpiritN20
```

Run that inside your `Arduino/libraries` folder. Note the `SpiritN20` on the end — it clones into a correctly named folder for you.

---

## Your first sketch

```cpp
#include <SpiritN20.h>

SpiritN20 Robot;

void setup() {
  Serial.begin(9600);   // you call this yourself
  Robot.begin();        // sets up every pin and the impeller timer
  Robot.setBattery(3);  // 2 or 3, to match your LiPo
}

void loop() {
  Robot.drive(150, 150);   // both motors forward
  delay(1000);
  Robot.stop();            // brake
  delay(1000);
}
```

`begin()` puts the board in a known state: motors braked, impeller off, both LEDs off, impeller timer configured. The library never calls `Serial.begin()` — that stays yours, so you pick the speed.

---

## Pin names

The library defines these for you, matching the board. Use the **names**, not the numbers.

| Name | Pin | What it is |
|---|---|---|
| `BTN1` | D12 | Button 1 |
| `BTN2` | D4 | Button 2 |
| `LED1` | D2 | Status LED 1 |
| `LED2` | D3 | Status LED 2 |
| `IMPELLER` | D11 | Impeller PWM |
| `AIN1` | D7 | Motor A (left) direction |
| `AIN2` | D6 | Motor A (left) direction |
| `PWMA` | D5 | Motor A (left) speed |
| `BIN1` | D8 | Motor B (right) direction |
| `BIN2` | D9 | Motor B (right) direction |
| `PWMB` | D10 | Motor B (right) speed |

Sensors get **A0–A7** on the front connector.

You will only ever type `BTN1`, `BTN2`, `LED1` and `LED2` yourself. The rest are listed for reference — the library drives those.

---

## The impeller

```cpp
Robot.setImpeller(40);   // 0 is off, higher is more suction
Robot.setImpeller(0);    // off
```

The impeller motor only spins **one way**, which is all a suction fan needs. Negative numbers are not a thing here; ask for one and you get a warning and zero.

### It ramps itself

Going straight to full power throws a large current spike into the 8520 coreless motor and heats it immediately. So `setImpeller()` climbs to the target one step at a time, 10 ms a step, rather than jumping. **You do not have to ramp it yourself.**

The climb starts from wherever the impeller already is, so nudging it from 40 up to 50 costs a tenth of a second rather than a full spin-up from zero.

> **`setImpeller()` waits while it climbs.** Going from nothing to 100 takes about a second, and your sketch does nothing else meanwhile. Set the impeller before you start driving, not inside a fast loop. Coming back down is instant, since slowing carries no spike to protect against.

Start at a target of **40** and raise it in steps of 5–10 across runs until you have the grip you want. N20 builds are light, around 120 g, and need less downforce than you might expect.

### Looking after the motor

A coreless motor has no iron core, which is what makes it spin up in milliseconds — and also what makes it heat up fast under sustained load.

- **Touch the motor body after each run.** Warm is fine. Too hot to keep a finger on for two seconds means back the speed off or rest longer.
- **Rest 30–60 seconds between runs.** Most important when the Y Pad is shorted and the motor is seeing full battery voltage.
- **Let it reach speed before you drive off.** Vacuum takes a moment to build. Roll off early and you have no downforce for the first corner, which is exactly where you wanted it.
- **Check the skirt is seated.** It seals the low-pressure zone. If air leaks in from the sides, the downforce drops a long way.

### Why it sounds like nothing

`analogWrite()` would drive this pin at about 490 Hz. You would hear that as a whine from the motor, and the current through the coil would keep jumping, wasting power as heat.

The library sets the timer up directly with no prescaler, giving **62,500 Hz** — far above hearing, with much smoother current. That is the one place this library talks to the chip's registers, and the reason it is Nano-only.

---

## Battery setting and the speed cap

```cpp
Robot.setBattery(2);   // 2 cell LiPo
Robot.setBattery(3);   // 3 cell LiPo
```

This is the line that protects your impeller motor. It sets a ceiling on how fast the impeller is allowed to run:

| Battery | Ceiling | Roughly what the motor sees |
|---|---|---|
| 2 cells (~8.4V) | 150 | about 5V average |
| 3 cells (~12.6V) | 100 | about 5V average |

Both land near 5V average, which is what the 8520 is happy with.

**If you never call it, the library assumes 3 cells.** That is deliberate. On a 2 cell pack you get a weaker impeller until you say otherwise, which is the right way round to be wrong — the other way round cooks the motor.

Ask for more than the ceiling and you get the ceiling, plus one line in the Serial Monitor telling you why.

### Raising the ceiling

```cpp
Robot.setImpellerMAX(180);   // override the battery ceiling
```

Sometimes you need more than the safe default. `setImpellerMAX()` sets the ceiling to whatever you ask, up to 255.

Two things to know:

- **Last call wins.** `setBattery()` sets the ceiling and so does `setImpellerMAX()`. Call `setBattery()` first, then `setImpellerMAX()` if you want to go past it. Calling them the other way round puts the battery ceiling back.
- **Going above the safe ceiling prints one warning.** Lowering it, or raising it within the safe range, stays silent. The warning is there so nobody exceeds the limit by accident and then wonders why the motor is too hot to touch.

Raising the cap changes only what the library **permits**. It does not change the voltage reaching the motor — that is your battery and the Y Pad. On a shorted Y Pad with a 3 cell pack, a cap of 200 puts roughly 9.9V average through a motor rated for 3.7V.

---

## Power: the X Pad and Y Pad

Two pairs of solder pads decide where each rail gets its power. Each rail has exactly two options and you pick **one**.

### X Pad — the sensor and logic rail

| Situation | What to do |
|---|---|
| ARC-8, or any sensor under about 300 mA | **Short the X Pad.** The Nano's 5V pin handles it. |
| ARC-16, or anything over 300 mA | **Leave it open, fit a 5V 3A BEC** on the left pads. |

### Y Pad — the impeller rail

| Situation | What to do |
|---|---|
| Battery voltage close to the motor's rating, within 3–4V | **Short the Y Pad.** Limit power with your PWM value in code. |
| Battery much higher than the motor rating | **Leave it open, fit a 5V 3A BEC** on the right pads. |

### ⚠️ Never do both on the same rail

If a BEC is fitted, the matching pad must **not** be shorted. If the pad is shorted, the BEC must **not** be fitted. Two power sources fighting on one rail can damage the BEC, the PCB traces, or both.

---

## Drive motors

```cpp
Robot.drive(speedA, speedB);
```

Each speed runs from **−255 to 255**. Motor A is the **left** motor, Motor B is the **right**.

| Call | What it does |
|---|---|
| `Robot.drive(150, 150)` | forward |
| `Robot.drive(-150, -150)` | backward |
| `Robot.drive(150, 50)` | turn right |
| `Robot.drive(50, 150)` | turn left |
| `Robot.drive(-150, 150)` | spin on the spot |
| `Robot.stop()` | brake both |

**A speed of 0 brakes.** The motor actively resists turning instead of coasting. On a suction robot with this much grip, braking is what makes a tight corner stop where you aimed it.

**`stop()` is drive motors only.** The impeller keeps running. To stop everything:

```cpp
Robot.stop();
Robot.setImpeller(0);
```

**Numbers past the limit are quietly trimmed.** `drive(1000, 1000)` runs at 255. No error, no surprise — useful when PID maths occasionally overshoots.

### If the robot drives the wrong way

Run the **MotorTest** example and watch what it does:

| What you see | Fix |
|---|---|
| Drives **backward** when it says forward | Swap the wires on **both** motors |
| **Turns** when it says forward | One motor is mounted the other way round — swap **that one only** |

Both fixes are in the wiring. No code change needed.

---

## Buttons

```cpp
if (Robot.isPressed(BTN1)) {
  // BTN1 is being held down right now
}
```

`isPressed()` returns **`true` when the button is pressed** — which sounds obvious, but the raw pin does the opposite. The buttons use `INPUT_PULLUP`, so `digitalRead()` gives 1 when released and 0 when pressed. `isPressed()` flips that round.

### Waiting for a press

```cpp
Robot.waitFor(BTN1);   // sketch pauses here until pressed
```

The classic competition startup: put the robot down, press a button when you are ready. See the **StartupSequence** example.

### Debouncing

```cpp
Robot.isPressed(BTN1)       // instant, exactly what the pin says
Robot.isPressed(BTN1, 20)   // ignore anything not steady for 20 ms
```

A push button is two pieces of metal springing together. For a few milliseconds after a press they bounce apart and back, and the Arduino sees every bounce — so **one press can look like five**.

Use the plain version for "is it held down". Use the debounced version whenever you are counting presses or toggling something. **20 ms suits most buttons.** Call it regularly, ideally every pass through `loop()`, since it works by watching how long the reading has stayed the same. Each button remembers separately.

---

## LEDs

```cpp
Robot.ledOn(LED1);
Robot.ledOff(LED1);
Robot.ledToggle(LED1);
```

Both start off after `begin()`. They are the fastest way to see what your robot is thinking while it is moving.

> **Never call `analogWrite()` on the LED2 pin.** It shares a timer with the impeller, and doing so would disturb the impeller's PWM. Turning it on and off with the functions above is completely safe — that only writes to the port, and never touches the timer.

---

## Full function reference

| Function | What it does |
|---|---|
| `SpiritN20()` | Creates the object. Put `SpiritN20 Robot;` above `setup()`. |
| `void begin()` | Sets up every pin and the impeller timer, brakes the motors, LEDs off. Call once in `setup()`. |
| `void setBattery(int cells)` | `2` or `3`. Sets the impeller ceiling. Defaults to 3 if never called. |
| `void setImpeller(int speed)` | Runs the impeller, 0 upward. Trimmed to the current ceiling. |
| `void setImpellerMAX(int maxSpeed)` | Sets the ceiling yourself, 0–255. Overrides the battery ceiling. |
| `void drive(int speedA, int speedB)` | Both motors, −255 to 255. 0 brakes. Out-of-range trimmed silently. |
| `void stop()` | Brakes both drive motors. Impeller keeps running. |
| `bool isPressed(int button, unsigned int debounceMs = 0)` | `true` while held. Pass `BTN1` or `BTN2`. |
| `void waitFor(int button)` | Pauses the sketch until that button is pressed. |
| `void ledOn(int led)` | Turns an LED on. Pass `LED1` or `LED2`. |
| `void ledOff(int led)` | Turns an LED off. |
| `void ledToggle(int led)` | Flips an LED. |

---

## Example sketches

All under **File → Examples → SpiritN20**.

| Example | What it teaches |
|---|---|
| **MotorTest** | Every direction in turn, to check your motor wiring. **Run this first.** |
| **ImpellerTest** | Ramping the impeller up and down safely, with the full safety notes. |
| **ButtonsAndLEDs** | Reading both buttons and driving both LEDs. |
| **StartupSequence** | The competition pattern: wait, set up, spin up, then drive. |

---

## Warning messages and what to do about them

**Each message prints only once per run**, so a mistake inside `loop()` cannot flood the Monitor and hide everything else.

| Message | What went wrong | How to fix it |
|---|---|---|
| `SpiritN20: call begin() first` | You used the board before setting it up. | Add `Robot.begin();` to `setup()`. |
| `SpiritN20: battery must be 2 or 3` | `setBattery()` got something else. | Only 2 and 3 cell packs are handled. The previous setting is kept. |
| `SpiritN20: impeller limited by the cap` | You asked for more than the ceiling allows. | Normal if you asked for a big number. If you want more, set the battery correctly, or raise it with `setImpellerMAX()`. |
| `SpiritN20: impeller only runs one way` | A negative speed was passed to `setImpeller()`. | The impeller cannot reverse. Use 0 upward. |
| `SpiritN20: impeller cap raised above the safe limit` | `setImpellerMAX()` went past what the battery allows. | Only a warning, not an error. Watch the motor temperature closely. |
| `SpiritN20: unknown button` | Something other than `BTN1` or `BTN2` was passed. | Use the names `BTN1` and `BTN2`. |
| `SpiritN20: unknown LED` | Something other than `LED1` or `LED2` was passed. | Use the names `LED1` and `LED2`. |

### Something is wrong but there is no message

| What you see | Almost certainly |
|---|---|
| **Nothing in the Serial Monitor** | No `Serial.begin(9600);`, or the Monitor's speed box does not match. |
| **Motors do not spin** | Battery flat, or loose motor connections. |
| **Impeller does not spin** | Check the Y Pad or right-hand BEC, and that your speed is above about 30 — very low values may not overcome the motor's starting friction. |
| **Impeller is weak** | You probably never called `setBattery(2)`, so it is capped at the 3 cell limit of 100. |
| **Impeller motor gets very hot** | Speed too high, runs too long, or too big a gap between battery voltage and the motor's rating on a shorted Y Pad. |
| **Robot slides on corners** | Skirt not sealed, impeller not up to speed yet, or too little impeller speed. |
| **Robot drives the wrong way** | See the motor wiring table above. |
| **A button fires several times** | Contact bounce. Use `isPressed(BTN1, 20)`. |

---

## Using it with a sensor array

SpiritN20 drives the robot; a sensor library reads the track. They are separate libraries and work together:

```cpp
#include <SpiritN20.h>
#include <ARC8.h>

SpiritN20 Robot;
ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

void setup() {
  Serial.begin(9600);
  Robot.begin();
  Robot.setBattery(3);

  sensors.begin(pins);
  sensors.setPID(0.03, 0.0, 0.8);   // a safe starting point

  Robot.waitFor(BTN1);            // press to calibrate
  Robot.ledOn(LED1);
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Robot.ledOff(LED1);

  Robot.waitFor(BTN2);            // press to run
  Robot.setImpeller(40);          // impeller first - it climbs on its own
  delay(1500);                    // let the vacuum build
  Robot.ledOn(LED2);
}

void loop() {
  sensors.read(values);
  int correction = sensors.getCorrection();
  Robot.drive(230 - correction, 230 + correction);
}
```

The sensor connector gives you **A0–A7**, which fits an [ARC8](https://github.com/Techgeeks-store/ARC8-Sensor-Library) directly. For an [ARC16](https://github.com/Techgeeks-store/ARC16-Sensor-Library), remember it draws about 310 mA, so leave the X Pad open and fit a BEC.

---

## Things to know

- **All the pins are fixed.** They are etched into the PCB, so there is nothing to configure.
- **Nano only.** The impeller needs this chip's Timer2 and its D11 = OC2A mapping. The library will not compile for anything else, on purpose.
- **The three PWM outputs are on three different timers.** Motor A on Timer0, Motor B on Timer1, impeller on Timer2. They cannot interfere with each other.
- **`analogWrite()` on the motor pins does not break `millis()` or `delay()`.** Timer0 is shared with those, but `analogWrite()` only changes the duty register, never the timer's mode or prescaler.
- **`stop()` does not stop the impeller.** That is on purpose — braking for a corner should not cost you your downforce.
- **The default battery setting is 3 cells**, the safer guess. Set it correctly for a 2 cell pack or your impeller will be weaker than it should be.
- **`BTN1`, `BTN2`, `LED1`, `LED2` and the motor pin names are global.** If another library defines any of the same names, you will get a "redefined" warning.
- **No memory tricks.** No `malloc`, no `new`, no `String`. Everything is fixed size, so it cannot fragment your RAM mid-race.
- **`setImpeller()` is the only thing that waits.** It holds your sketch while the impeller climbs, about 10 ms per step. Nothing else in the library blocks: the debounce uses `millis()`, and `waitFor()` spins on the pin by design.

### Memory use

Measured on an Arduino Nano, compiled with the real Arduino AVR core:

| Example | Flash | RAM |
|---|---|---|
| MotorTest | 3,568 bytes (10.9%) | 354 bytes (17.3%) |
| ButtonsAndLEDs | 3,770 bytes (11.5%) | 292 bytes (14.3%) |
| StartupSequence | 4,376 bytes (13.4%) | 332 bytes (16.2%) |
| ImpellerTest | 4,422 bytes (13.5%) | 313 bytes (15.3%) |

The library itself is about 1.7 KB of flash, leaving plenty for a sensor library and your own line following code.

---

## License

MIT — see [LICENSE](LICENSE). Use it, change it, build products with it. Just keep the copyright notice.

Copyright © 2026 TECHGEEKS
