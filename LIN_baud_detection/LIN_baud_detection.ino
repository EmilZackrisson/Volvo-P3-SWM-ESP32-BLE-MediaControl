#include <Arduino.h>

#define RX_PIN 16
#define TX_PIN 17

HardwareSerial LINSerial(2);

#define LIN_FRAME_MAX 16
#define LIN_TIMEOUT 200 // ms

// PID parity check
bool checkPIDParity(uint8_t pid) {
  uint8_t id = pid & 0x3F;
  uint8_t p0 = ((id >> 0) ^ (id >> 1) ^ (id >> 2) ^ (id >> 4)) & 0x01;
  uint8_t p1 = ~((id >> 1) ^ (id >> 3) ^ (id >> 4) ^ (id >> 5)) & 0x01;
  return (((p0 << 6) | (p1 << 7)) | id) == pid;
}

float detectBaud() {
  // Wait for LIN break: line LOW for > 1ms
  while (digitalRead(RX_PIN) == HIGH) {}
  unsigned long lowStart = micros();
  while (digitalRead(RX_PIN) == LOW) {}
  unsigned long lowDuration = micros() - lowStart;

  if (lowDuration < 1000) {
    return 0; // Not a real break
  }

  // Now measure one HIGH pulse of SYNC (0x55 is alternating bits)
  unsigned long highTime = pulseIn(RX_PIN, HIGH, 2000);
  if (highTime == 0) return 0;

  // One bit = highTime (or lowTime, similar)
  float bitTime = (float)highTime; // in µs
  float baud = 1000000.0 / bitTime;

  return baud;
}

void setup() {
  Serial.begin(115200);
  pinMode(RX_PIN, INPUT);
  Serial.println("Waiting for LIN frames...");
}

void loop() {
  float baud = detectBaud();
  if (baud > 0 && baud < 20000) { // reasonable LIN baud range
    Serial.printf("Detected Baud: %.2f\n", baud);

    LINSerial.end();
    LINSerial.begin((int)baud, SERIAL_8N1, RX_PIN, TX_PIN);

    unsigned long start = millis();
    uint8_t buffer[LIN_FRAME_MAX];
    int index = 0;

    while (millis() - start < LIN_TIMEOUT && index < LIN_FRAME_MAX) {
      if (LINSerial.available()) {
        buffer[index++] = LINSerial.read();
      }
    }

    if (index > 2 && buffer[0] == 0x55) {
      uint8_t pid = buffer[1];
      if (checkPIDParity(pid)) {
        Serial.printf("Valid LIN Frame @ %.2f baud | PID: 0x%02X | Data: ", baud, pid);
        for (int i = 2; i < index; i++) {
          Serial.printf("0x%02X ", buffer[i]);
        }
        Serial.println();
      }
    }
  }
}
