"""
Fake data emitter for the beehive server

Author: Clément Poulin
February 27th, 2026
"""

import requests
import random
import time


# The URL of your Flask server
# If running on the same computer, you can use localhost. 
URL = "http://127.0.0.1:5000/api/data"

print("Starting ESP32 Simulator... Press Ctrl+C to stop.")

try:
    while True:
        # Generate random, realistic data for a beehive
        fake_data = {
            "v": round(random.uniform(3.2, 4.2), 2),      # Battery voltage between 3.2V and 4.2V
            "t": round(random.uniform(20.0, 35.0), 1),    # Brood temperature is usually ~35C
            "h": round(random.uniform(40.0, 60.0), 1),    # Humidity percentage
            "c": random.randint(400, 2000)                # CO2 in ppm
        }

        print(f"Sending data: {fake_data}")

        # Send the POST request to the Flask server
        response = requests.post(URL, json=fake_data)

        # Print the server's reply
        print(f"Server replied: {response.status_code} - {response.text}\n")

        # Wait 5 seconds before sending the next reading
        time.sleep(5)

except KeyboardInterrupt:
    print("Simulator stopped.")
