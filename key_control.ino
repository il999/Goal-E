#include <AccelStepper.h>

// --- SETTINGS ---
// Start low (400-600) to test mechanical grip, then increase.
float MAX_SPEED = 900.0; 
float ACCEL = 8000.0;    // The "ramp" that prevents belt slipping/stalling

// --- CNC SHIELD PIN DEFINITIONS ---
#define ENABLE_PIN 8
#define Y_STEP_PIN 3
#define Y_DIR_PIN 6
#define X_STEP_PIN 2
#define X_DIR_PIN 5

// Initialize motors
AccelStepper stepperA(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperB(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);

void setup() {
  Serial.begin(9600);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable drivers

  stepperA.setMaxSpeed(MAX_SPEED);
  stepperA.setAcceleration(ACCEL);
  
  stepperB.setMaxSpeed(MAX_SPEED);
  stepperB.setAcceleration(ACCEL);

  Serial.println("H-Bot Test Ready. Send F, B, L, R, Q, E, Z, C, or S.");
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    // We use move() with a large number so they keep spinning 
    // until a new command or 'S' (stop) is sent.
    switch (command) {
      case 'F': setTargets(10000, 10000); break;  // Forward
      case 'B': setTargets(-10000, -10000); break; // Back
      case 'R': setTargets(10000, -10000); break;  // Right
      case 'L': setTargets(-10000, 10000); break;  // Left
      
      case 'E': setTargets(10000, 0); break;      // Forward-Right (Motor B stays locked)
      case 'Q': setTargets(0, 10000); break;      // Forward-Left (Motor A stays locked)
      case 'C': setTargets(0, -10000); break;     // Back-Right
      case 'Z': setTargets(-10000, 0); break;     // Back-Left
      
      case 'S': // Hard stop
        stepperA.stop(); 
        stepperB.stop(); 
        break;
    }
  }

  // This handles the acceleration ramping automatically
  stepperA.run();
  stepperB.run();
}

void setTargets(long targetA, long targetB) {
  // Reset positions so "10000" is always a long distance from current
  stepperA.setCurrentPosition(0);
  stepperB.setCurrentPosition(0);
  stepperA.moveTo(targetA);
  stepperB.moveTo(targetB);
}
