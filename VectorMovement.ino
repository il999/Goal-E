// --- CNC SHIELD PIN DEFINITIONS ---
#define ENABLE_PIN 8

// Stepper 1: Y-Axis Pins (H-Bot Motor A)
#define Y_STEP_PIN 3
#define Y_DIR_PIN 6

// Stepper 2: X-Axis Pins (H-Bot Motor B)
#define X_STEP_PIN 2
#define X_DIR_PIN 5

// --- System Configuration ---
const float MAX_STEPS_PER_SEC = 4000.0;  // Top speed (Push this higher to test limits)
const float ACCELERATION = 6000.0;       // Acceleration (steps/sec^2)
const unsigned long TIMEOUT_MS = 500;    // Safety timeout

// --- Motor States ---
float currentSpeedA = 0.0;
float currentSpeedB = 0.0;
float targetSpeedA = 0.0;
float targetSpeedB = 0.0;

float phaseA = 0.0;
float phaseB = 0.0;

unsigned long lastTime;
unsigned long lastCommandTime;

char serialBuffer[32];
int bufIndex = 0;

void setup() {
  // Configure CNC Shield Pins
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);

  // CNC Shields require ENABLE to be LOW to activate drivers
  digitalWrite(ENABLE_PIN, LOW);

  Serial.begin(115200);
  lastTime = micros();
  lastCommandTime = millis();
}

void loop() {
  unsigned long now = micros();
  float dt = (now - lastTime) * 0.000001;
  lastTime = now;

  readSerial();
  checkWatchdog();
  updateSpeeds(dt);
  generateSteps(dt);
}

void readSerial() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      serialBuffer[bufIndex] = '\0';
      parseVector(serialBuffer);
      bufIndex = 0;
    } else if (bufIndex < 31) {
      serialBuffer[bufIndex++] = c;
    }
  }
}

void parseVector(char* data) {
  char* commaPos = strchr(data, ',');
  if (commaPos != NULL) {
    *commaPos = '\0';
    float vx = atof(data);
    float vy = atof(commaPos + 1);

    vx = constrain(vx, -1.0, 1.0);
    vy = constrain(vy, -1.0, 1.0);

    // H-Bot Kinematics applied to CNC Shield Axes
    float speedX = vx * MAX_STEPS_PER_SEC;
    float speedY = vy * MAX_STEPS_PER_SEC;

    targetSpeedA = constrain(speedX + speedY, -MAX_STEPS_PER_SEC, MAX_STEPS_PER_SEC);
    targetSpeedB = constrain(speedX - speedY, -MAX_STEPS_PER_SEC, MAX_STEPS_PER_SEC);

    lastCommandTime = millis();
  }
}

void checkWatchdog() {
  if (millis() - lastCommandTime > TIMEOUT_MS) {
    targetSpeedA = 0.0;
    targetSpeedB = 0.0;
  }
}

void updateSpeeds(float dt) {
  float speedChange = ACCELERATION * dt;

  if (currentSpeedA < targetSpeedA) {
    currentSpeedA += speedChange;
    if (currentSpeedA > targetSpeedA) currentSpeedA = targetSpeedA;
  } else if (currentSpeedA > targetSpeedA) {
    currentSpeedA -= speedChange;
    if (currentSpeedA < targetSpeedA) currentSpeedA = targetSpeedA;
  }

  if (currentSpeedB < targetSpeedB) {
    currentSpeedB += speedChange;
    if (currentSpeedB > targetSpeedB) currentSpeedB = targetSpeedB;
  } else if (currentSpeedB > targetSpeedB) {
    currentSpeedB -= speedChange;
    if (currentSpeedB < targetSpeedB) currentSpeedB = targetSpeedB;
  }
}

void generateSteps(float dt) {
  phaseA += currentSpeedA * dt;
  phaseB += currentSpeedB * dt;

  // Motor A (Y-Axis Pins)
  if (phaseA >= 1.0) {
    digitalWrite(Y_DIR_PIN, HIGH);
    pulseMotor(Y_STEP_PIN);
    phaseA -= 1.0;
  } else if (phaseA <= -1.0) {
    digitalWrite(Y_DIR_PIN, LOW);
    pulseMotor(Y_STEP_PIN);
    phaseA += 1.0;
  }

  // Motor B (X-Axis Pins)
  if (phaseB >= 1.0) {
    digitalWrite(X_DIR_PIN, HIGH);
    pulseMotor(X_STEP_PIN);
    phaseB -= 1.0;
  } else if (phaseB <= -1.0) {
    digitalWrite(X_DIR_PIN, LOW);
    pulseMotor(X_STEP_PIN);
    phaseB += 1.0;
  }
}

void pulseMotor(int pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(2);
  digitalWrite(pin, LOW);
}
