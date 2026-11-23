# Pin Mapping (ESP32 DevKit)

Taken from the firmware (`firmware/road_surface_analyzer/config.h`).

| Module | Signal | ESP32 GPIO |
|---|---|---|
| ADXL345 accelerometer | SDA / SCL | 21 / 22 (default I2C) |
| NEO-6M GPS | GPS TX -> ESP32 RX / GPS RX <- ESP32 TX | 16 / 17 (UART2) |
| Road ultrasonic | TRIG / ECHO | 27 / 25 |
| Obstacle ultrasonic | TRIG / ECHO | 23 / 34 |
| Motor driver (L293D/L298N) | IN1 / IN2 / IN3 / IN4 | 32 / 33 / 26 / 14 |
| LCD 16x2 | RS / EN / D4 / D5 / D6 / D7 | 13 / 12 / 19 / 18 / 5 / 4 |

## Notes
- ESP32 logic is 3.3 V. HC-SR04 echo pins output 5 V, so use a voltage divider or level shifter.
- GPIO34 is input-only, which is fine for an echo pin.
- Power: bridge rectifier + 7805 (5 V) + 1117-3.3 V regulator, as in the circuit diagram (`docs/images/circuit-diagram.jpeg`).

## Cloud commands (`robot` variable)
| Char | Action |
|---|---|
| A | Forward |
| B | Reverse |
| C | Left |
| D | Right |
| E | Stop |

Motors stop automatically when the obstacle sensor reads <= 10 cm.

## Road Surface Index (RSI)
`RSI = vibrationScore + ultrasonicScore` (capped at 100)

| Vibration (m/s²) | Score | Ultrasonic (cm) | Type | Score |
|---|---|---|---|---|
| < 12 | 0 | <= 5 | Hump | 20 |
| 12-15 | 20 | 5-10 | Plain | 0 |
| 15-18 | 40 | > 10 | Pothole | 30 |
| >= 18 | 70 | | | |

RSI < 40 = GOOD ROAD, < 70 = MODERATE ROAD, otherwise BAD ROAD.
