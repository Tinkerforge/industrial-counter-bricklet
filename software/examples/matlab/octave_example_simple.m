function octave_example_simple()
    more off;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your Industrial Counter Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    ic = javaObject("com.tinkerforge.BrickletIndustrialCounter", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Get current counter from channel 0
    counter = ic.getCounter(ic.CHANNEL_0);
    fprintf("Counter (Channel 0): %d\n", java2int(counter));

    % Get current signal data from channel 0
    signalData = ic.getSignalData(ic.CHANNEL_0);

    fprintf("Duty Cycle (Channel 0): %g %%\n", signalData.dutyCycle/100.0);
    fprintf("Period (Channel 0): %d ns\n", java2int(signalData.period));
    fprintf("Frequency (Channel 0): %g Hz\n", java2int(signalData.frequency)/1000.0);
    fprintf("Value (Channel 0): %d\n", signalData.value);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

function int = java2int(value)
    if compare_versions(version(), "3.8", "<=")
        int = value.intValue();
    else
        int = value;
    end
end
