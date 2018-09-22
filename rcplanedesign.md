# Design for RC Plane

This document describes the components of the RC plane and how they fit together.

It's a living document - as things change, please keep it up to date.

## List of major components

* Raspberry Pi
  * Tablet
    * connects via VNC to Pi (using vncconnect).
      * On the Pi it runs a browser which it uses to view the browser UI page served by chatConnection.py.
      * Also a shell via which the user runs the Python script (mainGroundControl.py).
  * Joystick - connected via USB to Pi.
* Phone - runs ground-only WiFi hotspot for tablet/Pi comms.
* "UNO" - Arduino Uno
  * connects via USB serial to Pi (Pi is the host).
  * connects via SPI to SX1278 radio chip breakout board (ground).
* "NANO" - Arduino Nano
  * connects via SPI to SX1278 radio chip breakout board (onboard).
  * connects via Servo library to some servos and the ESC for the main prop.

## Raspberry Pi

There is one process running on Pi. It's written in Python (appropriately :-) ).

Files are in Dropbox/Programming/RC Plane/GroundControl/.

It comprises
* main (mainGroundControl.py) - sets up other components, defines the "mainLoop" actions, and starts chat which actually drives the event loop.
* joystick (joystick.py) - uses pygame to read joystick state and return it in appropriate format.
* radio (radio.py) - owns the serial link to the UNO, and allows tx/rx of data to it (which is then passed to the radio).
* chat (chatConnection.py) - runs the (tornado) event loop (which amongst other things calls "mainLoop" periodically), 
  runs a web server and websocket server for UI (which displays telemetry and flight controls in the user's browser 
  and also allows chat, hence the name).
* telemetry (manageData.py) - receives telemetry from downlink and formats it for display; 
  doesn't currently seem to send anything to the uplink but does read joystick etc state for local display.

There are some static files to support the browser UI.



## UNO

The UNO is running an Arduino IDE script: `RC_Ground_Pi_Link_V2` (*not* V3).

Files are in Dropbox/Programming/RC Plane/.

The UNO takes the info it receives via serial connection from the Pi, translates it (reduces byte values by 1 - see separate section for an explanation),
and sends it to the radio.

Also takes the information it receives from the radio, increases byte values by 1, and sends
to Pi over usb-serial.

It also provides a small amount of telemetry about itself (loops/sec). This is provided as additional info on the end of the NANO's telemetry.

The code is currently in a confused state, with a bunch of test code present and two different
kinds of LoRa code, both disabled.

UNO is forbidden from using 0, because when PI subtracts 1 it won't work right.

## NANO

The NANO is running an Arduino IDE script: `RC_Plane_Onboard_Control_V2` (*not* V3).

The NANO code has some commented-out code that uses the radio; instead it is currently set up to use serial (for testing purposes). In production
it will obviously use radio only. The serial version runs fine on an Uno as well,
with the servos connected to the Uno (via breadboard). This is unfortunately
necessary because currently we can't provide enough power to the NANO to test
it with servos.

The telemetry model the NANO presents is that there are a bunch of "ground registers".
These are regularly returned to ground via radio (every 0.5s, configurable).
The telemetry message is a sequence of one or more pairs of bytes: register byte and then value byte.
(See `addTelemetry`).
There is a ground register "CONFLICT" which indicates an error.

Aeroplane control variables - these globals get sent to the servos regularly by the event loop.
They are updated by radio command and also by emergency control.

ESC uses nonstandard servo range, so needs special setup on `attach`.

Main loop does:

* get incoming data and execute it (via `redirectincomingData`)
* if emergency mode on, do emergency actions via `emergencyControlManager`
* write servo state
* do telemetry if required (based on time)

Ground to plane commands: a message is a sequence of one or more pairs of bytes.
Each pair is (register, value).
Commands do various things - most just write to a global (e.g., setting
control surfaces / throttle etc). Some also add specific telemetry info.
Others are not currently implemented.

Telemetry sending - the serial alternative seems to have some inadvertent
offset logic. This should probably not be here.

NANO must not use byte 255 (because that would not work right when 1 is added by UNO).



## Concerns

* On NANO, what happens if we're up for more than MAX_LONG microseconds? (66 minutes)



## Other files not currently in use

There are some other files lying around - don't be distracted.

* no-serial files, out of date by a few months.
* V3 files - didn't complete this; use V2 instead.
* Unversioned ground-control and ground-pi-link files. Old.
* Nothing in the Arduino sketchbook is up to date.
* There is also a file jetcon.py which is not used.


