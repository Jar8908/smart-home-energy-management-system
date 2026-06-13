#define BLYNK_TEMPLATE_ID "TMPL5Wx4VsSqF"
#define BLYNK_TEMPLATE_NAME "SmartHome"
#define BLYNK_AUTH_TOKEN "_PI1EXBFLcnIFplLRvlyRrS3crAW99OW"

#define BLYNK_PRINT Serial

#include <SPI.h>
#include <WiFiS3.h>
#include <BlynkSimpleWifi.h>
#include <Servo.h>
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>

char ssid[] = "TALKTALK9992AC";
char pass[] = "GCQ6JENT";

RTC_DS3231 rtc;
bool rtcOK = false;

Servo doorServo;
BlynkTimer timer;

const int lightPin = 3;
const int fanPin = 4;
const int doorPin = 10;
const int pirPin = 2;

// ---------- ENERGY MONITORING ADDED ----------
const float lightPower = 5.0;
const float fanPower = 10.0;

float todayWh = 0.0;
float savedWh = 0.0;
float maxPossibleWh = 0.0;
float savedPercent = 100.0;

unsigned long lastEnergyTime = 0;
// --------------------------------------------

int lightState = 0;
int fanState = 0;
int doorState = 0;

int autoLightOnTime = -1;   
int allOffTime = -1;        
int reminderTime = -1;      
int lightAlertMinutes = 1;  
int fanAutoOffMinutes = 1;  

int lastLightRunKey = -1;
int lastOffRunKey = -1;
int lastReminderRunKey = -1;

unsigned long lightOnTime = 0;
unsigned long fanOnTime = 0;

bool lightAutoOffActive = false;
bool fanAutoOffActive = false;
bool lightLongAlertSent = false;

unsigned long lightAlertTimeMs = 60000UL;
unsigned long lightAutoOffTimeMs = 360000UL;
unsigned long fanAutoOffTimeMs = 60000UL;

// EEPROM
const int EEPROM_MAGIC_ADDR = 0;
const int EEPROM_DATA_ADDR = 10;
const int EEPROM_MAGIC = 2468;

struct Settings {
  int autoLightOnTime;
  int allOffTime;
  int reminderTime;
  int lightAlertMinutes;
  int fanAutoOffMinutes;
};

// ---------- SAFE NOTIFICATION QUEUE ----------

struct NotificationItem {
  bool pending;
  char eventCode[32];
  char message[120];
};

NotificationItem queueList[6];

unsigned long lastNotificationTime = 0;
const unsigned long notificationGapMs = 65000UL;

void addNotification(const char* code, const char* msg) {
  for (int i = 0; i < 6; i++) {
    if (queueList[i].pending && strcmp(queueList[i].eventCode, code) == 0) {
      return;
    }
  }

  for (int i = 0; i < 6; i++) {
    if (!queueList[i].pending) {
      queueList[i].pending = true;

      strncpy(queueList[i].eventCode, code, sizeof(queueList[i].eventCode) - 1);
      strncpy(queueList[i].message, msg, sizeof(queueList[i].message) - 1);

      queueList[i].eventCode[sizeof(queueList[i].eventCode) - 1] = '\0';
      queueList[i].message[sizeof(queueList[i].message) - 1] = '\0';

      Serial.print("NOTIFICATION ADDED: ");
      Serial.println(code);
      return;
    }
  }
}

void processNotificationQueue() {
  if (!Blynk.connected()) {
    return;
  }

  if (lastNotificationTime != 0 && millis() - lastNotificationTime < notificationGapMs) {
    return;
  }

  for (int i = 0; i < 6; i++) {
    if (queueList[i].pending) {
      Blynk.logEvent(queueList[i].eventCode, queueList[i].message);
      Blynk.run();

      Serial.print("NOTIFICATION SENT: ");
      Serial.println(queueList[i].eventCode);

      queueList[i].pending = false;
      lastNotificationTime = millis();
      return;
    }
  }
}

// ---------- TIME ----------

bool isValidHHMM(int value) {
  if (value < 0) return false;
  int h = value / 100;
  int m = value % 100;
  return (h >= 0 && h <= 23 && m >= 0 && m <= 59);
}

int nowHHMM(DateTime now) {
  return now.hour() * 100 + now.minute();
}

int runKey(DateTime now) {
  return now.day() * 10000 + nowHHMM(now);
}

// ---------- SETTINGS ----------

void saveSettings() {
  Settings s;
  s.autoLightOnTime = autoLightOnTime;
  s.allOffTime = allOffTime;
  s.reminderTime = reminderTime;
  s.lightAlertMinutes = lightAlertMinutes;
  s.fanAutoOffMinutes = fanAutoOffMinutes;

  EEPROM.put(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
  EEPROM.put(EEPROM_DATA_ADDR, s);
}

void loadSettings() {
  int magic;
  EEPROM.get(EEPROM_MAGIC_ADDR, magic);

  if (magic == EEPROM_MAGIC) {
    Settings s;
    EEPROM.get(EEPROM_DATA_ADDR, s);

    if (isValidHHMM(s.autoLightOnTime)) autoLightOnTime = s.autoLightOnTime;
    if (isValidHHMM(s.allOffTime)) allOffTime = s.allOffTime;
    if (isValidHHMM(s.reminderTime)) reminderTime = s.reminderTime;
    if (s.lightAlertMinutes >= 1 && s.lightAlertMinutes <= 60) lightAlertMinutes = s.lightAlertMinutes;
    if (s.fanAutoOffMinutes >= 1 && s.fanAutoOffMinutes <= 60) fanAutoOffMinutes = s.fanAutoOffMinutes;
  }
}

void updateDurations() {
  lightAlertTimeMs = (unsigned long)lightAlertMinutes * 60000UL;
  lightAutoOffTimeMs = (unsigned long)(lightAlertMinutes + 5) * 60000UL;
  fanAutoOffTimeMs = (unsigned long)fanAutoOffMinutes * 60000UL;
}

// ---------- ENERGY FUNCTIONS ADDED ----------

void sendEnergy() {
  if (Blynk.connected()) {
    Blynk.virtualWrite(V4, todayWh);
    Blynk.virtualWrite(V5, savedPercent);
  }
}

void calculateEnergy() {
  unsigned long currentTime = millis();

  float hours = (currentTime - lastEnergyTime) / 3600000.0;
  lastEnergyTime = currentTime;

  float currentPower = 0.0;

  if (lightState == 1) {
    currentPower += lightPower;
  }

  if (fanState == 1) {
    currentPower += fanPower;
  }

  todayWh += currentPower * hours;

  maxPossibleWh += (lightPower + fanPower) * hours;

  savedWh = maxPossibleWh - todayWh;

  if (savedWh < 0) {
    savedWh = 0;
  }

  if (maxPossibleWh > 0) {
    savedPercent = (savedWh / maxPossibleWh) * 100.0;
  } else {
    savedPercent = 100.0;
  }

  if (savedPercent < 0) savedPercent = 0;
  if (savedPercent > 100) savedPercent = 100;

  sendEnergy();
}

// ---------- DEVICE CONTROL ----------

void turnLightOnAuto() {
  lightState = 1;
  digitalWrite(lightPin, HIGH);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, 1);
  }

  lightOnTime = millis();
  lightAutoOffActive = true;
  lightLongAlertSent = false;
}

void allDevicesOff() {
  lightState = 0;
  fanState = 0;
  doorState = 0;

  digitalWrite(lightPin, LOW);
  digitalWrite(fanPin, HIGH);
  doorServo.write(0);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
  }

  lightAutoOffActive = false;
  fanAutoOffActive = false;
  lightLongAlertSent = false;
}

// ---------- AUTOMATION ----------

void checkAutomation() {
  if (!rtcOK) return;

  DateTime now = rtc.now();
  int currentTime = nowHHMM(now);
  int key = runKey(now);

  if (isValidHHMM(autoLightOnTime) &&
      currentTime == autoLightOnTime &&
      lastLightRunKey != key) {

    turnLightOnAuto();
    lastLightRunKey = key;
  }

  if (isValidHHMM(reminderTime) &&
      currentTime == reminderTime &&
      lastReminderRunKey != key) {

    addNotification("night_reminder", "Reminder: Please check if any device is still ON.");
    lastReminderRunKey = key;
  }

  if (isValidHHMM(allOffTime) &&
      currentTime == allOffTime &&
      lastOffRunKey != key) {

    allDevicesOff();
    addNotification("all_devices_off", "All devices switched OFF automatically.");
    lastOffRunKey = key;
  }
}

void checkLightAlertAndAutoOff() {
  if (lightState == 1 && lightAutoOffActive) {
    unsigned long elapsed = millis() - lightOnTime;

    if (elapsed >= lightAlertTimeMs && !lightLongAlertSent) {
      addNotification("light_on_long", "Alert: Light has been ON too long.");
      lightLongAlertSent = true;
    }

    if (elapsed >= lightAutoOffTimeMs) {
      lightState = 0;
      digitalWrite(lightPin, LOW);

      if (Blynk.connected()) {
        Blynk.virtualWrite(V0, 0);
      }

      lightAutoOffActive = false;
    }
  }
}

void checkFanAutoOff() {
  if (fanState == 1 && fanAutoOffActive) {
    if (millis() - fanOnTime >= fanAutoOffTimeMs) {
      fanState = 0;
      digitalWrite(fanPin, HIGH);

      if (Blynk.connected()) {
        Blynk.virtualWrite(V1, 0);
      }

      fanAutoOffActive = false;
    }
  }
}

// ---------- BLYNK ----------

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V8);
  Blynk.syncVirtual(V9);
  Blynk.syncVirtual(V10);

  sendEnergy();
}

BLYNK_WRITE(V0) {
  lightState = param.asInt();
  digitalWrite(lightPin, lightState ? HIGH : LOW);

  if (lightState == 1) {
    lightOnTime = millis();
    lightAutoOffActive = true;
    lightLongAlertSent = false;
  } else {
    lightAutoOffActive = false;
    lightLongAlertSent = false;
  }
}

BLYNK_WRITE(V1) {
  fanState = param.asInt();
  digitalWrite(fanPin, fanState ? LOW : HIGH);

  if (fanState == 1) {
    fanOnTime = millis();
    fanAutoOffActive = true;
  } else {
    fanAutoOffActive = false;
  }
}

BLYNK_WRITE(V2) {
  doorState = param.asInt();
  doorServo.write(doorState ? 90 : 0);
}

BLYNK_WRITE(V6) {
  int value = param.asInt();
  if (isValidHHMM(value)) {
    autoLightOnTime = value;
    lastLightRunKey = -1;
    saveSettings();
  }
}

BLYNK_WRITE(V7) {
  int value = param.asInt();
  if (isValidHHMM(value)) {
    allOffTime = value;
    lastOffRunKey = -1;
    saveSettings();
  }
}

BLYNK_WRITE(V8) {
  int value = param.asInt();
  if (isValidHHMM(value)) {
    reminderTime = value;
    lastReminderRunKey = -1;
    saveSettings();
  }
}

BLYNK_WRITE(V9) {
  int value = param.asInt();
  if (value >= 1 && value <= 60) {
    lightAlertMinutes = value;
    updateDurations();
    saveSettings();
  }
}

BLYNK_WRITE(V10) {
  int value = param.asInt();
  if (value >= 1 && value <= 60) {
    fanAutoOffMinutes = value;
    updateDurations();
    saveSettings();
  }
}

void sendMotion() {
  if (Blynk.connected()) {
    Blynk.virtualWrite(V3, digitalRead(pirPin));
  }
}

// ---------- SETUP + LOOP ----------

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(lightPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(pirPin, INPUT);

  digitalWrite(lightPin, LOW);
  digitalWrite(fanPin, HIGH);

  doorServo.attach(doorPin);
  doorServo.write(0);

  Wire.begin();

  if (rtc.begin()) {
    rtcOK = true;

    if (rtc.lostPower()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    rtcOK = false;
  }

  loadSettings();
  updateDurations();

  lastEnergyTime = millis();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, checkAutomation);
  timer.setInterval(1000L, checkLightAlertAndAutoOff);
  timer.setInterval(1000L, checkFanAutoOff);
  timer.setInterval(1000L, processNotificationQueue);
  timer.setInterval(1000L, sendMotion);

  // ENERGY TIMER ADDED
  timer.setInterval(3000L, calculateEnergy);
}

void loop() {
  Blynk.run();
  timer.run();
}
