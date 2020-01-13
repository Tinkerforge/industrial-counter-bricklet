#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Industrial Counter Bricklet

# Handle incoming all counter callbacks
tinkerforge dispatch industrial-counter-bricklet $uid all-counter &

# Set period for all counter callback to 1s (1000ms)
tinkerforge call industrial-counter-bricklet $uid set-all-counter-callback-configuration 1000 true

echo "Press key to exit"; read dummy

kill -- -$$ # Stop callback dispatch in background
