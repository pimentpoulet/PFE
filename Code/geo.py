"""
geolocalisation code testing

Author: Clément Poulin
March 20th, 2026
"""

import geocoder

# Récupère la position basée sur l'adresse IP actuelle
g = geocoder.ip('me')

latitude = g.latlng[0]
longitude = g.latlng[1]

print(f"Position de l'ordi fixe : {latitude}, {longitude}")
