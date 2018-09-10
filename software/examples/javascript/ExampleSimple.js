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
        // Get current counter from channel 0
        ic.getCounter(Tinkerforge.BrickletIndustrialCounter.CHANNEL_0,
            function (counter) {
                console.log('Counter (Channel 0): ' + counter);
            },
            function (error) {
                console.log('Error: ' + error);
            }
        );

        // Get current signal data from channel 0
        ic.getSignalData(Tinkerforge.BrickletIndustrialCounter.CHANNEL_0,
            function (dutyCycle, period, frequency, value) {
                console.log('Duty Cycle (Channel 0): ' + dutyCycle/100.0 + ' %');
                console.log('Period (Channel 0): ' + period + ' ns');
                console.log('Frequency (Channel 0): ' + frequency/1000.0 + ' Hz');
                console.log('Value (Channel 0): ' + value);
            },
            function (error) {
                console.log('Error: ' + error);
            }
        );
    }
);

console.log('Press key to exit');
process.stdin.on('data',
    function (data) {
        ipcon.disconnect();
        process.exit(0);
    }
);
