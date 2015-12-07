#include "application.h"
#include "SparkFunMicroOLED.h"

SYSTEM_MODE(SEMI_AUTOMATIC);

const int OLED_PIN_RESET = A0;
const int OLED_PIN_DC = A1;
const int OLED_PIN_CS = A2;
MicroOLED oled(MODE_SPI, OLED_PIN_RESET, OLED_PIN_DC, OLED_PIN_CS);

void setup() {
  oled.begin();     // Initialize the OLED
  oled.clear(ALL);  // Clear the library's display buffer
  oled.display();   // Display what's in the buffer (splashscreen)
  Particle.connect();
}
