from flask import Flask, request, jsonify


# Initialisation de l'application Flask
app = Flask(__name__)

# Définition de la route (l'URL) qui correspond à ce qu'on a mis dans l'ESP32
@app.route('/api/data', methods=['POST'])
def reception_donnees():
    # Flask extrait automatiquement le JSON de la requête
    donnees = request.get_json()

    if donnees:
        # On récupère les valeurs avec les mêmes clés ("v", "t", "h", "c") que dans ton C++
        volts = donnees.get('v')
        temp = donnees.get('t')
        hum = donnees.get('h')
        co2 = donnees.get('c')

        # Affichage dans la console
        print("\n --- Nouvelles données de la ruche reçues ! --- ")
        print(f"Batterie    : {volts} V")
        print(f"Température : {temp} °C")
        print(f"Humidité    : {hum} %")
        print(f"CO2         : {co2} ppm")
        print(" ------------------------------------------------ \n")

        # On renvoie un code 200 (OK) à l'ESP32 pour lui dire que c'est bien reçu
        return jsonify({"status": "success", "message": "Data received"}), 200
        
    else:
        # S'il n'y a pas de JSON valide, on renvoie une erreur 400 (Bad Request)
        return jsonify({"status": "error", "message": "No data"}), 400

if __name__ == '__main__':
    # host='0.0.0.0' est crucial : cela permet d'accepter les connexions 
    # venant d'autres appareils sur ton réseau Wi-Fi local (comme ton ESP32).
    app.run(host='0.0.0.0', port=5000, debug=True)
