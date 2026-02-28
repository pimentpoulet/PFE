from flask import Flask, request, jsonify, render_template


app = Flask(__name__)

# 1. NEW: A variable to remember the latest data
dernieres_donnees = {
    "v": 0.0, "t": 0.0, "h": 0.0, "c": 0
}


@app.route('/api/data', methods=['POST'])
def reception_donnees():
    global dernieres_donnees # Tells Python to modify the memory variable above
    donnees = request.get_json()

    if donnees:
        # 2. NEW: Save the incoming data to our memory
        dernieres_donnees = donnees 

        print("\n --- Nouvelles données de la ruche reçues ! --- ")
        print(f"Batterie    : {donnees.get('v')} V")
        print(f"Température : {donnees.get('t')} °C")
        print(f"Humidité    : {donnees.get('h')} %")
        print(f"CO2         : {donnees.get('c')} ppm")
        print(" ------------------------------------------------ \n")

        return jsonify({"statut": "succes"}), 200
    else:
        return jsonify({"statut": "erreur"}), 400


# 3. NEW: A route for your future webpage to ask for the latest data
@app.route('/api/latest', methods=['GET'])
def get_latest_data():
    return jsonify(dernieres_donnees)


# 4. NEW: A route to serve the visual dashboard (we will build the HTML next!)
@app.route('/')
def index():
    return render_template('index.html')


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
