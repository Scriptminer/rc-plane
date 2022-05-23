// This is an autogenerated file, see generate.py for edits. //

#define reg_setAilerons 0
#define reg_setElevator 1
#define reg_setRudder 2
#define reg_setThrottle 3
#define reg_setDropDoor 4
#define reg_setAutopilotState 5
#define reg_requestTelemetry 6
#define reg_currentRoll 64
#define reg_currentPitch 65
#define reg_currentYaw 66
#define reg_reportedDropDoorState 67
#define reg_onboardLoopSpeed 68 // Calculated hectomicroseconds (0.1ms units) each loop takes on average.
#define reg_reportedControlState 69
#define reg_onboardRSSI 70 // Received signal strength (as a positive value, not negative)
#define reg_currentBattVoltage 71 // Voltage of battery (raw reading - processing to occur on PI)
#define reg_onboardCorruptedMessages 72
#define reg_onboardError 73 // 
#define reg_onboardPacketReceiveRate 74 // Number of packets received since last request
#define reg_groundLoopSpeed 128 // Milliseconds each loop takes on average.
#define reg_groundRSSI 129
#define reg_groundRadioStarted 130 // 0 for success, 1 for failure
#define reg_aileronTrimPos 131
#define reg_elevatorTrimPos 132
#define reg_rudderTrimPos 133
#define reg_groundPacketSendRate 134 // Incremented only when transmitted packet is >0 bytes.
#define reg_testChannel1 192
#define reg_testChannel2 193
#define reg_testChannel3 194
#define reg_testChannel4 195
#define reg_DO_NOT_USE 255

const int batteryVoltageReadingOffset = 550;
const int maxRadioMessageLength = 32; // Bytes
const int unlockDoorSignal = 100; // Signal to send from ground to unlock door.
const int lockDoorSignal = 200; // Signal to send from ground to lock door.
const long groundToAirFrequency = 459062500.0;
const long groundToAirBandwidth = 125000.0;
const long airToGroundFrequency = 434440000.0;
const long airToGroundBandwidth = 125000.0;
const int groundToAirTxPower = 20;
const int airToGroundTxPower = 10;
const int airTelemetryInterval = 500; // Milliseconds between requests for telemetry from plane.
const int groundTelemetryInterval = 250; // Milliseconds between sending Ground-Pi telemetry.
const int conflictingDropDoorMessageERR = 0; // Two messages sent to the drop door were both different
const int conflictingControlStateMessageERR = 1; // Two messages sent to change the control state were both different