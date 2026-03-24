// ============================================================
//  Air Hockey Defensive Pusher — Arduino Firmware
//  CoreXY belt system
//  Receives:  "x,y\n"  (table-space coords 0..600, 0..300)
//  Drives two stepper motors in CoreXY kinematics
// ============================================================

#include <Arduino.h>

// ---- Pin definitions ----
#define STEP1   9
#define DIR1    10
#define STEP2   12
#define DIR2    13

// ---- Table dimensions (must match Python TABLE_W / TABLE_H) ----
#define TABLE_W  600
#define TABLE_H  300

// ---- Physical step range — tune to your belt & motor setup ----
#define STEPS_X  8000     // total steps across TABLE_W
#define STEPS_Y  4000     // total steps across TABLE_H

// ---- Speed: microseconds between step pulses (lower = faster) ----
#define STEP_DELAY_US   60
#define INTER_STEP_US   50

// ---- Current CoreXY motor positions (in step counts) ----
long pos1 = 0;   // motor A
long pos2 = 0;   // motor B

// ============================================================
//  Single step pulse
// ============================================================
inline void pulseStep(int pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(STEP_DELAY_US);
  digitalWrite(pin, LOW);
}

// ============================================================
//  Move both motors simultaneously (CoreXY)
// ============================================================
void stepMotors(long a, long b) {

  digitalWrite(DIR1, a > 0 ? HIGH : LOW);
  digitalWrite(DIR2, b > 0 ? HIGH : LOW);

  long stepsA = abs(a);
  long stepsB = abs(b);

  while (stepsA > 0 || stepsB > 0) {
    if (stepsA > 0) { pulseStep(STEP1); stepsA--; }
    if (stepsB > 0) { pulseStep(STEP2); stepsB--; }
    delayMicroseconds(INTER_STEP_US);
  }
}

// ============================================================
//  Move to absolute table coordinate (x, y)
// ============================================================
void moveTo(int x, int y) {

  // Clamp to valid range
  x = constrain(x, 0, TABLE_W);
  y = constrain(y, 0, TABLE_H);

  // Map table coords → step counts
  long targetX = map(x, 0, TABLE_W, 0, STEPS_X);
  long targetY = map(y, 0, TABLE_H, 0, STEPS_Y);

  // CoreXY kinematics:
  //   Motor A (pos1) = Y - X
  //   Motor B (pos2) = X + Y
  long targetA = targetY - targetX;
  long targetB = targetX + targetY;

  long deltaA = targetA - pos1;
  long deltaB = targetB - pos2;

  if (deltaA == 0 && deltaB == 0) return;   // already there

  stepMotors(deltaA, deltaB);

  pos1 = targetA;
  pos2 = targetB;
}

// ============================================================
//  Setup
// ============================================================
void setup() {

  pinMode(STEP1, OUTPUT);
  pinMode(DIR1,  OUTPUT);
  pinMode(STEP2, OUTPUT);
  pinMode(DIR2,  OUTPUT);

  Serial.begin(115200);
  Serial.setTimeout(10);        // short timeout so loop stays fast

  // Home to center of defense line (optional — safe default)
  // moveTo(TABLE_W / 2, TABLE_H / 2);

  Serial.println("READY");
}

// ============================================================
//  Main loop — parse "x,y\n" from Python
// ============================================================
void loop() {

  if (Serial.available()) {

    // Read until newline
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) return;

    int comma = line.indexOf(',');
    if (comma < 0) return;

    int x = line.substring(0, comma).toInt();
    int y = line.substring(comma + 1).toInt();

    moveTo(x, y);

    // Optional ACK — comment out if it causes lag
    // Serial.print("OK ");
    // Serial.print(x); Serial.print(","); Serial.println(y);
  }
}
