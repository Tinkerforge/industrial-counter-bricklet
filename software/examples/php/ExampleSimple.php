<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletIndustrialCounter.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletIndustrialCounter;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Industrial Counter Bricklet

$ipcon = new IPConnection(); // Create IP connection
$ic = new BrickletIndustrialCounter(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Get current counter from channel 0
$counter = $ic->getCounter(BrickletIndustrialCounter::CHANNEL_0);
echo "Counter (Channel 0): $counter\n";

// Get current signal data from channel 0
$signal_data = $ic->getSignalData(BrickletIndustrialCounter::CHANNEL_0);

echo "Duty Cycle (Channel 0): " . $signal_data['duty_cycle']/100.0 . " %\n";
echo "Period (Channel 0): " . $signal_data['period'] . " ns\n";
echo "Frequency (Channel 0): " . $signal_data['frequency']/1000.0 . " Hz\n";
echo "Value (Channel 0): " . $signal_data['value'] . "\n";

echo "Press key to exit\n";
fgetc(fopen('php://stdin', 'r'));
$ipcon->disconnect();

?>
