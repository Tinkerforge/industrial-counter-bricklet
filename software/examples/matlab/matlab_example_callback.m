function matlab_example_callback()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletIndustrialCounter;

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your Industrial Counter Bricklet

    ipcon = IPConnection(); % Create IP connection
    ic = handle(BrickletIndustrialCounter(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register all counter callback to function cb_all_counter
    set(ic, 'AllCounterCallback', @(h, e) cb_all_counter(e));

    % Set period for all counter callback to 1s (1000ms)
    ic.setAllCounterCallbackConfiguration(1000, true);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end

% Callback function for all counter callback
function cb_all_counter(e)
    fprintf('Counter (Channel 0): %i\n', e.counter(1));
    fprintf('Counter (Channel 1): %i\n', e.counter(2));
    fprintf('Counter (Channel 2): %i\n', e.counter(3));
    fprintf('Counter (Channel 3): %i\n', e.counter(4));
    fprintf('\n');
end
