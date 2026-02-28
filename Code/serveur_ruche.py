import sqlite3
import random

from datetime import datetime
from pathlib import Path
from flask import Flask, request, jsonify, render_template


# Initialisation de l'application Flask
app = Flask(__name__)


@app.route('/api/data', methods=['POST'])
def reception_donnees():
    # Flask extrait automatiquement le JSON de la requête
    donnees = request.get_json()

    if donnees:
        # On récupère les valeurs avec les mêmes clés ("v", "t", "h", "c") que dans ton C++
        v = donnees.get('v')
        t = donnees.get('t')
        h = donnees.get('h')
        co2 = donnees.get('c')

        # Affichage dans la console
        print("\n --- Nouvelles données de la ruche reçues ! --- ")
        print(f"Batterie    : {v} V")
        print(f"Température : {t} °C")
        print(f"Humidité    : {h} %")
        print(f"CO2         : {co2} ppm")
        print(" ------------------------------------------------ \n")

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

        # add data to db
        date = datetime.now().strftime("%Y-%m-%d")
        time = datetime.now().strftime("%H:%M:%S")
        cur.execute("INSERT INTO beehive VALUES (?, ?, ?, ?, ?, ?)", (date, time, v, t, h, co2))
        con.commit()

        # close db
        con.close()

        # On renvoie un code 200 (OK) à l'ESP32 pour lui dire que c'est bien reçu
        return jsonify({"status": "success", "message": "Data received"}), 200
        
    else:
        # S'il n'y a pas de JSON valide, on renvoie une erreur 400 (Bad Request)
        return jsonify({"status": "error", "message": "No data"}), 400


@app.route('/api/history', methods=['GET'])
def get_history():
    try:
        with sqlite3.connect("beehive_db.db") as con:
            con.row_factory = sqlite3.Row
            cur = con.cursor()

            cur.execute("""
                SELECT date, time, voltage, temperature, humidity, CO2
                FROM beehive
                ORDER BY date, time
            """)

            rows = cur.fetchall()

            # convert rows to list of dicts
            data = [dict(row) for row in rows]

        return jsonify({
            "status": "success",
            "count": len(data),
            "data": data
        })

    except sqlite3.Error as e:
        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500


@app.route('/')
def display_dashboard():
    return render_template('index.html')


if __name__ == '__main__':
    # host='0.0.0.0' est crucial : cela permet d'accepter les connexions 
    # venant d'autres appareils sur ton réseau Wi-Fi local (comme ton ESP32).
    app.run(host='0.0.0.0', port=5000, debug=True)
