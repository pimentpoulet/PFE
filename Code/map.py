import pandas as pd
import os

from bokeh.io import output_file, save
from bokeh.plotting import gmap
from bokeh.models import GMapOptions, ColumnDataSource, HoverTool, ColorBar
from bokeh.transform import linear_cmap
from bokeh.palettes import Turbo256


api_key = os.environ.get('GOOGLE_API_KEY', 'AIzaSyBwRAE0FlF4FgbcsTdE9GmQgUM6-7Y76bk')

file_path = r"distance_data\2026_04_17_distance_test_ile.csv"
try:
    df = pd.read_csv(file_path)
except FileNotFoundError:
    print(f"Le fichier {file_path} est introuvable.")
    exit()

df = df.dropna(subset=['lat', 'lon'])
if not df.empty:
    center_lat = df['lat'].mean()
    center_lon = df['lon'].mean()
else:
    print("Le fichier CSV ne contient aucune coordonnée valide.")
    exit()

df_signal = df.dropna(subset=['rssi'])
df_no_signal = df[df['rssi'].isna()]

map_styles = """
[
  {
    "stylers": [
      { "saturation": -100 },
      { "lightness": 60 }
    ]
  },
  {
    "featureType": "road",
    "elementType": "labels",
    "stylers": [
      { "visibility": "off" }
    ]
  },
  {
    "featureType": "poi",
    "elementType": "labels",
    "stylers": [
      { "visibility": "off" }
    ]
  }
]
"""
gmap_options = GMapOptions(
    lat=center_lat,
    lng=center_lon,
    map_type="satellite",
    zoom=18,
    styles=map_styles
)
p = gmap(api_key, gmap_options, title="Test de portée LoRa - Projet Ruche", sizing_mode="stretch_both")

palette = Turbo256[::-1] 
min_rssi = df_signal['rssi'].min()
max_rssi = df_signal['rssi'].max()
mapper = linear_cmap(
    field_name='rssi',
    palette=palette,
    low=min_rssi,
    high=max_rssi
)

if not df_no_signal.empty:
    source_no_signal = ColumnDataSource(df_no_signal)
    p.circle('lon', 'lat', size=3, alpha=0.4, color='gray', source=source_no_signal, legend_label="Aucun signal")

if not df_signal.empty:
    source_signal = ColumnDataSource(df_signal)
    signal_renderer = p.circle('lon', 'lat', size=8, alpha=0.9, color=mapper, source=source_signal)

    hover = HoverTool(
        renderers=[signal_renderer],
        tooltips=[
            ('RSSI', '@rssi dBm'),
            ('SNR', '@snr dB')
        ]
    )
    p.add_tools(hover)

    color_bar = ColorBar(
        color_mapper=mapper['transform'],
        location=(0,0),
        title="RSSI [dBm]",
        width=50,
        title_text_font_size="14pt",
        major_label_text_font_size="12pt",
        padding=15
    )
    p.add_layout(color_bar, 'right')

output_folder = "maps"
os.makedirs(output_folder, exist_ok=True)
base_name = os.path.splitext(os.path.basename(file_path))[0]
output_file_path = os.path.join(output_folder, f"{base_name}_gmap.html")

output_file(output_file_path)
save(p)

print(f"La carte a été sauvegardée avec succès sous : {output_file_path}")
