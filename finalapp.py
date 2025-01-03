from flask import Flask, Response
from flask_cors import CORS
from mss import mss
from PIL import Image
import io
import time
import numpy as np
import cv2

app = Flask(__name__)
CORS(app)

def generate_frames():
    with mss() as sct:
        while True:
            try:
                img = sct.grab(sct.monitors[0])
                # Convert mss.MSS screenshot to numpy array
                img_array = np.array(img) 
                # Convert BGRA to RGB
                img_rgb = cv2.cvtColor(img_array, cv2.COLOR_BGRA2RGB) 
                # Convert numpy array to PIL Image
                img_pil = Image.fromarray(img_rgb) 

                img_bytes = io.BytesIO()
                img_pil.save(img_bytes, format='JPEG')
                frame = img_bytes.getvalue()

                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
            except Exception as e:
                print(f"Error capturing screen: {e}")
            time.sleep(0.01)  # Adjust frame rate as needed

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False, threaded=True)