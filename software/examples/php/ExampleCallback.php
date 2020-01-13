<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletIndustrialCounter.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletIndustrialCounter;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Industrial Counter Bricklet

// Callback function for all counter callback
function cb_allCounter($counter)
{
    echo "Counter (Channel 0): $counter\n";
    echo "Counter (Channel 1): $counter\n";
    echo "Counter (Channel 2): $counter\n";
    echo "Counter (Channel 3): $counter\n";
    echo "\n";
}

$ipcon = new IPConnection(); // Create IP connection
$ic = new BrickletIndustrialCounter(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Register all counter callback to function cb_allCounter
$ic->registerCallback(BrickletIndustrialCounter::CALLBACK_ALL_COUNTER, 'cb_allCounter');

// Set period for all counter callback to 1s (1000ms)
$ic->setAllCounterCallbackConfiguration(1000, TRUE);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
