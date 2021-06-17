
#define groundregPING  1
#define groundregROLL  2
#define groundregPITCH  3
#define groundregYAW  4
#define groundregDROP  5 // Returns drop door state
#define groundregLOOP  6 // Loop time register (avg loop)
#define groundregAUTOMODE  7 // Autopilot state
#define groundregCONFLICT  8 // Conflicting incoming signal warning. #bit1 Drop door, #bit2 control state toggle, #bit3 loops per second out of range. #bit4 out of range int submitted. #bit5 received message of wrong length
#define groundregTESTING  9 // For testing only
#define groundregRSSI  10 // Returns the received signal strength (as a positive value, not negative)
#define groundregBATT_VOLTAGE 11 // Voltage of battery (raw reading - processing will occur on PI)

const int batteryVoltageReadingOffset = 600; // Provisional number!!

const int maxRadioMessageLength = 32; // Any radio packages (either way) should not be longer than this
