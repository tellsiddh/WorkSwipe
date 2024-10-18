import evdev
import os
import time
from datetime import datetime

# Constants
MAX_CARDHOLDER_NAME_LEN = 50
ATTENDANCE_LOG_FILE = "attendance_log.csv"
MAX_CARD_DATA_LEN = 256

# Cardholder data (list of dictionaries)
cardholders = [
    {"card_data": "1226137070", "name": "Demo Person"},
    {"card_data": "1229499663", "name": "Cyriac"},
    {"card_data": "1229662137", "name": "Jay"},
    {"card_data": "1229454098", "name": "Shubham"},
    {"card_data": "1233706138", "name": "Chinmay"},
    {"card_data": "1222313196", "name": "Riyank"},
    {"card_data": "1229700331", "name": "Ajay"},
    {"card_data": "1229582226", "name": "Prashant"},
    {"card_data": "1229560633", "name": "Narain"},
    {"card_data": "1229581862", "name": "Sujith"},
    {"card_data": "1229544344", "name": "Aditya"},
    {"card_data": "1229714371", "name": "Vikram"},
    {"card_data": "1229624957", "name": "Ankit"},
    {"card_data": "1228930549", "name": "Shrey"}
]

# Ensure the log file exists and has the proper headers
def ensure_log_file():
    if not os.path.exists(ATTENDANCE_LOG_FILE):
        with open(ATTENDANCE_LOG_FILE, "w") as file:
            file.write("Timestamp,Cardholder,Card Data,Type\n")

# Log attendance data to the log file
def log_attendance(card_data):
    current_timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    cardholder = "Unknown Cardholder"

    # Find the cardholder based on card_data
    found = False
    for holder in cardholders:
        if card_data == holder["card_data"]:
            cardholder = holder["name"]
            found = True
            break

    if not found:
        with open(ATTENDANCE_LOG_FILE, "a") as file:
            file.write(f"{current_timestamp},{cardholder},{card_data},entry\n")
        print(f"Unknown cardholder swipe recorded for card data: {card_data} at {current_timestamp}")
        return

    # Check the last swipe for the same cardholder
    last_swipe_time = None
    last_swipe_type = ""

    with open(ATTENDANCE_LOG_FILE, "r") as file:
        for line in file:
            log_timestamp, log_cardholder, log_card_data, log_type = line.strip().split(',')
            if log_card_data == card_data and log_cardholder == cardholder:
                last_swipe_time = datetime.strptime(log_timestamp, "%Y-%m-%d %H:%M:%S")
                last_swipe_type = log_type

    current_time = datetime.now()

    # Determine the swipe type (entry/exit)
    swipe_type = "entry"
    if last_swipe_time and last_swipe_time.date() == current_time.date():
        swipe_type = "exit" if last_swipe_type == "entry" else "entry"

    # Log the new swipe
    with open(ATTENDANCE_LOG_FILE, "a") as file:
        file.write(f"{current_timestamp},{cardholder},{card_data},{swipe_type}\n")
    print(f"{swipe_type} recorded for {cardholder} at {current_timestamp}")

# Main function to capture input from the device
def capture_input():
    ensure_log_file()

    # Replace this path with your actual device path
    device_path = "/dev/input/by-id/usb-0acd_0200-event-kbd"
    
    try:
        dev = evdev.InputDevice(device_path)
    except FileNotFoundError:
        print(f"Error: Device {device_path} not found.")
        return

    print(f"Input device name: \"{dev.name}\"")
    print("Reading card swipes...")

    card_data = ""
    for event in dev.read_loop():
        if event.type == evdev.ecodes.EV_KEY and event.value == 1:  # Key press event
            key_event = evdev.categorize(event)
            if key_event.keycode.startswith("KEY_"):
                key = key_event.keycode.replace("KEY_", "")
                if key.isdigit():
                    card_data += key
                elif key == "ENTER":
                    log_attendance(card_data)
                    card_data = ""  # Reset card data for the next swipe
        time.sleep(0.01)

if __name__ == "__main__":
    capture_input()
