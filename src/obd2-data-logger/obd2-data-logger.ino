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
char *EngineRPM, *VehicleSpeed, *ThrottlePosition;

// Data will be temporarily stored to this buffer before being written to the file
char engineRpmBuffer[64], vehicleSpeedBuffer[64], throttlePositionBuffer[64];

bool isActive; // State of logging activity

// Names of files to write config info and data to
char configFilename[] = "data.csv";
char dataFilename[] = "config.txt";
char columnHeaders[] = "engine_rpm,vehicle_speed,throttle_position";

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
  
  // Write LED pins low to turn them on
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);
  
  // Initialize CAN Controller 
  if (Canbus.init(CANSPEED_500)) {  // Initialize MCP2515 CAN controller at the specified speed
    Serial.println("CAN init successful");
    delay(1500);
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
    delay(1500);
  }

  // Check if data file exists.  If not, write column headers.
  setupDataFile(dataFilename, columnHeaders);
  generateConfigFile(configFilename);

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
    digitalWrite(LED_A, HIGH); // Turn on LED_A to indicate active CAN Bus traffic
    
    Canbus.ecu_req(ENGINE_RPM, engineRpmBuffer); // Request engine RPM
    Serial.print("Engine RPM: ");
    Serial.println(engineRpmBuffer);
    
    Canbus.ecu_req(VEHICLE_SPEED, vehicleSpeedBuffer); // Request vehicle speed
    Serial.print("Vehicle speed: ");
    Serial.println(vehicleSpeedBuffer);

    Canbus.ecu_req(THROTTLE, throttlePositionBuffer); // Request throttle position
    Serial.print("Throttle position: ");
    Serial.println(throttlePositionBuffer);
   
    digitalWrite(LED_A, LOW); // Turn off LED_A
    digitalWrite(LED_B, HIGH); // Turn on LED_B
    delay(500);
      
    File dataFile = openFile(dataFilename);

    dataFile.print(EngineRPM);
    dataFile.print(",");
    dataFile.print(VehicleSpeed);
    dataFile.print(",");
    dataFile.print(ThrottlePosition);
    dataFile.println();

    dataFile.flush();
    dataFile.close(); // Close data logging file

    digitalWrite(LED_B, LOW); // Turn off LED_B
  }
  
  // Check for joystick click and update state.
  if (digitalRead(CLICK)==LOW) {
    isActive = !isActive;
    while (digitalRead(CLICK)==LOW) {} // Hold code in this state until button is released
  }
}

// Check if data file exists.  If not, write column headers to data file.
void setupDataFile(char filename[], char columnHeaders[]) {
  if (!SD.exists(filename)) {
    File dataFile = openFile(filename);

    dataFile.println(columnHeaders);
    
    dataFile.flush();
    dataFile.close(); // Close data logging file
  }
}

// Create config file with available PIDs.
void generateConfigFile(char filename[]) {
  char *AvailablePidBuffer;
  Canbus.ecu_req(0x01, AvailablePidBuffer); // Request all available PIDs
  Serial.print("Available PIDs: ");
  Serial.println(AvailablePidBuffer);

  File configFile = openFile(filename);
  configFile.print("Available PIDs: ");
  configFile.println(AvailablePidBuffer);
  
  configFile.flush();
  configFile.close();
}

File openFile(char filename[]) {
  File file = SD.open(filename, FILE_WRITE); // Open uSD file to log data
  
  // If data file can't be opened, throw error.
  if (!file){
    Serial.println("Error opening file: ");
    Serial.println(filename);
  }

  return file;
}
