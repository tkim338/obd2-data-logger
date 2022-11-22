# obd2-data-logger
Log ECU data using an Arduino, CAN bus shield, and ODB2 port

## Hardware
1) Arduino UNO
2) SparkFun CAN-BUS shield
3) ODB2-DB9 cable*
4) micro-SD card

*cable pinout*:
| pin description | OBD-II | DB9 |
|--|--|--|
J1850 BUS+ | 2 | 7
Chassis Ground | 4 | 2
Signal Ground | 5 | 1
CAN High J-2284 | 6 | 3
ISO 9141-2 K Line | 7 | 4
J1850 BUS- | 10 | 6
CAN Low J-2284 | 14 | 5
ISO 9141-2 L Line | 15 | 8
Battery Power | 16 | 9

## Operation
Click the CAN-BUS shield's built-in joystick to switch states between `logging` and `idle`.  There are three LEDs on the SparkFun CAN-BUS shield: the power LED and two optional LEDs.  The power LED is always on.  When the code is in the `logging` state, the two optional LEDs flash between on and off.  When the code is in the `idle` state, neither of the optional LEDs will turn on.