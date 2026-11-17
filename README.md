# Posture Reminder Device

A wearable that vibrates when the wearer slouches. An ESP32 reads orientation from a BNO055 inertial measurement unit (IMU) worn on the back, compares it with a calibrated upright position, and switches on a small vibration motor when the wearer has been out of position for more than two seconds.

I designed, wired, and tested this device for Junior Design (EECE 4991) at the University of Memphis in Spring 2026. The firmware was written with the help of an AI coding assistant. I flashed it, calibrated it, and tested it on the hardware.

## Hardware

| Part | Detail |
| --- | --- |
| Microcontroller | ESP32 DevKit V4 |
| Sensor | BNO055 9-axis IMU, I2C address 0x28 |
| Actuator | N20 DC vibration motor |
| Power | USB power bank |

## Wiring

| From | To |
| --- | --- |
| BNO055 VIN | ESP32 3V3 |
| BNO055 GND | ESP32 GND |
| BNO055 SDA | ESP32 GPIO21 |
| BNO055 SCL | ESP32 GPIO22 |
| Motor wire 1 | ESP32 GPIO4 |
| Motor wire 2 | ESP32 GND |

## How it works

1. **Startup.** The ESP32 starts I2C communication and checks that the sensor responds. If the sensor is missing, the program stops and prints an error.
2. **Calibration.** The motor buzzes three times, then the wearer has five seconds to stand up straight. The ESP32 reads the sensor's Euler angles once and saves the x and y values as the baseline. One long buzz confirms calibration.
3. **Monitoring.** Ten times per second, the ESP32 reads the current angles and calculates how far each one is from the baseline. The angleDiff function handles the wrap-around at 360 degrees, so 359 degrees and 1 degree count as 2 degrees apart. The larger of the two differences is the deviation.
4. **Decision.** If the deviation is above 15 degrees, a timer starts. If it stays above 15 degrees for more than 2 seconds, the motor turns on. The delay prevents a buzz every time the wearer bends briefly.
5. **Reset.** As soon as the deviation drops back under 15 degrees, the timer clears and the motor turns off.

This is a simple closed feedback loop: a sensor measures, the controller compares the measurement with a setpoint, and an actuator responds.

## Settings

| Constant | Value | Meaning |
| --- | --- | --- |
| SLOUCH_THRESHOLD | 15.0 | Degrees away from the baseline that count as bad posture |
| BAD_POSTURE_DELAY | 2000 | Milliseconds of bad posture before the motor turns on |
| MOTOR_PIN | 4 | GPIO pin that drives the motor |

## Known limitations

- The x Euler angle from the BNO055 is compass heading. Turning to face a different direction changes it, so turning around can trigger the motor even with good posture. Using only the tilt angles would fix this.
- The baseline is captured once at power-up. Recalibrating requires resetting the board.
- The motor is driven directly from a GPIO pin. A transistor driver would be the safer design for the pin's current limit.
- The loop uses a blocking 100 ms delay, which is fine here but would not scale to a system doing other work.

## Running it

Open posture_reminder.ino in the Arduino IDE, install the Adafruit BNO055 and Adafruit Unified Sensor libraries, select the ESP32 Dev Module board, and upload. Open the serial monitor at 115200 baud to watch the deviation readings.
