"""
receiver's geolocalisation code

Author: Clément Poulin
March 20th, 2026
"""

import serial
import time
import math


def extract_lat_lon(response):
    """
    gets the latitude and longitude from the
    received LoRa transmission
    """
    data = response.split(',')
    if len(data) >= 2:
        lat = data[0].replace("LAT:", "").strip()
        lon = data[1].replace("LON:", "").strip()
        return float(lat), float(lon)
    return None, None


def calculate_haversine_distance(lat1, lon1, lat2, lon2):
    """
    calculates the distance between 2 points
    with the haversine formula
    """
    R = 6371.0    # [km]

    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)

    # haversine formula
    a = math.sin(dphi / 2)**2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlambda / 2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    
    return R * c    # [km]


home_lat, home_lon = 46.80477, -71.234223
chris_lat, chris_lon = 46.7925047, -71.2749168
print(calculate_haversine_distance(home_lat, home_lon, chris_lat, chris_lon))

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
        response = ser.readline().decode('utf-8').strip()
        lat, lon = extract_lat_lon(response)
        haversine_distance = calculate_haversine_distance(home_lat, home_lon, lat, lon)
        print(f"Heltec feedback: {response}")
        print(f"Distance between boards: {haversine_distance}")
except KeyboardInterrupt:
    print("\nUser stop.")
finally:
    ser.close()
    print("Heltec disconnected.")
