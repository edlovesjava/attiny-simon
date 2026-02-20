#include <Arduino.h>

// ATtiny85 pin allocation (5 usable I/O pins):
//   PB0–PB3: shared LED + button (time-multiplexed)
//   PB4:     buzzer
//
// Each PB0–PB3 pin is wired as:
//   PBx ──┬──[220R]──LED──GND
//          └──[button]──GND
//
// LED phase:  pinMode OUTPUT, digitalWrite HIGH/LOW
// Button phase: pinMode INPUT_PULLUP, digitalRead

namespace {
constexpr uint8_t BUTTON_COUNT = 4;
constexpr uint8_t MAX_SEQUENCE = 32;

constexpr uint8_t PINS[BUTTON_COUNT] = {PB0, PB1, PB2, PB3};
constexpr uint8_t BUZZER_PIN = PB4;

constexpr uint16_t TONE_FREQS[BUTTON_COUNT] = {262, 330, 392, 523};

constexpr uint16_t STEP_ON_MS = 250;
constexpr uint16_t STEP_GAP_MS = 120;
constexpr uint16_t INPUT_TIMEOUT_MS = 4000;
constexpr uint8_t DEBOUNCE_SAMPLES = 3;

uint8_t sequenceBuffer[MAX_SEQUENCE] = {};
uint8_t sequenceLength = 0;

uint32_t lcgState = 0xA5A5A5A5UL;

uint8_t pseudoRandom4() {
  lcgState = (1103515245UL * lcgState + 12345UL);
  return static_cast<uint8_t>((lcgState >> 16) & 0x03);
}

// --- Pin mode switching ---

void setPinOutput(uint8_t index) {
  pinMode(PINS[index], OUTPUT);
}

void setAllPinsOutput() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    pinMode(PINS[i], OUTPUT);
  }
}

void setAllPinsInput() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    pinMode(PINS[i], INPUT_PULLUP);
  }
}

// --- Debounced pin read (pins must be in INPUT_PULLUP mode) ---

bool pinIsLow(uint8_t pin) {
  for (uint8_t s = 0; s < DEBOUNCE_SAMPLES; ++s) {
    if (digitalRead(pin) != LOW) return false;
    delay(1);
  }
  return true;
}

// --- LED control (pins must be in OUTPUT mode) ---

void ledOn(uint8_t index) {
  setPinOutput(index);
  digitalWrite(PINS[index], HIGH);
}

void ledOff(uint8_t index) {
  digitalWrite(PINS[index], LOW);
}

void allLedsOn() {
  setAllPinsOutput();
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    digitalWrite(PINS[i], HIGH);
  }
}

void allLedsOff() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    digitalWrite(PINS[i], LOW);
  }
}

// --- Game feedback ---

void playStep(uint8_t index, uint16_t durationMs) {
  ledOn(index);
  tone(BUZZER_PIN, TONE_FREQS[index], durationMs);
  delay(durationMs);
  noTone(BUZZER_PIN);
  ledOff(index);
}

void playFailureSignal() {
  for (uint8_t i = 0; i < 2; ++i) {
    allLedsOn();
    tone(BUZZER_PIN, 180, 200);
    delay(220);
    noTone(BUZZER_PIN);
    allLedsOff();
    delay(140);
  }
}

void playSuccessSignal() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    playStep(i, 80);
    delay(30);
  }
}

// --- Sequence display (pins in OUTPUT mode) ---

void showSequence() {
  setAllPinsOutput();
  allLedsOff();
  delay(300);
  for (uint8_t i = 0; i < sequenceLength; ++i) {
    playStep(sequenceBuffer[i], STEP_ON_MS);
    delay(STEP_GAP_MS);
  }
}

// --- Button reading (pins switch to INPUT_PULLUP mode) ---

int8_t waitForButtonPress(uint16_t timeoutMs) {
  setAllPinsInput();
  delay(5);  // let pull-ups settle

  const uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
      if (pinIsLow(PINS[i])) {
        // wait for release
        while (digitalRead(PINS[i]) == LOW) {
          delay(5);
        }
        delay(20);  // debounce
        return static_cast<int8_t>(i);
      }
    }
    delay(1);
  }
  return -1;
}

bool readAndValidateInput() {
  for (uint8_t i = 0; i < sequenceLength; ++i) {
    const int8_t pressed = waitForButtonPress(INPUT_TIMEOUT_MS);
    if (pressed < 0) {
      return false;
    }

    // briefly switch to output to flash LED + play tone
    playStep(static_cast<uint8_t>(pressed), 120);

    if (static_cast<uint8_t>(pressed) != sequenceBuffer[i]) {
      return false;
    }
  }
  return true;
}

// --- Game state ---

void resetGame() {
  sequenceLength = 0;
}

void extendSequence() {
  if (sequenceLength < MAX_SEQUENCE) {
    sequenceBuffer[sequenceLength++] = pseudoRandom4();
  }
}

// --- Button check helpers ---

bool anyButtonPressed() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    if (pinIsLow(PINS[i])) return true;
  }
  return false;
}

bool checkButtonPress() {
  setAllPinsInput();
  delay(5);
  bool pressed = anyButtonPressed();
  setAllPinsOutput();
  allLedsOff();
  return pressed;
}

void waitForRelease() {
  setAllPinsInput();
  delay(5);
  while (anyButtonPressed()) {
    delay(5);
  }
  delay(20);
}

// --- Startup screen ---

bool waitForStartup() {
  uint8_t currentLed = 0;
  uint32_t lastSwitch = millis();

  setAllPinsOutput();
  allLedsOff();
  ledOn(currentLed);

  while (true) {
    if ((millis() - lastSwitch) >= 200) {
      ledOff(currentLed);
      currentLed = (currentLed + 1) % BUTTON_COUNT;
      ledOn(currentLed);
      lastSwitch = millis();
    }

    // Poll buttons briefly (~5ms flicker, imperceptible)
    setAllPinsInput();
    delay(5);
    if (anyButtonPressed()) {
      lcgState ^= micros();
      uint32_t pressStart = millis();
      while (anyButtonPressed()) {
        delay(5);
      }
      delay(20);  // debounce
      uint32_t holdDuration = millis() - pressStart;

      setAllPinsOutput();
      allLedsOff();
      return holdDuration >= 3000;
    }

    // Restore LED chase
    setAllPinsOutput();
    allLedsOff();
    ledOn(currentLed);

    delay(12);  // ~20ms poll cycle (5ms settle + 3ms sample + 12ms)
  }
}

// --- Demo mode ---

void runDemo() {
  resetGame();
  while (true) {
    extendSequence();

    // Show sequence, checking for button press between steps
    setAllPinsOutput();
    allLedsOff();
    delay(300);
    for (uint8_t i = 0; i < sequenceLength; ++i) {
      if (checkButtonPress()) return;
      playStep(sequenceBuffer[i], STEP_ON_MS);
      delay(STEP_GAP_MS);
    }

    // Auto-play the correct response
    delay(200);
    for (uint8_t i = 0; i < sequenceLength; ++i) {
      if (checkButtonPress()) return;
      playStep(sequenceBuffer[i], 120);
      delay(80);
    }

    if (checkButtonPress()) return;
    playSuccessSignal();
    delay(250);
  }
}

}  // namespace

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  setAllPinsOutput();
  allLedsOff();
  delay(250);

  if (waitForStartup()) {
    runDemo();
    waitForRelease();
    lcgState ^= micros();
  }
  resetGame();
}

void loop() {
  if (sequenceLength == 0) {
    extendSequence();
  }

  showSequence();

  if (readAndValidateInput()) {
    playSuccessSignal();
    extendSequence();
    delay(250);
  } else {
    playFailureSignal();
    resetGame();
    delay(450);
  }
}
