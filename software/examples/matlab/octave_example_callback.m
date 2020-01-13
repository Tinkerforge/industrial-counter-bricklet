function octave_example_callback()
    more off;

    HOST = "localhost";
    PORT = 4223;
    UID = "XYZ"; % Change XYZ to the UID of your Industrial Counter Bricklet

    ipcon = javaObject("com.tinkerforge.IPConnection"); % Create IP connection
    ic = javaObject("com.tinkerforge.BrickletIndustrialCounter", UID, ipcon); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register all counter callback to function cb_all_counter
    ic.addAllCounterCallback(@cb_all_counter);

    % Set period for all counter callback to 1s (1000ms)
    ic.setAllCounterCallbackConfiguration(1000, true);

    input("Press key to exit\n", "s");
    ipcon.disconnect();
end

% Callback function for all counter callback
function cb_all_counter(e)
    fprintf("Counter (Channel 0): %d\n", java2int(e.counter(1)));
    fprintf("Counter (Channel 1): %d\n", java2int(e.counter(2)));
    fprintf("Counter (Channel 2): %d\n", java2int(e.counter(3)));
    fprintf("Counter (Channel 3): %d\n", java2int(e.counter(4)));
    fprintf("\n");
end

function int = java2int(value)
    if compare_versions(version(), "3.8", "<=")
        int = value.intValue();
    else
        int = value;
    end
end
