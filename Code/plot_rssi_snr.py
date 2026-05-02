import os
import math
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np


# ── Haversine ────────────────────────────────────────────────────────────────
def haversine_m(lat1, lon1, lat2, lon2):
    """Return the great-circle distance in metres between two GPS points."""
    R = 6_371_000  # Earth radius in metres
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlam = math.radians(lon2 - lon1)
    a = math.sin(dphi / 2) ** 2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlam / 2) ** 2
    return 2 * R * math.asin(math.sqrt(a))


# ── Load all CSV files ────────────────────────────────────────────────────────
DATA_DIR = os.path.join(os.path.dirname(__file__), "..", "distance_data/SF")

datasets = {}  # label -> DataFrame with columns [distance_m, rssi, snr]

for fname in sorted(os.listdir(DATA_DIR)):
    if not fname.endswith(".csv"):
        continue

    fpath = os.path.join(DATA_DIR, fname)
    df = pd.read_csv(fpath)

    # Must have RSSI column to be useful
    if "rssi" not in df.columns:
        print(f"Skipping {fname}: no RSSI column.")
        continue

    # Compute distance in metres
    if "haversine distance" in df.columns:
        # Pre-computed column is in km → convert to metres
        df["distance_m"] = df["haversine distance"] * 1000
    elif {"lat", "lon"}.issubset(df.columns):
        # Calculate from the first valid GPS point
        first = df[["lat", "lon"]].dropna().iloc[0]
        df["distance_m"] = df.apply(
            lambda row: haversine_m(first["lat"], first["lon"], row["lat"], row["lon"])
            if pd.notna(row["lat"]) and pd.notna(row["lon"])
            else float("nan"),
            axis=1,
        )
    else:
        print(f"Skipping {fname}: no distance or GPS columns.")
        continue

    # Keep only rows with valid signal data
    df = df.dropna(subset=["rssi", "distance_m"])
    # Remove cluster around 0 m AND around -100 dBm simultaneously
    df = df[~((df["distance_m"] <= 10) & (df["rssi"].between(-120, -90)))]
    if df.empty:
        print(f"Skipping {fname}: no valid rows after filtering.")
        continue

    label = fname.replace(".csv", "")
    datasets[label] = df[["distance_m", "rssi", "snr"]].copy()
    print(f"Loaded '{label}': {len(df)} points, "
          f"distance {df['distance_m'].min():.0f}–{df['distance_m'].max():.0f} m")


if not datasets:
    print("No usable data found.")
    exit(1)


# ── Plot ──────────────────────────────────────────────────────────────────────
colors = ["black", "purple", "blue", "green", "red", "pink"]

fig, ((ax_rssi, ax_rssi_trend), (ax_snr, ax_snr_trend)) = plt.subplots(
    2, 2, figsize=(16, 8), sharex=False
)

for (label, df), color in zip(datasets.items(), colors):
    dist = df["distance_m"]
    label = label[-20:].lstrip("_")

    # Raw data (left column)
    ax_rssi.scatter(dist, df["rssi"], label=label, color=color, alpha=0.7, s=30)
    ax_snr.scatter(dist, df["snr"],  label=label, color=color, alpha=0.7, s=30)

    # Trend lines (right column)
    if len(df) >= 3 and dist.max() > dist.min():
        try:
            x_range = np.linspace(dist.min(), dist.max(), 200)
            # RSSI: logarithmic fit
            log_dist = np.log10(dist.clip(lower=1))
            log_x_range = np.log10(np.maximum(x_range, 1))
            coef_rssi = np.polyfit(log_dist, df["rssi"], 1)
            ax_rssi_trend.plot(x_range, np.polyval(coef_rssi, log_x_range),
                               label=label, color=color, linewidth=1.5)
            # SNR: linear fit (polynomial degree 1 on distance)
            coef_snr = np.polyfit(dist, df["snr"], 1)
            ax_snr_trend.plot(x_range, np.polyval(coef_snr, x_range),
                              label=label, color=color, linewidth=1.5)
        except Exception:
            pass

ax_rssi.set_title("RSSI — données brutes", fontsize=16)
ax_rssi.set_ylabel("RSSI (dBm)", fontsize=14)
ax_rssi.set_xlabel("Distance (m)", fontsize=14)
ax_rssi.grid(True, linestyle=":", alpha=0.5)
ax_rssi.legend(fontsize=14, loc="upper right")

ax_rssi_trend.set_title("RSSI — courbes de tendance", fontsize=16)
ax_rssi_trend.set_ylabel("RSSI (dBm)", fontsize=14)
ax_rssi_trend.set_xlabel("Distance (m)", fontsize=14)
ax_rssi_trend.grid(True, linestyle=":", alpha=0.5)
ax_rssi_trend.legend(fontsize=14, loc="upper right")

ax_snr.set_title("SNR — données brutes", fontsize=16)
ax_snr.set_ylabel("SNR (dB)", fontsize=14)
ax_snr.set_xlabel("Distance (m)", fontsize=14)
ax_snr.grid(True, linestyle=":", alpha=0.5)
ax_snr.legend(fontsize=14, loc="upper right")

ax_snr_trend.set_title("SNR — courbes de tendance", fontsize=16)
ax_snr_trend.set_ylabel("SNR (dB)", fontsize=14)
ax_snr_trend.set_xlabel("Distance (m)", fontsize=14)
ax_snr_trend.grid(True, linestyle=":", alpha=0.5)
ax_snr_trend.legend(fontsize=14, loc="upper right")

plt.tight_layout()
plt.show()
