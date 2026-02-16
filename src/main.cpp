#include <Arduino.h>

namespace {
constexpr uint8_t BUTTON_COUNT = 4;
constexpr uint8_t MAX_SEQUENCE = 32;

constexpr uint8_t LED_PINS[BUTTON_COUNT] = {0, 1, 2, 3};
constexpr uint8_t BUTTON_PINS[BUTTON_COUNT] = {4, 5, 6, 7};

constexpr uint16_t TONE_FREQS[BUTTON_COUNT] = {262, 330, 392, 523};
constexpr uint8_t BUZZER_PIN = 8;

constexpr uint16_t STEP_ON_MS = 250;
constexpr uint16_t STEP_GAP_MS = 120;
constexpr uint16_t INPUT_TIMEOUT_MS = 4000;

uint8_t sequenceBuffer[MAX_SEQUENCE] = {};
uint8_t sequenceLength = 0;

uint32_t lcgState = 0xA5A5A5A5UL;

uint8_t pseudoRandom4() {
  lcgState = (1103515245UL * lcgState + 12345UL);
  return static_cast<uint8_t>((lcgState >> 16) & 0x03);
}

void clearAllLeds() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

void playStep(uint8_t index, uint16_t durationMs) {
  digitalWrite(LED_PINS[index], HIGH);
  tone(BUZZER_PIN, TONE_FREQS[index], durationMs);
  delay(durationMs);
  noTone(BUZZER_PIN);
  digitalWrite(LED_PINS[index], LOW);
}

void playFailureSignal() {
  for (uint8_t i = 0; i < 2; ++i) {
    for (uint8_t j = 0; j < BUTTON_COUNT; ++j) {
      digitalWrite(LED_PINS[j], HIGH);
    }
    tone(BUZZER_PIN, 180, 200);
    delay(220);
    noTone(BUZZER_PIN);
    clearAllLeds();
    delay(140);
  }
}

void playSuccessSignal() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    playStep(i, 80);
    delay(30);
  }
}

void showSequence() {
  delay(300);
  for (uint8_t i = 0; i < sequenceLength; ++i) {
    playStep(sequenceBuffer[i], STEP_ON_MS);
    delay(STEP_GAP_MS);
  }
}

int8_t waitForButtonPress(uint16_t timeoutMs) {
  const uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {
    for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        while (digitalRead(BUTTON_PINS[i]) == LOW) {
          delay(5);
        }
        delay(20);
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

    playStep(static_cast<uint8_t>(pressed), 120);

    if (static_cast<uint8_t>(pressed) != sequenceBuffer[i]) {
      return false;
    }
  }
  return true;
}

void resetGame() {
  sequenceLength = 0;
}

void extendSequence() {
  if (sequenceLength < MAX_SEQUENCE) {
    sequenceBuffer[sequenceLength++] = pseudoRandom4();
  }
}
}  // namespace

void setup() {
  for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
    pinMode(LED_PINS[i], OUTPUT);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  pinMode(BUZZER_PIN, OUTPUT);
  clearAllLeds();

  lcgState ^= micros();
  delay(250);
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