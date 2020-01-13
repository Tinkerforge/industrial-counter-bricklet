#!/usr/bin/perl

use strict;
use Tinkerforge::IPConnection;
use Tinkerforge::BrickletIndustrialCounter;

use constant HOST => 'localhost';
use constant PORT => 4223;
use constant UID => 'XYZ'; # Change XYZ to the UID of your Industrial Counter Bricklet

# Callback subroutine for all counter callback
sub cb_all_counter
{
    my ($counter) = @_;

    print "Counter (Channel 0): " . @{$counter}[0] . "\n";
    print "Counter (Channel 1): " . @{$counter}[1] . "\n";
    print "Counter (Channel 2): " . @{$counter}[2] . "\n";
    print "Counter (Channel 3): " . @{$counter}[3] . "\n";
    print "\n";
}

my $ipcon = Tinkerforge::IPConnection->new(); # Create IP connection
my $ic = Tinkerforge::BrickletIndustrialCounter->new(&UID, $ipcon); # Create device object

$ipcon->connect(&HOST, &PORT); # Connect to brickd
# Don't use device before ipcon is connected

# Register all counter callback to subroutine cb_all_counter
$ic->register_callback($ic->CALLBACK_ALL_COUNTER, 'cb_all_counter');

# Set period for all counter callback to 1s (1000ms)
$ic->set_all_counter_callback_configuration(1000, 1);

print "Press key to exit\n";
<STDIN>;
$ipcon->disconnect();
