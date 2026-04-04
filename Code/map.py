import pandas as pd
import folium
import branca.colormap as cm
import os

import warnings
warnings.filterwarnings("ignore", message=".*The global interpreter lock.*")


# Fonction pour déterminer la couleur en fonction du RSSI
def get_color(rssi):
    # Les valeurs LoRa vont généralement de -40 (excellent) à -130 (très faible)
    if rssi >= -80:
        return 'green'       # Excellent signal
    elif rssi >= -105:
        return 'orange'      # Signal moyen
    elif rssi >= -120:
        return 'red'         # Signal faible
    else:
        return 'darkred'     # Signal à la limite de la perte


file_path = r"distance_data\2026_03_04_distance_data_2.csv"

try:
    df = pd.read_csv(file_path)
except FileNotFoundError:
    print(f"Le fichier {file_path} est introuvable.")
    exit()

# Retirer les lignes où il n'y a pas de coordonnées GPS valides
df = df.dropna(subset=['lat', 'lon'])

# 2. Déterminer le centre de la carte (moyenne des coordonnées)
if not df.empty:
    center_lat = df['lat'].mean()
    center_lon = df['lon'].mean()
else:
    print("Le fichier CSV ne contient aucune coordonnée valide.")
    exit()

# Créer la carte de base
m = folium.Map(location=[center_lat, center_lon],
               zoom_start=20,
               tiles='cartodbpositron')

# 1. Créer le gradient de couleurs (Rouge = -150, Jaune = -75, Vert = 0)
colormap = cm.LinearColormap(
    colors=['red', 'purple', 'blue'],
    vmin=-150,
    vmax=0
)
colormap.caption = 'Force du signal LoRa (RSSI en dBm)'

# 2. Boucle pour ajouter les points
for idx, row in df.iterrows():
    lat, lon = row['lat'], row['lon']
    rssi, snr = row['rssi'], row['snr']

    if pd.isna(rssi):
        folium.CircleMarker(
            location=[lat, lon],
            radius=3,
            color='gray',
            fill=True,
            fill_color='gray',
            fill_opacity=0.6,
            popup="Aucun signal"
        ).add_to(m)
    else:
        # Obtenir la couleur directement via le colormap
        color = colormap(rssi) 

        folium.CircleMarker(
            location=[lat, lon],
            radius=6,
            color=color,
            fill=True,
            fill_color=color,
            fill_opacity=0.9,
            popup=f"RSSI: {rssi} dBm<br>SNR: {snr} dB"
        ).add_to(m)

# 3. Ajouter la légende visuelle dans le coin de la carte
colormap.add_to(m)

output_folder = "maps"
os.makedirs(output_folder, exist_ok=True)
base_name = os.path.splitext(os.path.basename(file_path))[0]

output_file = os.path.join(output_folder, f"{base_name}.html")
m.save(output_file)
