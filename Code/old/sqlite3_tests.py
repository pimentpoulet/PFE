"""
sqlite3 package testing code

Author: Clément Poulin
February 28th, 2026
"""

import sqlite3
import random

from datetime import datetime
from pathlib import Path


# create db
db_path = Path("beehive_db.db")
if db_path.is_file():
    print(f"Fichier existant : {db_path.name}")
else:
    print("Création d'une nouvelle base de données.")

# connect to db
try:
    con = sqlite3.connect(db_path)
except sqlite3.Error as e:
    print(f"Error: {e}")

# create beehive table
try:
    cur = con.cursor()
    cur.execute("CREATE TABLE beehive(date, time, voltage, temperature, humidity, CO2)")
except sqlite3.OperationalError:
    pass

print(cur.execute("SELECT temperature FROM beehive").fetchall())
