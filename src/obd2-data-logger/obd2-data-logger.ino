//#include <SPI.h>
#include <SD.h>
//#include <SoftwareSerial.h>
#include <CAN.h>
#include <OBD2.h>

// Chip Select pin is tied to pin 9 on the SparkFun CAN-Bus Shield
const int chipSelect = 9;

// Define Joystick connection pins
//#define UP     A1
//#define DOWN   A3
//#define LEFT   A2
//#define RIGHT  A5
#define CLICK  A4

// Define LED pins
#define LED_A 7
#define LED_B 8

// Declare SD File
File dataFile;

bool isActive; // State of logging activity

// Names of files to write config info and data to
String configFilename = "config.txt";
String dataFilename = "data.csv";
String columnHeaders = "";
int pidsOfInterest[] = {
  ::CALCULATED_ENGINE_LOAD,
  ::ENGINE_COOLANT_TEMPERATURE,
  ::INTAKE_MANIFOLD_ABSOLUTE_PRESSURE,
  ::ENGINE_RPM,
  ::VEHICLE_SPEED,
  ::TIMING_ADVANCE,
  ::THROTTLE_POSITION
};

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

  // Check if data file exists.  If not, write column headers.
  generateConfigFile(configFilename);
  setupDataFile(dataFilename, columnHeaders);

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
    String data = readEcuParams(pidsOfInterest);
    String dataList[] = {data};
    writeToFile(dataFilename, dataList, 1);
  }

  // Check for joystick click and update state.
  if (digitalRead(CLICK) == LOW) {
    isActive = !isActive;
    while (digitalRead(CLICK) == LOW) {} // Hold code in this state until button is released
  }
}

// Check if data file exists.  If not, write column headers to data file.
void setupDataFile(String filename, String columnHeaders) {
  if (!SD.exists(filename)) {
    File dataFile = openFile(filename);

    dataFile.println(columnHeaders);

    dataFile.flush();
    dataFile.close(); // Close data logging file
  }
}

// Create config file with available PIDs.
String generateConfigFile(String filename) {
  String pids = "";
  String pidNames = "";
  for (int pid = 0; pid < 96; pid++) {
    if (OBD2.pidSupported(pid)) {
      pids = pids + (String) pid + ",";
      pidNames = pidNames + OBD2.pidName(pid) + " [" + OBD2.pidUnits(pid) + "],";
    }
  }
  String dataList[] = {pids, pidNames, ""};
  writeToFile(filename, dataList, 3);
}

String readEcuParams(int pids[]) {
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_A, LOW);
  return "";
}

File openFile(String filename) {
  File file = SD.open(filename, FILE_WRITE); // Open uSD file to log data

  // If data file can't be opened, throw error.
  if (!file) {
    Serial.println("Error opening file: ");
    Serial.println(filename);
  }

  return file;
}

void writeToFile(String filename, String lines[], int numLines) {
  digitalWrite(LED_B, HIGH); // Turn on LED_B

  File file = openFile(filename);
  for (int i = 0; i < numLines; i++) {
    file.print(lines[i]);
    file.println();
  }
  file.flush();
  file.close();

  digitalWrite(LED_B, LOW); // Turn off LED_B
}
