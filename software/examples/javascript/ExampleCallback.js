var Tinkerforge = require('tinkerforge');

var HOST = 'localhost';
var PORT = 4223;
var UID = 'XYZ'; // Change XYZ to the UID of your Industrial Counter Bricklet

var ipcon = new Tinkerforge.IPConnection(); // Create IP connection
var ic = new Tinkerforge.BrickletIndustrialCounter(UID, ipcon); // Create device object

ipcon.connect(HOST, PORT,
    function (error) {
        console.log('Error: ' + error);
    }
); // Connect to brickd
// Don't use device before ipcon is connected

ipcon.on(Tinkerforge.IPConnection.CALLBACK_CONNECTED,
    function (connectReason) {
        // Set period for all counter callback to 1s (1000ms)
        ic.setAllCounterCallbackConfiguration(1000, true);
    }
);

// Register all counter callback
ic.on(Tinkerforge.BrickletIndustrialCounter.CALLBACK_ALL_COUNTER,
    // Callback function for all counter callback
    function (counter) {
        console.log('Counter (Channel 0): ' + counter[0]);
        console.log('Counter (Channel 1): ' + counter[1]);
        console.log('Counter (Channel 2): ' + counter[2]);
        console.log('Counter (Channel 3): ' + counter[3]);
        console.log();
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
