from flask import Flask, request
import time
import os

app = Flask(__name__)
os.makedirs("frames", exist_ok=True)

@app.post("/frame")
def frame():
    img = request.data  # contenido bruto del POST
    ts = time.time()
    fname = f"frames/frame_{ts:.3f}.jpg"
    with open(fname, "wb") as f:
        f.write(img)
    print("Guardado:", fname)
    return "OK", 200

if __name__ == "__main__":
    # Escucha en todas las interfaces, puerto 8000
    app.run(host="0.0.0.0", port=8000)
