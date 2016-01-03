#include "application.h"
#include "SparkFunMicroOLED.h"
#include "TinyGPS++.h"
#include "GeocacheFetcher.h"

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

typedef void (*LoopHandler_t)(void);
LoopHandler_t loopHandler = NULL;
unsigned long appTime = 0;

/* This symbol is provided by environment.cpp. If this file is not present,
 * don't throw a compiler error, just ignore it */
extern char *opencachingApiKey __attribute__((weak));

void changeState(LoopHandler_t newHandler, unsigned int delay = 0);
void initializeDisplay();
void connectToNetwork();
void waitForConnection();
void showConnection();
void getCurrentLocation();
void waitForLocation();
void getTargetLocation();
void waitForTarget();
void announceTracking();
void trackTarget();
void updateScreen();
void displayMessage(const char *line1, const char *line2, const char *line3);

const int OLED_PIN_RESET = A0;
const int OLED_PIN_DC = A1;
const int OLED_PIN_CS = A2;
MicroOLED screen(MODE_SPI, OLED_PIN_RESET, OLED_PIN_DC, OLED_PIN_CS);

TinyGPSPlus gps;
GeocacheFetcher fetcher(opencachingApiKey);

Point current;
Point target;

double distanceToTarget = 9999;
const char *cardinalToTarget = "";

enum {
  ADDR_TARGET = 0,
  ADDR_CURRENT = ADDR_TARGET + sizeof(target),
};

STARTUP(initializeDisplay());

void changeState(LoopHandler_t newHandler, unsigned int delay) {
  loopHandler = newHandler;
  appTime = millis() + delay * 1000;
}

bool stateTimeout(unsigned int seconds) {
  return millis() - appTime > seconds * 1000;
}

bool stateDelayed() {
  return ((long)appTime - (long)millis()) > 0;
}

void setup() {
  Serial1.begin(9600);
  loopHandler = connectToNetwork;
}

void loop() {
  if(stateDelayed()) {
    return;
  }

  if(loopHandler) {
    loopHandler();
  }
}

void initializeDisplay() {
  screen.begin();     // Initialize the OLED
  screen.clear(ALL);  // Clear the library's display buffer
  screen.display();   // Display what's in the buffer (splashscreen)
  delay(1000);
}

void connectToNetwork() {
  screen.clear(PAGE);
  screen.setFontType(FONT_8x16);
  displayMessage("Access", "Imperial", "Network");

  Serial.println("Connecting to network");
  Particle.connect();
  changeState(waitForConnection);
}

void waitForConnection() {
  if(Particle.connected()) {
    Serial.println("Connected to network");

    displayMessage("Secure", "Network", "Access");
    changeState(getCurrentLocation, 2);

  } else if(stateTimeout(20)) {
    Serial.println("Timeout while waiting for network");

    displayMessage("No", "Network", "Access");
    changeState(getCurrentLocation, 2);
  }
}

void getCurrentLocation() {
  displayMessage("Galaxy", "Position", "System");
  changeState(waitForLocation, 2);
}

void waitForLocation() {
  if(gps.location.isValid()) {
    current.latitude = gps.location.lat();
    current.longitude = gps.location.lng();
    EEPROM.put(ADDR_CURRENT, current);

    Serial.println("Got current location from GPS " +
        String(current.latitude) + "," + String(current.longitude));

    displayMessage("GPS", "Position", "Found");
    changeState(getTargetLocation, 2);
  } else if(stateTimeout(60)) {
    EEPROM.get(ADDR_CURRENT, current);

    Serial.println("Timeout while waiting for GPS");
    Serial.println("Using previous location " +
        String(current.latitude) + "," + String(current.longitude));

    displayMessage("No", "GPS", "Position");
    changeState(getTargetLocation, 2);
  }
}

void getTargetLocation() {
  displayMessage("Search", "for Jedi", "Hideout");
  changeState(waitForTarget, 2);
}

void waitForTarget() {
  if(Particle.connected()) {
    Serial.println("Getting target location from OpenCaching");
    // This function blocks :-(
    target = fetcher.searchNearest("jedi", current.latitude, current.longitude);
  }

  if(target.isValid()) {
    EEPROM.put(ADDR_TARGET, target);

    Serial.println("Got target location " +
        String(target.latitude) + "," + String(target.longitude));
  } else {
    EEPROM.get(ADDR_TARGET, target);

    Serial.println("Using previous target location " +
        String(target.latitude) + "," + String(target.longitude));
  }

  displayMessage("Jedi", "Hideout", "Found");
  changeState(announceTracking);
}

void announceTracking() {
  displayMessage("Tracking", "Distance", "to Jedi");
  changeState(trackTarget, 2);
}

void trackTarget() {
  if(gps.location.isValid()) {
    distanceToTarget = TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      target.latitude, 
      target.longitude);
    double courseToTarget = TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      target.latitude, 
      target.longitude);
    cardinalToTarget = TinyGPSPlus::cardinal(courseToTarget);

    static int lastUpdate = 0;
    if(millis() - lastUpdate > 1000) {
      lastUpdate = millis();
      Serial.println(
        String(gps.location.lat()) + "," +
        String(gps.location.lng()) + " -> " +
        String(target.latitude) + "," +
        String(target.longitude) + "=" +
        String(distanceToTarget) + " (" +
        String(cardinalToTarget) + ")"
      );
    }

    updateScreen();
  }
}

void updateScreen() {
  static int lastUpdate = 0;
  if(millis() - lastUpdate > 20) {
    screen.clear(PAGE);
    screen.setFontType(FONT_8x16);
    screen.setCursor((8 - strlen(cardinalToTarget)) * 4, 0);
    screen.print(cardinalToTarget);

    screen.setFontType(FONT_STAR_WARS);
    screen.setCursor(0, 32);
    int num = distanceToTarget > 9999 ? 9999 : (int)distanceToTarget;
    String numStr;
    if(num < 10) {
      numStr = "000";
    } else if(num < 100) {
      numStr = "00";
    } else if(num < 1000) {
      numStr = "0";
    }
    numStr += String(num);

    screen.print(numStr);
    screen.display();

    lastUpdate = millis();
  }
}

void displayMessage(const char *line1, const char *line2, const char *line3) {
  screen.clear(PAGE);
  screen.setFontType(FONT_8x16);
  screen.setCursor((8 - strlen(line1)) * 4, 0);
  screen.print(line1);
  screen.setCursor((8 - strlen(line2)) * 4, 16);
  screen.print(line2);
  screen.setCursor((8 - strlen(line3)) * 4, 32);
  screen.print(line3);
  screen.display();
}

// Read characters from the GPS
void serialEvent1()
{
  while(Serial1.available()) {
    char c = Serial1.read();
    gps.encode(c);
  }
}
