"""
transmitter's geolocalisation code using the GPS2IP Lite app

Author: Clément Poulin
March 20th, 2026
Last modified April 2th, 2026
"""

import socket
import serial
import time
import pandas as pd
import os


def get_lat_lon(data):
    """
    gets the latitude and longitude from the GPS2IP Lite app
    """
    data = data.split(',')
    if len(data) < 7 or not data[3] or not data[5]:
        return None, None

    # latitude
    lat_data = data[3]
    lat = float(lat_data[:2]) + (float(lat_data[2:]) / 60)

    # longitude
    lon_data = data[5]
    lon = float(lon_data[:3]) + (float(lon_data[3:]) / 60)

    # signs
    lat_dir, lon_dir = data[4], data[6]
    match (lat_dir, lon_dir):
        case ('N', 'E'): return lat, lon
        case ('N', 'W'): return lat, -lon
        case ('S', 'E'): return -lat, lon
        case ('S', 'W'): return -lat, -lon


def send_location(lat, lon, serial_port):
    """
    sends localisation to a connected esp32 board so
    that it can be sent via LoRa to the receiver module
    """
    message = f"LAT:{lat},LON:{lon}\n"

    print(f"\nEnvoi au Heltec : {message.strip()}")
    serial_port.write(message.encode('utf-8'))
    time.sleep(1)

    # check esp32 response
    while serial_port.in_waiting > 0:
        reponse = serial_port.readline().decode('utf-8').strip()
        print(f"Heltec feedback: {reponse}")

    time.sleep(2)


# Iphone data
UDP_IP = "172.20.10.1"
UDP_PORT = 11123

try:
    ser = serial.Serial('COM5', 115200, timeout=1)
    time.sleep(2)
    print("Heltec connected.")
except Exception as e:
    print(f"Error: {e}")
    exit()

cols = ['lat', 'lon']
df = pd.DataFrame(columns=cols)
file_path = '2026_03_04_distance_home_ulaval.csv'
file_exists = os.path.isfile(file_path)

# Iphone connection
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((UDP_IP, UDP_PORT))

try:
    while True:
        data = sock.recv(1024).decode('utf-8')
        for line in data.split('\r\n'):
            if "$GPRMC" in line and ",A," in line:
                lat, lon = None, None
                try:
                    lat, lon = get_lat_lon(line)
                except ValueError:
                    pass

                print(lat, lon)
                new_data = pd.DataFrame([[lat, lon]], columns=cols)
                new_data.to_csv(file_path, mode='a', index=False, header=not file_exists)
                file_exists = True
                if lat is not None:
                    send_location(lat, lon, ser)

except KeyboardInterrupt:
    print("\nUser stop.")
finally:
    sock.close()
    ser.close()
    print("Heltec disconnected.")
