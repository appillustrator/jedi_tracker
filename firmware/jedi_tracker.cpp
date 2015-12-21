#include "application.h"
#include "SparkFunMicroOLED.h"
#include "TinyGPS++.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

void updateLocation();
void updateScreen();

const int OLED_PIN_RESET = A0;
const int OLED_PIN_DC = A1;
const int OLED_PIN_CS = A2;
MicroOLED screen(MODE_SPI, OLED_PIN_RESET, OLED_PIN_DC, OLED_PIN_CS);

TinyGPSPlus gps;

void setup() {
  screen.begin();     // Initialize the OLED
  screen.clear(ALL);  // Clear the library's display buffer
  screen.display();   // Display what's in the buffer (splashscreen)

  Serial1.begin(9600);
  Particle.connect();
}

const double TARGET_LAT = 42.374499;
const double TARGET_LONG = -83.542796;

void loop()
{
  updateLocation();
  updateScreen();
}

void updateLocation() {
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }

  if(gps.location.isValid()) {
    double distanceToTarget = TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      TARGET_LAT, 
      TARGET_LONG);
    (void)distanceToTarget;
  }
}

void updateScreen() {
  static int lastUpdate = 0;
  static int num = 0;
  if(millis() - lastUpdate > 20) {
    screen.clear(PAGE);
    screen.setFontType(FONT_STAR_WARS);
    screen.setCursor(0, 32);
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
  num++;
  if(num >= 10000) {
    num = 0;
  }
}
