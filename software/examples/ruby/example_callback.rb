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

# Register all counter callback
ic.register_callback(BrickletIndustrialCounter::CALLBACK_ALL_COUNTER) do |counter|
  puts "Counter (Channel 0): #{counter[0]}"
  puts "Counter (Channel 1): #{counter[1]}"
  puts "Counter (Channel 2): #{counter[2]}"
  puts "Counter (Channel 3): #{counter[3]}"
  puts ''
end

# Set period for all counter callback to 1s (1000ms)
ic.set_all_counter_callback_configuration 1000, true

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
