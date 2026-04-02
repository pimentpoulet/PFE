"""
receiver's geolocalisation code

Author: Clément Poulin
March 20th, 2026
"""

import pandas as pd
import serial
import time
import math


def extract_lat_lon(response):
    """
    gets the latitude and longitude from the
    received LoRa transmission
    """
    if "Coordonnées Reçues :" in response:
        clean_data = response.replace("Coordonnées Reçues :", "").strip()
        parts = clean_data.split(',')
        if len(parts) >= 2:
            try:
                lat = float(parts[0].strip())
                lon = float(parts[1].strip())
                return lat, lon
            except ValueError:
                return None, None
    return None, None


def extract_RSSI_SNR(response):
    """
    Extracts RSSI and SNR from the Arduino DEBUG line:
    "DEBUG: RSSI: -105.00 dBm | SNR: -12.50 dB"
    """
    if "DEBUG:" in response:
        try:
            # Split the line by the pipe character
            parts = response.split('|')
            
            # Extract RSSI: find string between 'RSSI:' and 'dBm'
            rssi_part = parts[0].split('RSSI:')[1].split('dBm')[0].strip()
            rssi = float(rssi_part)
            
            # Extract SNR: find string between 'SNR:' and 'dB'
            snr_part = parts[1].split('SNR:')[1].split('dB')[0].strip()
            snr = float(snr_part)
            
            return rssi, snr
        except (IndexError, ValueError):
            return None, None
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

try:
    ser = serial.Serial('COM4', 115200, timeout=1)
    time.sleep(2)
    print("Heltec connected.")
except Exception as e:
    print(f"Error: {e}")
    exit()

# check esp32 response
df = pd.DataFrame(columns=['lat','lon','haversine distance'])
current_rssi, current_snr = None, None

try:
    while True:
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        if not response:
            continue
        lat, lon = extract_lat_lon(response)
        rssi, snr = extract_RSSI_SNR(response)
        if rssi is not None:
            current_rssi, current_snr = rssi, snr
        if lat is not None and lon is not None:
            haversine_distance = calculate_haversine_distance(home_lat, home_lon, lat, lon)

            print("-" * 30)
            print(f"Location: {lat}, {lon}")
            print(f"Distance: {haversine_distance:.2f} km")
            if current_rssi is not None:
                print(f"Signal: RSSI {current_rssi} dBm | SNR {current_snr} dB")

            df.loc[len(df)] = [lat, lon, haversine_distance, current_rssi, current_snr]
            df.to_csv('distance_data.csv')
except KeyboardInterrupt:
    print("\nUser stop.")
finally:
    ser.close()
    print("Heltec disconnected.")
