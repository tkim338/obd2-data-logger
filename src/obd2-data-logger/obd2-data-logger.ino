#include <SPI.h> // dependency of SD.h
#include <SD.h>
#include <CAN.h> // dependency of OBD2.h
#include <OBD2.h>

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

bool isActive; // State of logging activity

int pidsOfInterest[] = {
  ::RUN_TIME_SINCE_ENGINE_START,
  ::CALCULATED_ENGINE_LOAD,
  ::ENGINE_COOLANT_TEMPERATURE,
  ::SHORT_TERM_FUEL_TRIM_BANK_1,
  ::LONG_TERM_FUEL_TRIM_BANK_1,
  ::INTAKE_MANIFOLD_ABSOLUTE_PRESSURE,
  ::ENGINE_RPM,
  ::VEHICLE_SPEED,
  ::TIMING_ADVANCE,
  ::AIR_INTAKE_TEMPERATURE,
  ::THROTTLE_POSITION,
  ::OXYGEN_SENSOR_2_SHORT_TERM_FUEL_TRIM,
  ::FUEL_RAIL_GAUGE_PRESSURE,
  ::FUEL_TANK_LEVEL_INPUT,
  ::ABSOLULTE_BAROMETRIC_PRESSURE,
  ::CATALYST_TEMPERATURE_BANK_1_SENSOR_1,
};
int numCols = 16;
//const int numCols = sizeof(pidsOfInterest);
float dataRow[16];
//float dataRow[sizeof(pidsOfInterest)];

// Names of files to write config info and data to
char configFilename[] = "config.txt";
char dataFilename[] = "data_000.csv";

//********************************Setup Loop*********************************//
void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(9600);
  Serial.println(F("*** ECU data logger initializing ***"));

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
    Serial.println(F("CAN init successful"));
    delay(100);
  }
  else {
    Serial.println(F("CAN init failed"));
    return;
  }

  // Check if uSD card initialized
  if (!SD.begin(chipSelect)) {
    Serial.println(F("uSD card failed to initialize, or is not present"));
    return;
  }
  else {
    Serial.println(F("uSD card initialized"));
    delay(100);
  }

  // Record all supported PIDs (up to 96) to config file
  File configFile = SD.open(configFilename, FILE_WRITE);
  char configFileRow [100];
  for (int pid = 0; pid < 96; pid++) {
    if (OBD2.pidSupported(pid)) {
      sprintf(configFileRow, "%i,%s,%s", pid, OBD2.pidName(pid), OBD2.pidUnits(pid));
      configFile.println(configFileRow);
    }
  }
  configFile.flush();
  configFile.close();

  // Initialize data file: find next num of data file, create file, and write column headers to first line.
  int n = 0;
  while (SD.exists(dataFilename)) {
    n += 1;
    sprintf(dataFilename, "data_%03i.csv", n);
  }
  File dataFile = SD.open(dataFilename, FILE_WRITE);
  for (int i = 0; i < numCols; i++) {
    dataFile.print(OBD2.pidName(pidsOfInterest[i]) + "[" + OBD2.pidUnits(pidsOfInterest[i]) + "],");
  }
  dataFile.println();
  dataFile.flush();
  dataFile.close();

  // Set initial state of logger
  isActive = false;

  // Turn off LEDs
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
}

//********************************Main Loop*********************************//
void loop() {
  if (isActive) {
    Serial.println(F("Logging is active. Click to stop logging."));
  }
  else {
    Serial.println(F("Idle. Click to start logging."));
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
