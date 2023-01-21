#include <SPI.h> // dependency of SD.h
#include <SD.h>
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

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

// Names of files to write data to
char dataFilename[] = "passive_data.csv";

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
  if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
    Serial.println("CAN Init ok");
  else
    Serial.println("Can't init CAN");

  // Check if uSD card initialized
  if (!SD.begin(chipSelect)) {
    Serial.println(F("uSD card failed to initialize, or is not present"));
    return;
  }
  else {
    Serial.println(F("uSD card initialized"));
    delay(100);
  }

  // Set initial state of logger
  isActive = true;

  // Turn off LEDs
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
}

//********************************Main Loop*********************************//
void loop() {
  if (isActive) {
    // Read row of data from OBD2
    digitalWrite(LED_A, HIGH);
    digitalWrite(LED_B, LOW);

    tCAN message;
    if (mcp2515_check_message()) {
      if (mcp2515_get_message(&message)) {
      //if(message.id == 0x620 and message.data[2] == 0xFF) { //uncomment when you want to filter
        digitalWrite(LED_A, LOW);
        digitalWrite(LED_B, HIGH);
      
        File dataFile = SD.open(dataFilename, FILE_WRITE);
        
        dataFile.print(F("ID: "));
        dataFile.print(message.id, HEX);
        dataFile.print(F(", "));
        dataFile.print(F("Data: "));
        dataFile.print(message.header.length, DEC);
        for(int i=0;i<message.header.length;i++) { 
          dataFile.print(message.data[i], HEX);
          dataFile.print(F(" "));
        }
        dataFile.println();
        dataFile.flush();
        dataFile.close();
      //}
      }
    }
  }
  
  // Check for joystick click and update state.
  if (digitalRead(CLICK) == LOW) {
    isActive = !isActive;
    while (digitalRead(CLICK) == LOW) {} // Hold code in this state until button is released
  }
}
