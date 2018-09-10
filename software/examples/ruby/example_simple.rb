#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_industrial_counter'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your Industrial Counter Bricklet

ipcon = IPConnection.new # Create IP connection
ic = BrickletIndustrialCounter.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Get current counter from channel 0
counter = ic.get_counter BrickletIndustrialCounter::CHANNEL_0
puts "Counter (Channel 0): #{counter}"

# Get current signal data from channel 0 as [duty_cycle, period, frequency, value]
signal_data = ic.get_signal_data BrickletIndustrialCounter::CHANNEL_0

puts "Duty Cycle (Channel 0): #{signal_data[0]/100.0} %"
puts "Period (Channel 0): #{signal_data[1]} ns"
puts "Frequency (Channel 0): #{signal_data[2]/1000.0} Hz"
puts "Value (Channel 0): #{signal_data[3]}"

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
