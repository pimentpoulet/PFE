"""
transmitter's geolocalisation code using the GPS2IP Lite app

Author: Clément Poulin
March 20th, 2026
"""

import socket
import serial
import time

def get_lat_lon(data):

    data = list(data.split(','))

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
    gets the computer's geolocalisation and sends it
    to a connected esp32 board so that it can be sent
    via LoRa to the receiver module
    """
    message = f"LAT:{lat},LON:{lon}\n"

    print(f"Envoi au Heltec : {message.strip()}")
    serial_port.write(message.encode('utf-8'))
    time.sleep(1)

    # check esp32 response
    while serial_port.in_waiting > 0:
        reponse = serial_port.readline().decode('utf-8').strip()
        print(f"Heltec feedback: {reponse}")


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

# Iphone connection
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((UDP_IP, UDP_PORT))

try:
    while True:
        data = sock.recv(1024).decode('utf-8')
        for line in data.split('\r\n'):
            if "$GPRMC" in line and ",A," in line:
                lat, lon = get_lat_lon(line)
                if lat is not None:
                    send_location(lat, lon, ser)
except KeyboardInterrupt:
    print("\nUser stop.")
finally:
    sock.close()
    ser.close()
    print("Heltec disconnected.")
