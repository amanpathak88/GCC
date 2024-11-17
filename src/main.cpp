#include <Arduino.h>
#include <AFMotor.h>
#include <RH_ASK.h>

RH_ASK driver;

// Create motor objects for M1 and M3
AF_DCMotor rightMotor(1);  // M1
AF_DCMotor leftMotor(3);   // M3

// Convert percentage to motor speed (0-255)
int speed(int percent) {
  return map(percent, 0, 100, 0, 255);
}

uint8_t get_command() {
    uint8_t buf[1];  // Single byte buffer since transmitter sends one byte
    uint8_t buflen = sizeof(buf);
    uint8_t command = 5;  // Default to 5 (Stop) if no valid command received
    
    if (driver.recv(buf, &buflen)) {  // Non-blocking receive
        command = buf[0];  // Get the command byte
        
        // Validate the received command
        if (command >= 1 && command <= 5) {
            // Debug output with direction
            Serial.print("Received: ");
            switch(command) {
                case 1:
                    Serial.println("1 (Forward)");
                    break;
                case 2:
                    Serial.println("2 (Backward)");
                    break;
                case 3:
                    Serial.println("3 (Left)");
                    break;
                case 4:
                    Serial.println("4 (Right)");
                    break;
                case 5:
                    Serial.println("5 (Stop)");
                    break;
            }
        }
    }
    
    return command;
}

uint8_t serial_command() {
    uint8_t command = 5;  // Default to stop (5)
    
    if (Serial.available() > 0) {
        // Read the incoming byte
        char input = Serial.read();
        
        // Convert character to integer and validate
        if (input >= '1' && input <= '5') {
            command = input - '0';  // Convert ASCII to integer
            
            // Debug output with direction
            Serial.print("Serial Command: ");
            switch(command) {
                case 1:
                    Serial.println("1 (Forward)");
                    break;
                case 2:
                    Serial.println("2 (Backward)");
                    break;
                case 3:
                    Serial.println("3 (Left)");
                    break;
                case 4:
                    Serial.println("4 (Right)");
                    break;
                case 5:
                    Serial.println("5 (Stop)");
                    break;
            }
        } else {
            // Clear invalid input
            while(Serial.available()) {
                Serial.read();
            }
            Serial.println("Invalid input! Use 1-5");
        }
    }
    
    return command;
}

void setup() {
  Serial.begin(9600);
  if (!driver.init())
    Serial.println("init failed");

  Serial.println("Motor Control Ready");
  Serial.println("Commands:");
  Serial.println("1: Forward");
  Serial.println("2: Backward");
  Serial.println("3: Left");
  Serial.println("4: Right");
  Serial.println("5: Stop");
  
  // Set initial speed for both motors (70% speed)
  rightMotor.setSpeed(speed(100));
  leftMotor.setSpeed(speed(40));
  
  // Initially stop both motors
  rightMotor.run(RELEASE);
  leftMotor.run(RELEASE);
}

unsigned long lastCheckTime = 0;  // Time of last RF check
const unsigned long CHECK_INTERVAL = 2000;  // Check every 2 seconds
uint8_t currentCommand = 5;  // Current active command

void loop() {
    // Check RF module every 2 seconds
    if (millis() - lastCheckTime >= CHECK_INTERVAL) {
        uint8_t rf_cmd = get_command();  // Check RF receiver
        
        // Update only if new command is different
        if (rf_cmd != currentCommand) {
            currentCommand = rf_cmd;  // Update current command
            Serial.print("New command: ");
            Serial.println(currentCommand);
        }
        
        lastCheckTime = millis();  // Update last check time
    }
    
    // Execute current command continuously
    switch (currentCommand) {
        case 1:  // Forward
            rightMotor.run(FORWARD);
            leftMotor.run(FORWARD);
            break;
            
        case 2:  // Backward
            rightMotor.run(BACKWARD);
            leftMotor.run(BACKWARD);
            break;
            
        case 3:  // Left Turn
            rightMotor.run(FORWARD);
            leftMotor.run(BACKWARD);
            break;
            
        case 4:  // Right Turn
            rightMotor.run(BACKWARD);
            leftMotor.run(FORWARD);
            break;
            
        case 5:  // Stop
            rightMotor.run(RELEASE);
            leftMotor.run(RELEASE);
            break;
            
        default:
            rightMotor.run(RELEASE);
            leftMotor.run(RELEASE);
            Serial.println("Invalid command! Use 1-5");
            break;
    }
    
    delay(10);  // Small delay to prevent CPU overload
}