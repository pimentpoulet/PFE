"""
receiver's geolocalisation code

Author: Clément Poulin
March 20th, 2026
"""

import serial
import time

home_lat, home_lon = 46.80477, -71.234223
chris_lat, chris_lon = 36.175, -115.1372

try:
    ser = serial.Serial('COM6', 115200, timeout=1)
    time.sleep(2)
    print("Heltec connected.")
except Exception as e:
    print(f"Error: {e}")
    exit()

# check esp32 response
try:
    while True:
        reponse = ser.readline().decode('utf-8').strip()
        print(f"Heltec feedback: {reponse}")
except KeyboardInterrupt:
    print("\nUser stop.")
finally:
    ser.close()
    print("Heltec disconnected.")
