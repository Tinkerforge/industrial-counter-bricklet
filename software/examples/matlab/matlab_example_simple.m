function matlab_example_simple()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletIndustrialCounter;

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your Industrial Counter Bricklet

    ipcon = IPConnection(); % Create IP connection
    ic = handle(BrickletIndustrialCounter(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Get current counter from channel 0
    counter = ic.getCounter(BrickletIndustrialCounter.CHANNEL_0);
    fprintf('Counter (Channel 0): %i\n', counter);

    % Get current signal data from channel 0
    signalData = ic.getSignalData(BrickletIndustrialCounter.CHANNEL_0);

    fprintf('Duty Cycle (Channel 0): %g %%\n', signalData.dutyCycle/100.0);
    fprintf('Period (Channel 0): %i ns\n', signalData.period);
    fprintf('Frequency (Channel 0): %g Hz\n', signalData.frequency/1000.0);
    fprintf('Value (Channel 0): %i\n', signalData.value);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end
