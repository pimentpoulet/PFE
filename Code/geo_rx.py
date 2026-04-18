"""
Receiver's geolocalisation and datalogger code (Mobile unit)
Reads GPS from iPhone and listens for LoRa RSSI/SNR from ESP32.

Author: Clément Poulin
March 20th, 2026
Last modified April 2th, 2026
"""

import socket
import serial
import time
import pandas as pd
import os
import threading


def get_lat_lon(data):
    """
    gets the latitude and longitude from the GPS2IP Lite app
    """
    data = data.split(',')
    if len(data) < 7 or not data[3] or not data[5]:
        return None, None

    lat_data = data[3]
    lat = float(lat_data[:2]) + (float(lat_data[2:]) / 60)

    lon_data = data[5]
    lon = float(lon_data[:3]) + (float(lon_data[3:]) / 60)

    lat_dir, lon_dir = data[4], data[6]
    match (lat_dir, lon_dir):
        case ('N', 'E'): return lat, lon
        case ('N', 'W'): return lat, -lon
        case ('S', 'E'): return -lat, lon
        case ('S', 'W'): return -lat, -lon


def extract_RSSI_SNR(response):
    """
    Extracts RSSI and SNR from the Arduino DEBUG line
    """
    if "DEBUG:" in response:
        try:
            parts = response.split('|')
            rssi_part = parts[0].split('RSSI:')[1].split('dBm')[0].strip()
            rssi = float(rssi_part)

            snr_part = parts[1].split('SNR:')[1].split('dB')[0].strip()
            snr = float(snr_part)
            return rssi, snr
        except (IndexError, ValueError):
            return None, None
    return None, None


# Variables globales pour stocker le dernier signal LoRa
current_rssi = None
current_snr = None


def read_serial_data(ser):
    """
    Tâche de fond pour lire les données LoRa entrantes en continu
    """
    global current_rssi, current_snr
    while True:
        try:
            if ser.in_waiting > 0:
                response = ser.readline().decode('utf-8', errors='ignore').strip()
                if response:
                    print(f"Heltec: {response}")
                    rssi, snr = extract_RSSI_SNR(response)
                    if rssi is not None:
                        current_rssi = rssi
                        current_snr = snr
        except Exception as e:
            pass
        time.sleep(0.01)


# Configuration Iphone
UDP_IP = "172.20.10.1"
UDP_PORT = 11123

# Initialisation du port série
try:
    ser = serial.Serial('COM5', 115200, timeout=1)
    time.sleep(2)
    print("Heltec connected.")

    # Lancement du Thread pour écouter le port série sans bloquer le GPS
    serial_thread = threading.Thread(target=read_serial_data, args=(ser,), daemon=True)
    serial_thread.start()
except Exception as e:
    print(f"Error: {e}")
    exit()


file_path = '2026_04_17_distance_test_ile.csv'
file_exists = os.path.isfile(file_path)
cols = ['lat', 'lon', 'rssi', 'snr']

# Connexion Iphone
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((UDP_IP, UDP_PORT))

print("Début de l'enregistrement...")

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

                if lat is not None and lon is not None:
                    print(f"GPS: {lat:.5f}, {lon:.5f} | Signal: RSSI {current_rssi}, SNR {current_snr}")

                    new_data = pd.DataFrame([[lat, lon, current_rssi, current_snr]], columns=cols)
                    new_data.to_csv(file_path, mode='a', index=False, header=not file_exists)
                    file_exists = True

                    # On efface les données LoRa une fois inscrites. 
                    # Si on ne reçoit plus de ping, les prochaines lignes auront des cases vides (None)
                    current_rssi = None
                    current_snr = None

except KeyboardInterrupt:
    print("\nUser stop.")
finally:
    sock.close()
    ser.close()
    print("Heltec disconnected.")
