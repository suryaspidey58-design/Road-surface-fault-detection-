#pragma once
// ---------------------------------------------------------------
// Pin map and tuning constants for the road surface analyzer
// Board: ESP32 DevKit
// ---------------------------------------------------------------

// ---- LCD 16x2 (RS, EN, D4, D5, D6, D7) ----
#define LCD_RS 13
#define LCD_EN 12
#define LCD_D4 19
#define LCD_D5 18
#define LCD_D6 5
#define LCD_D7 4

// ---- GPS (UART2) ----
#define GPS_RX 16
#define GPS_TX 17

// ---- Road-facing ultrasonic sensor ----
#define TRIG_PIN 27
#define ECHO_PIN 25

// ---- Obstacle (front) ultrasonic sensor ----
#define TRIG_OBS 23
#define ECHO_OBS 34   // GPIO34 is input-only: fine for an echo pin

// ---- Motor driver inputs (L293D / L298N) ----
#define MOTOR_IN1 32
#define MOTOR_IN2 33
#define MOTOR_IN3 26
#define MOTOR_IN4 14

// ---- Road classification thresholds ----
// Road-facing ultrasonic distance (cm)
#define DIST_HUMP_MAX_CM       5    // <= 5  -> Hump
#define DIST_PLAIN_MAX_CM      10   // <= 10 -> Plain, above -> Pothole

// Vibration magnitude sqrt(x^2+y^2+z^2) in m/s^2
#define VIB_SMOOTH_MAX         12   // <12 smooth
#define VIB_SLIGHT_MAX         15   // <15 slightly rough
#define VIB_ROUGH_MAX          18   // <18 rough, otherwise very rough

// Obstacle stop distance (cm)
#define OBSTACLE_STOP_CM       10
