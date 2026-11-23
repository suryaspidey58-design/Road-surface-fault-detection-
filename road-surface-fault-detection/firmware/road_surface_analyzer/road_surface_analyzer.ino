/*
 * IoT Platform for Road Surface Analysis using MEMS data
 * ESP32 + ADXL345 + 2x ultrasonic + NEO-6M GPS + L293D/L298N + Arduino IoT Cloud
 *
 * Author : Suriya Chakravarthy R
 * Project: M.A.M College of Engineering and Technology, ECE (2026)
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <TinyGPS++.h>
#include <LiquidCrystal.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include "config.h"
#include "motors.h"
#include "secrets.h"      // copy secrets.example.h -> secrets.h

/**************** PERIPHERALS ****************/
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

/**************** VARIABLES ****************/
float distanceCM;
float obstacleDistance;
float vibration;
int   RSI;                       // Road Surface Index (0-100, higher = worse)
char  roadType   = 'G';          // H = hump, G = plain, P = pothole
String smoothness = "SMT";       // SMT / SLT / RCH / V-R
double lat = 0, lng = 0;

/******** CLOUD VARIABLES ********/
String aDXL_Ultra;
String roadStatus;
String obstatus;
float  rsivalue;
CloudLocation mapi;
String robot;                    // command character from dashboard (A-E)

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(aDXL_Ultra, READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(roadStatus, READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(rsivalue,   READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(mapi,       READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(robot,      READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(obstatus,   READWRITE, ON_CHANGE, NULL);
}

/**************** ULTRASONIC ****************/
float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 50000);   // 50 ms timeout
  return duration * 0.034 / 2;                   // cm
}

/**************** SETUP ****************/
void setup() {
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.print("Road Analyzer");

  if (!accel.begin()) {
    Serial.println("ADXL345 not detected!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_OBS, OUTPUT);
  pinMode(ECHO_OBS, INPUT);

  motorsInit();
  stopMotors();

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}

/**************** LOOP ****************/
void loop() {
  ArduinoCloud.update();
  Serial.println(robot);   // debug: last command from dashboard

  /* ---- GPS ---- */
  while (gpsSerial.available())
    gps.encode(gpsSerial.read());

  if (gps.location.isUpdated()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
    mapi = {lat, lng};
  } else {
    mapi = {0, 0};
  }

  /* ---- ADXL345 vibration magnitude ---- */
  sensors_event_t event;
  accel.getEvent(&event);
  vibration = sqrt(
      event.acceleration.x * event.acceleration.x +
      event.acceleration.y * event.acceleration.y +
      event.acceleration.z * event.acceleration.z);

  /* ---- Ultrasonic distances ---- */
  distanceCM       = getDistance(TRIG_PIN, ECHO_PIN);
  obstacleDistance = getDistance(TRIG_OBS, ECHO_OBS);

  /******** ROAD CLASSIFICATION ********/
  int ultrasonicScore = 0;
  if (distanceCM <= DIST_HUMP_MAX_CM)       { roadType = 'H'; ultrasonicScore = 20; }
  else if (distanceCM <= DIST_PLAIN_MAX_CM) { roadType = 'G'; ultrasonicScore = 0;  }
  else                                      { roadType = 'P'; ultrasonicScore = 30; }

  int vibrationScore;
  if (vibration < VIB_SMOOTH_MAX)      { smoothness = "SMT"; vibrationScore = 0;  }
  else if (vibration < VIB_SLIGHT_MAX) { smoothness = "SLT"; vibrationScore = 20; }
  else if (vibration < VIB_ROUGH_MAX)  { smoothness = "RCH"; vibrationScore = 40; }
  else                                 { smoothness = "V-R"; vibrationScore = 70; }

  RSI = vibrationScore + ultrasonicScore;
  if (RSI > 100) RSI = 100;
  rsivalue = RSI;

  String roadText = (roadType == 'H') ? "Hump" : (roadType == 'P') ? "Pothole" : "Plain";
  roadStatus = (RSI < 40) ? "GOOD ROAD" : (RSI < 70) ? "MODERATE ROAD" : "BAD ROAD";

  /************* ROBOT CONTROL *************/
  if (obstacleDistance <= OBSTACLE_STOP_CM) {
    obstatus = "OBSTACLE";
    stopMotors();
  } else {
    obstatus = "CLEAR";
    if      (robot == "A") forward();
    else if (robot == "B") reverse();
    else if (robot == "C") left();
    else if (robot == "D") right();
    else if (robot == "E") stopMotors();
  }

  /* ---- Cloud status string ---- */
  aDXL_Ultra =
      "ADXL:" + String(vibration, 1) +
      "|ULTRA:" + String(distanceCM, 1) + "|" + roadText +
      "|OBS:" + String(obstacleDistance, 1) + "|" + obstatus;
}
