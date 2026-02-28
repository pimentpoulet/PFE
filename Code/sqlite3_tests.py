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

if False:
    # fill with fake testing data
    for i in range(10):

        fake_date = datetime.now().strftime("%Y-%m-%d")
        fake_time = datetime.now().strftime("%H:%M:%S")
        v = round(random.uniform(3.2, 4.2), 1)
        t = round(random.uniform(20.0, 35.0), 1)
        h = round(random.uniform(40.0, 60.0), 1)
        c = round(random.randint(400, 2000))

        cur.execute("INSERT INTO beehive VALUES (?, ?, ?, ?, ?, ?)", (fake_date, fake_time, v, t, h, c))

    con.commit()

print(cur.execute("SELECT temperature FROM beehive").fetchall())

