#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <Canbus.h>

// Chip Select pin is tied to pin 9 on the SparkFun CAN-Bus Shield
const int chipSelect = 9;

// Define Joystick connection pins 
//#define UP     A1
//#define DOWN   A3
//#define LEFT   A2
//#define RIGHT  A5
#define CLICK  A4

//Define LED pins
#define LED_A 7
#define LED_B 8

// Declare SD File
File dataFile;

// Declare CAN variables for communication
char *EngineRPM;
char buffer[64];  // Data will be temporarily stored to this buffer before being written to the file

bool isActive; // State of logging activity

//********************************Setup Loop*********************************//
void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(9600);
  Serial.println("ECU Demo");
  
  // Initialize pins as necessary
  pinMode(chipSelect, OUTPUT);
  pinMode(CLICK, INPUT);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  // Pull analog pins high to enable reading of joystick movements
  digitalWrite(CLICK, HIGH);
  
  // Write LED pins low to turn them off by default
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
  
  // Initialize CAN Controller 
//  if (Canbus.init(CANSPEED_500)) {  // Initialize MCP2515 CAN controller at the specified speed
//    Serial.println("CAN Init Ok");
//    delay(1500);
//  } 
//  else {
//    Serial.println("Can't init CAN");
//    return;
//  } 

  // Check if uSD card initialized
//  if (!SD.begin(chipSelect)) {
//    Serial.println("uSD card failed to initialize, or is not present");
//    return;
//  }
//  else {
//    Serial.println("uSD card initialized.");
//    delay(1500);
//  }

  // Set initial state of logger
  isActive = false;
}

//********************************Main Loop*********************************//
void loop() {
  Serial.println(isActive);
  
  if (isActive) {
    digitalWrite(LED_A, HIGH); // Turn on LED_A to indicate CAN Bus traffic
//    Canbus.ecu_req(ENGINE_RPM,buffer); // Request engine RPM
//    EngineRPM = buffer;
    Serial.print("Engine RPM: ");
//    Serial.println(buffer);
    delay(100);
   
    digitalWrite(LED_A, LOW); // Turn off LED_A
    digitalWrite(LED_B, HIGH); // Turn on LED_B
    delay(500);
      
//    File dataFile = SD.open("data.txt", FILE_WRITE); // Open uSD file to log data
      
    // If data file can't be opened, throw error.
//    if (!dataFile){
//      Serial.print("Error opening");
//      Serial.println("data.txt");
//    }
        
    Serial.println("Logging. Click to stop logging.");
    
//    dataFile.print("Engine RPM: ");
//    dataFile.println(EngineRPM);
//    
//    dataFile.println();
//    dataFile.flush();
//    dataFile.close(); // Close data logging file

    digitalWrite(LED_B, LOW); // Turn off LED_B
  }
  
  // Check for joystick click and update state.
  if (digitalRead(CLICK)==LOW) {
    isActive = !isActive;
    while (digitalRead(CLICK)==LOW) {} // Hold code in this state until button is released
  }
}
