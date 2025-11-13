#include <Wire.h>
#include <Adafruit_SSD1306.h>

// --- OLED Setup ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Pins ---
#define POT_PIN A0
#define BUTTON_PIN 12

// --- Characters ---
const char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const int numChars = sizeof(charSet) - 1; // exclude null terminator
const int BACKSPACE_INDEX = numChars;     // last section of potentiometer = backspace

// --- Variables ---
String committedText = "";
char previewChar = ' ';

// --- Button debounce ---
bool buttonState = HIGH;            // current reading
bool lastButtonState = HIGH;        // previous reading
unsigned long lastDebounceTime = 0; // last state change time
const unsigned long debounceDelay = 50; // ms

// --- Blinking cursor ---
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500; // ms
bool cursorVisible = true;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Start Typing");
  display.display();
  delay(1000);
}

void loop() {
  // --- Read potentiometer ---
  int potValue = analogRead(POT_PIN); // 0-1023
  int mappedIndex = map(potValue, 0, 1023, 0, numChars); // last section = backspace

  if (mappedIndex >= numChars) {
    previewChar = '<'; // display backspace symbol
  } else {
    previewChar = charSet[mappedIndex];
  }

  // --- Button handling with debounce ---
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // reset debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Button pressed (active LOW)
      if (buttonState == LOW) {
        if (mappedIndex >= numChars) {
          // Backspace
          if (committedText.length() > 0) {
            committedText.remove(committedText.length() - 1);
          }
        } else {
          // Commit character
          committedText += previewChar;
        }
      }
    }
  }

  lastButtonState = reading;

  // --- Update blinking cursor ---
  if (millis() - lastBlinkTime >= blinkInterval) {
    cursorVisible = !cursorVisible;
    lastBlinkTime = millis();
  }

  // --- Update display ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Enter Plate:");

  // Committed text
  display.setCursor(0, 25);
  display.print(committedText);

  // Preview character with blinking cursor
  display.setCursor(8 * committedText.length(), 25);
  if (cursorVisible) {
    display.print(previewChar);
  }

  display.display();
  delay(20); // small delay for smooth display updates
}
