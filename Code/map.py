import pandas as pd
import folium


# 1. Charger les données du fichier CSV 
# Assurez-vous que le fichier 'distance_tests.csv' est dans le même dossier
df = pd.read_csv('distance_tests.csv')

# 2. Nettoyer les doublons pour un tracé fluide
# Le fichier contient des points répétés 
path_data = df[['lat', 'lon']].drop_duplicates().values.tolist()

# 3. Créer la carte centrée sur le point moyen du parcours
map_center = [df['lat'].mean(), df['lon'].mean()]
m = folium.Map(location=map_center, zoom_start=18, tiles='OpenStreetMap')

# 4. Ajouter le tracé (Polyline)
folium.PolyLine(
    path_data, 
    color="blue", 
    weight=5, 
    opacity=0.7,
    tooltip="Mon Parcours"
).add_to(m)

# 5. Ajouter des marqueurs distinctifs pour le départ et l'arrivée
folium.Marker(
    location=path_data[0], 
    popup="Départ", 
    icon=folium.Icon(color='green', icon='play')
).add_to(m)

folium.Marker(
    location=path_data[-1], 
    popup="Arrivée", 
    icon=folium.Icon(color='red', icon='stop')
).add_to(m)

# 6. Ajouter un marqueur pour votre point de référence spécifique
ref_point = [46.80477, -71.234223]
folium.Marker(
    location=ref_point,
    popup="Point de référence",
    icon=folium.Icon(color='purple', icon='info-sign')
).add_to(m)

# 7. Sauvegarder la carte
m.save('ma_carte_quebec.html')

print("La carte a été générée avec succès : ouvrez 'ma_carte_quebec.html' dans votre navigateur.")
