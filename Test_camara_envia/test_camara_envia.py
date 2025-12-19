from flask import Flask, request
import os
import time

app = Flask(__name__)

# Directorio donde se guardarán las imágenes
UPLOAD_FOLDER = 'frames'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)  # Crea el directorio si no existe

@app.route('/frame', methods=['POST'])
def receive_frame():
    try:
        # Obtén la imagen (binario) del cuerpo de la solicitud POST
        img_data = request.data
        if not img_data:
            return "No image data received", 400

        # Nombre del archivo basado en la hora actual para evitar sobrescribir
        filename = f"frame_{time.time():.6f}.jpg"
        filepath = os.path.join(UPLOAD_FOLDER, filename)

        # Guardar la imagen en el archivo
        with open(filepath, 'wb') as f:
            f.write(img_data)

        print(f"Imagen guardada en {filepath}")
        return f"Imagen guardada como {filename}", 200
    except Exception as e:
        print(f"Error al recibir la imagen: {e}")
        return "Error al procesar la imagen", 500

# Inicia el servidor Flask
if __name__ == "__main__":
    print("Servidor escuchando en http://0.0.0.0:8000")
    app.run(host='0.0.0.0', port=8000)
