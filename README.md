
# Overview
There are two core components, the Arduino UNO (UNO) to transmit control signals to the plane via LoRa radio, and the Arduino NANO (NANO) which reads the control signals and writes these values to the onboard servos / controls.
Optionally, a computer such as the Raspberry Pi (Pi) can be connected via USB serial to the UNO to run a python script which interprets and presents the inbound telemetry on a webpage it locally hosts using tornado.

# Arduino UNO
Runs RC_Full_Ground_Control.ino, which reads raw joystick and button inputs and converts to register-value pairs, which are sent over a LoRa radio link to the NANO.
The UNO periodically appends to these packets sent to the NANO a request for telemetry and listens for a response packet.
The UNO also periodically sends any inbound radio packets from the NANO, along with the positions and trim of the joysticks and it's own telemetry over serial, to be received by the Pi.

# NANO
Runs RC_Plane_Onboard_Control.ino, which continually checks for incoming LoRa radio packets, interprets them, and adjust servos and controls accordingly.
Also responds to requests for telemetry (i.e. onboard sensors, loop speed, packets received, number of corrupted packets, received signal strength etc.).
Also has an emergency cutoff mode, where if no signal has been received for a given period, it turns the throttle to 0% and returns the servos to their central positions.

# Pi

# Radio Protocol
Radio packets between the NANO and the UNO consist of a series of bytes in the pattern: register byte, value byte, register byte, value byte etc...
The LoRa library handles packet termination.
Registers are defined in generate.py, which in turn generates the CommonConstants.h and CommonConstants.py files.
* Registers 0-63 should be used for commands sent from the ground (either UNO or Pi) to the NANO.
* Registers 64-127 should be used for telemetry from the NANO.
* Registers 128-191 should be used for telemetry from the UNO.
* Registers 192-254 should be used for any testing.
* Register 255 should not be used, as if this appears as the first byte in a packet sent via serial from the UNO to the Pi, the Pi will misread the EOL and read corrupted messages from the UNO.

# Serial Protocol
Serial packets between the UNO and the Pi use the same protocol and registers as the radio link between the UNO and the NANO.
The EOL marker is two 255 bytes in a row at the end of a message. This must be followed by the first byte of the next message (which, being a register byte, cannot be 255) before the Pi actually reads the message. Thus, 255 can still appear as a value byte and not interfere with the EOL.
