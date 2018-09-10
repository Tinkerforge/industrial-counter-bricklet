#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Industrial Counter Bricklet

# Get current counter from channel 0
tinkerforge call industrial-counter-bricklet $uid get-counter channel-0

# Get current signal data from channel 0
tinkerforge call industrial-counter-bricklet $uid get-signal-data channel-0
