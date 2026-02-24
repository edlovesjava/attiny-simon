// ATtiny85 Simon Game
//
// Pin allocation (5 usable I/O pins):
//   PB0–PB3: shared LED + button (time-multiplexed)
//   PB4:     buzzer
//
// Each PB0–PB3 pin is wired as:
//   PBx ──┬──[220R]──LED──GND
//         ├──[10K]────────VCC
//         └──[button]─────GND
//
// LED phase:  pinMode OUTPUT, digitalWrite HIGH/LOW
// Button phase: pinMode INPUT, digitalRead (external 10K pull-up to VCC)

#define BUTTON_COUNT 4
#define MAX_SEQUENCE 32

const uint8_t PINS[BUTTON_COUNT] = {PB0, PB1, PB2, PB3};
const uint8_t BUZZER_PIN = PB4;

const uint16_t TONE_FREQS[BUTTON_COUNT] = {262, 330, 392, 523};

const uint16_t STEP_ON_MS = 250;
const uint16_t STEP_GAP_MS = 120;
const uint16_t INPUT_TIMEOUT_MS = 4000;

uint8_t sequenceBuffer[MAX_SEQUENCE] = {};
uint8_t sequenceLength = 0;

uint32_t lcgState = 0xA5A5A5A5UL;

uint8_t pseudoRandom4() {
  lcgState = (1103515245UL * lcgState + 12345UL);
  return (uint8_t)((lcgState >> 16) & 0x03);
}

// --- Pin mode switching ---

void setPinOutput(uint8_t index) {
  pinMode(PINS[index], OUTPUT);
}

void setAllPinsOutput() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(PINS[i], OUTPUT);
  }
}

void setAllPinsInput() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(PINS[i], INPUT);
  }
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
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    digitalWrite(PINS[i], HIGH);
  }
}

void allLedsOff() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
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
  for (uint8_t i = 0; i < 2; i++) {
    allLedsOn();
    tone(BUZZER_PIN, 180, 200);
    delay(220);
    noTone(BUZZER_PIN);
    allLedsOff();
    delay(140);
  }
}

void playSuccessSignal() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    playStep(i, 80);
    delay(30);
  }
}

// --- Sequence display (pins in OUTPUT mode) ---

void showSequence() {
  setAllPinsOutput();
  allLedsOff();
  delay(300);
  for (uint8_t i = 0; i < sequenceLength; i++) {
    playStep(sequenceBuffer[i], STEP_ON_MS);
    delay(STEP_GAP_MS);
  }
}

// --- Button reading (pins switch to INPUT mode) ---

int8_t waitForButtonPress(uint16_t timeoutMs) {
  setAllPinsInput();
  delay(2);  // let pull-ups settle

  uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
      if (digitalRead(PINS[i]) == LOW) {
        // wait for release
        while (digitalRead(PINS[i]) == LOW) {
          delay(5);
        }
        delay(20);  // debounce
        return (int8_t)i;
      }
    }
    delay(1);
  }
  return -1;
}

bool readAndValidateInput() {
  for (uint8_t i = 0; i < sequenceLength; i++) {
    int8_t pressed = waitForButtonPress(INPUT_TIMEOUT_MS);
    if (pressed < 0) {
      return false;
    }

    // briefly switch to output to flash LED + play tone
    playStep((uint8_t)pressed, 120);

    if ((uint8_t)pressed != sequenceBuffer[i]) {
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

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  setAllPinsOutput();
  allLedsOff();

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
