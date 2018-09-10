#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletIndustrialCounter;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your Industrial Counter Bricklet

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $ic = Tinkerforge::BrickletIndustrialCounter->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Get current counter from channel 0
my $counter = $ic->get_counter($ic->CHANNEL_0);
print "Counter (Channel 0): $counter\n";

# Get current signal data from channel 0
my ($duty_cycle, $period, $frequency, $value) = $ic->get_signal_data($ic->CHANNEL_0);

print "Duty Cycle (Channel 0): " . $duty_cycle/100.0 . " %\n";
print "Period (Channel 0): $period ns\n";
print "Frequency (Channel 0): " . $frequency/1000.0 . " Hz\n";
print "Value (Channel 0): $value\n";

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
