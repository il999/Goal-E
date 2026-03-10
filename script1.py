import serial
import time
from pynput import keyboard

# ⚠️ REPLACE THIS with your actual Arduino port from the Arduino IDE
ARDUINO_PORT = '/dev/cu.usbmodem2101'
BAUD_RATE = 9600

# Connect to the Arduino
try:
    ser = serial.Serial(ARDUINO_PORT, BAUD_RATE)
    time.sleep(2)  # Give Arduino a moment to reset after connection
    print(f"Connected to Arduino on {ARDUINO_PORT}")
except Exception as e:
    print(f"Failed to connect: {e}")
    print("Make sure the Arduino IDE Serial Monitor is CLOSED!")
    exit()

print("\n🚗 Ready to drive! Use Arrow Keys (including diagonals). Press 'Esc' to quit.")

# 1. Track the state of the keys we care about
keys_pressed = {
    keyboard.Key.up: False,
    keyboard.Key.down: False,
    keyboard.Key.left: False,
    keyboard.Key.right: False
}

# Keep track of the last sent command so we don't spam the Arduino
last_command = None


def update_movement():
    """Calculates the current direction based on held keys and sends to Arduino."""
    global last_command

    up = keys_pressed.get(keyboard.Key.up, False)
    down = keys_pressed.get(keyboard.Key.down, False)
    left = keys_pressed.get(keyboard.Key.left, False)
    right = keys_pressed.get(keyboard.Key.right, False)

    command = b'S'  # Default to stop
    action_name = "Stop"

    # 2. Check for Diagonals first
    if up and right:
        command = b'E'
        action_name = "Forward-Right"
    elif up and left:
        command = b'Q'
        action_name = "Forward-Left"
    elif down and right:
        command = b'C'
        action_name = "Backward-Right"
    elif down and left:
        command = b'Z'
        action_name = "Backward-Left"

    # 3. Check for straight lines (ignoring conflicting keys like Up+Down)
    elif up and not down:
        command = b'F'
        action_name = "Forward"
    elif down and not up:
        command = b'B'
        action_name = "Backward"
    elif left and not right:
        command = b'L'
        action_name = "Left"
    elif right and not left:
        command = b'R'
        action_name = "Right"

    # 4. Only send the command if the movement actually changed
    if command != last_command:
        ser.write(command)
        print(action_name)
        last_command = command


def on_press(key):
    if key in keys_pressed:
        keys_pressed[key] = True
        update_movement()


def on_release(key):
    if key == keyboard.Key.esc:
        print("Exiting...")
        return False  # Stop the listener

    if key in keys_pressed:
        keys_pressed[key] = False
        update_movement()




\





















































# Start listening to the keyboard
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()

# Ensure motors stop when the script exits
ser.write(b'S')
ser.close()