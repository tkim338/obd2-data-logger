#include <SPI.h> // dependency of SD.h
#include <SD.h>
#include <CAN.h> // dependency of OBD2.h
#include "OBD2.h"

// Chip Select pin is tied to pin 9 on the SparkFun CAN-Bus Shield
#define chipSelect 9

// Define Joystick connection pins
//#define UP     A1
//#define DOWN   A3
//#define LEFT   A2
//#define RIGHT  A5
#define CLICK  A4

// Define LED pins
#define LED_A 7
#define LED_B 8

//OBD2Class OBD2();

bool isActive; // State of logging activity

int numCols = 6;
int pidsOfInterest[] = {
  ::CALCULATED_ENGINE_LOAD,
  ::INTAKE_MANIFOLD_ABSOLUTE_PRESSURE,
  ::ENGINE_RPM,
  ::VEHICLE_SPEED,
  ::TIMING_ADVANCE,
  ::THROTTLE_POSITION
};
float dataRow[6];

// Names of files to write config info and data to
char configFilename[] = "config.txt";
char dataFilename[] = "data.csv";

//********************************Setup Loop*********************************//
void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(9600);
  Serial.println("*** ECU data logger initializing ***");

  // Initialize pins as necessary
  pinMode(chipSelect, OUTPUT);
  pinMode(CLICK, INPUT);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Pull analog pins high to enable reading of joystick movements
  digitalWrite(CLICK, HIGH);

  // Write LED pins high to turn them on
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);

  // Initialize CAN Controller
  if (OBD2.begin()) {
    Serial.println("CAN init successful");
    delay(100);
  }
  else {
    Serial.println("CAN init failed");
    return;
  }

  // Check if uSD card initialized
  if (!SD.begin(chipSelect)) {
    Serial.println("uSD card failed to initialize, or is not present");
    return;
  }
  else {
    Serial.println("uSD card initialized");
    delay(100);
  }

  // Record all supported PIDs (up to 96) to config file
  File configFile = SD.open(configFilename, FILE_WRITE);
  for (int pid = 0; pid < 96; pid++) {
    if (OBD2.pidSupported(pid)) {
      configFile.print(pid);
      configFile.print(',');
      configFile.print(OBD2.pidName(pid));
      configFile.print(',');
      configFile.print(OBD2.pidUnits(pid));
      configFile.println();
    }
  }
  configFile.flush();
  configFile.close();

  // Initialize data file.  If file doesn't already exist, write column headers to first line.
  if (!SD.exists(dataFilename)) {
    File dataFile = SD.open(dataFilename, FILE_WRITE);
    for (int i = 0; i < numCols; i++) {
      dataFile.print(OBD2.pidName(pidsOfInterest[i]));
      dataFile.print(",");
    }
    dataFile.println();
    dataFile.flush();
    dataFile.close();
  }

  // Set initial state of logger
  isActive = false;

  // Turn off LEDs
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
}

//********************************Main Loop*********************************//
void loop() {
  if (isActive) {
    Serial.println("Logging is active. Click to stop logging.");
  }
  else {
    Serial.println("Idle. Click to start logging.");
  }

  if (isActive) {
    // Read row of data from OBD2
    digitalWrite(LED_A, HIGH);
    for (int i = 0; i < numCols; i++) {
      dataRow[i] = OBD2.pidRead(pidsOfInterest[i]);
    }
    digitalWrite(LED_A, LOW);
  
    // Write row of data to file
    digitalWrite(LED_B, HIGH); // Turn on LED_B
    File dataFile = SD.open(dataFilename, FILE_WRITE);
    for (int i = 0; i < numCols; i++) {
      dataFile.print(dataRow[i]);
      dataFile.print(',');
    }
    dataFile.println();
    dataFile.flush();
    dataFile.close();
    digitalWrite(LED_B, LOW); // Turn off LED_B
  }

  // Check for joystick click and update state.
  if (digitalRead(CLICK) == LOW) {
    isActive = !isActive;
    while (digitalRead(CLICK) == LOW) {} // Hold code in this state until button is released
  }
}
