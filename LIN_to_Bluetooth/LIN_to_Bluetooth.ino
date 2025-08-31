#include <BleKeyboard.h>

// ===== LIN Config =====
#define RX_PIN 16
#define TX_PIN 17
#define LIN_BAUD 9434
#define LIN_MAX_BUF 32

#define LED 2

BleKeyboard bleKeyboard("Volvo SWM BT");

// LIN frames for Next / Previous track
uint8_t nextTrackData[4] = {0x00, 0x10, 0x00, 0x00};
uint8_t prevTrackData[4] = {0x00, 0x02, 0x00, 0x00};
uint8_t enterData[4]     = {0x00, 0x20, 0x00, 0x00};
uint8_t exitData[4]      = {0x00, 0x40, 0x00, 0x00};

uint8_t nextChecksum  = 0xEF;
uint8_t prevChecksum  = 0xFD;
uint8_t enterChecksum = 0xDF;
uint8_t exitChecksum  = 0xBF;

// State tracking (edge detection)
bool prevNextPressed = false;
bool prevPrevPressed = false;
bool prevPlayPressed = false;
bool prevPausePressed = false;

// Rate-limit the not connected message
unsigned long lastNotConnectedMsg = 0;

// ===== Helper Functions =====
bool compareData(uint8_t *data, uint8_t *pattern, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (data[i] != pattern[i]) return false;
  }
  return true;
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  Serial2.begin(LIN_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("Starting BLE...");
  bleKeyboard.begin();
  Serial.println("ESP32 ready. Pair with your phone.");
}

void loop() {
  static uint8_t buffer[LIN_MAX_BUF];
  static size_t bufIndex = 0;

  while (Serial2.available()) {
    uint8_t b = Serial2.read();
    buffer[bufIndex++] = b;

    if (bufIndex > LIN_MAX_BUF) bufIndex = 0;

    if (bufIndex >= 7) {
      for (int i = 0; i <= bufIndex - 7; i++) {
        if (buffer[i] == 0x55 && buffer[i + 1] == 0x20) {
          uint8_t data[4] = {buffer[i + 2], buffer[i + 3], buffer[i + 4], buffer[i + 5]};
          uint8_t checksum = buffer[i + 6];

          bool nextPressed  = (compareData(data, nextTrackData, 4) && checksum == nextChecksum);
          bool prevPressed  = (compareData(data, prevTrackData, 4) && checksum == prevChecksum);
          bool playPressed  = (compareData(data, enterData, 4) && checksum == enterChecksum);
          bool pausePressed = (compareData(data, exitData, 4) && checksum == exitChecksum);

          // Detect edge: only trigger when transitioning from false -> true
          if (nextPressed && !prevNextPressed) {
            Serial.println("Next Track detected");
            nextTrack();
          }

          if (prevPressed && !prevPrevPressed) {
            Serial.println("Previous Track detected");
            previousTrack();
          }

          if (playPressed && !prevPlayPressed) {
            Serial.println("Play detected");
            sendPlayPause();
          }

          if (pausePressed && !prevPausePressed) {
            Serial.println("Pause detected");
            sendPlayPause();
          }

          // Update previous states
          prevNextPressed  = nextPressed;
          prevPrevPressed  = prevPressed;
          prevPlayPressed  = playPressed;
          prevPausePressed = pausePressed;

          // Reset buffer after processing a valid frame
          bufIndex = 0;
          break;
        }
      }
    }
  }

  if (!bleKeyboard.isConnected()) {
    print_not_connected();
  }
}

void print_not_connected() {
  unsigned long now = millis();
  if ((now - lastNotConnectedMsg) > 5000) {
    Serial.println("BLE Not connected...");
    lastNotConnectedMsg = now;
  }
}

void sendPlayPause() {
  Serial.println("Sending play/pause");
  if (bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
  }
}

void nextTrack() {
  if (bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
  }
}

void previousTrack() {
  if (bleKeyboard.isConnected()) {
    bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
  }
}
