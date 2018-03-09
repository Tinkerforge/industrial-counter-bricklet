#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "abc1" 

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_industrial_counter import BrickletIndustrialCounter
import time

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    counter = BrickletIndustrialCounter(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected


#    counter.set_counter(counter.PIN_0, )

    counter.set_counter_configuration(counter.PIN_0, 
                                      counter.COUNT_EDGE_RISING,
                                      counter.COUNT_DIRECTION_UP,
                                      counter.DUTY_CYCLE_PRESCALER_1,
                                      counter.FREQUENCY_INTEGRATION_TIME_256_MS)

    counter.set_counter_configuration(counter.PIN_1, 
                                      counter.COUNT_EDGE_RISING,
                                      counter.COUNT_DIRECTION_UP,
                                      counter.DUTY_CYCLE_PRESCALER_1,
                                      counter.FREQUENCY_INTEGRATION_TIME_256_MS)

    counter.set_counter_configuration(counter.PIN_2, 
                                      counter.COUNT_EDGE_RISING,
                                      counter.COUNT_DIRECTION_UP,
                                      counter.DUTY_CYCLE_PRESCALER_1,
                                      counter.FREQUENCY_INTEGRATION_TIME_256_MS)

    counter.set_counter_configuration(counter.PIN_3, 
                                      counter.COUNT_EDGE_RISING,
                                      counter.COUNT_DIRECTION_UP,
                                      counter.DUTY_CYCLE_PRESCALER_1,
                                      counter.FREQUENCY_INTEGRATION_TIME_256_MS)

    while True:
        t = time.time()
        signal_data = counter.get_all_signal_data()
        count = counter.get_all_counter()
        print("Count: {}".format(count))
        print("Signal Data: {}".format(signal_data))
#        print(signal_data.frequency - signal_data.frequency % 10000000)
        time.sleep(0.2 - (time.time()-t))

    raw_input("Press key to exit\n") # Use input() in Python 3
    ipcon.disconnect()
