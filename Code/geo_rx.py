"""
receiver's geolocalisation code

Author: Clément Poulin
March 20th, 2026
"""

import serial
import time

home_lat, homee_lon = 46.80477, -71.234223

ser = serial.Serial('COM5', 115200, timeout=1)

# check esp32 response
time.sleep(2)
while ser.in_waiting > 0:
    reponse = ser.readline().decode('utf-8').strip()
    print(f"Heltec feedback: {reponse}")

ser.close()
