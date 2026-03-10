#include <AccelStepper.h>

#define MOTOR_INTERFACE_TYPE AccelStepper::FULL4WIRE

AccelStepper stepper1(MOTOR_INTERFACE_TYPE, 3, 4, 5, 6);
AccelStepper stepper2(MOTOR_INTERFACE_TYPE, 8, 9, 10, 11);

// --- STRAIGHT MOVEMENTS ---
void go_forward(){
  stepper1.setSpeed(1200);
  stepper2.setSpeed(1200);
}
void go_right(){
  stepper1.setSpeed(1200);
  stepper2.setSpeed(-1200);
}
void go_left(){
  stepper1.setSpeed(-1200);
  stepper2.setSpeed(1200);
}
void go_back(){
  stepper1.setSpeed(-1200);
  stepper2.setSpeed(-1200);
}

// --- DIAGONAL MOVEMENTS (H-Bot Kinematics) ---
// In an H-Bot, spinning only one motor creates 45-degree diagonal movement.
void go_forward_right(){
  stepper1.setSpeed(0);
  stepper2.setSpeed(-1200);
}
void go_forward_left(){
  stepper1.setSpeed(-1200);
  stepper2.setSpeed(0);
}
void go_back_right(){
  stepper1.setSpeed(1200);
  stepper2.setSpeed(0);
}
void go_back_left(){
  stepper1.setSpeed(0);
  stepper2.setSpeed(1200);
}

void stop_motors(){
  stepper1.setSpeed(0);
  stepper2.setSpeed(0);
}

void setup() {
  // Start serial communication at 9600 baud
  Serial.begin(9600); 
  
  stepper1.setMaxSpeed(1200);
  stepper2.setMaxSpeed(1200);
  
  // Make sure motors start completely stopped
  stop_motors(); 
}

void loop() {
  // Check if the Mac has sent a command
  if (Serial.available() > 0) {
    char command = Serial.read();

    // Trigger the functions based on the letter received
    switch (command) {
      // Straight
      case 'F': go_forward(); break;
      case 'B': go_back(); break;
      case 'L': go_left(); break;
      case 'R': go_right(); break;
      case 'S': stop_motors(); break;
      
      // Diagonals
      case 'E': go_forward_right(); break; // E is top-right of WASD
      case 'Q': go_forward_left(); break;  // Q is top-left of WASD
      case 'C': go_back_right(); break;    // C is bottom-right
      case 'Z': go_back_left(); break;     // Z is bottom-left
    }
  }

  // 🔥 THIS IS THE IMPORTANT PART
  stepper1.runSpeed();
  stepper2.runSpeed();
}