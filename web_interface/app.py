#!/usr/bin/env python3
"""
Web Interface Backend for Parking System
"""

import sys
import os
import time
from flask import Flask, jsonify, request, send_from_directory

# Add parent directory to path to import i2c_master
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from i2c_master import ParkingMaster

app = Flask(__name__, static_folder='static')

# Global parking master instance
# Initialize it lazily or on startup
try:
    master = ParkingMaster()
    print("✅ ParkingMaster initialized successfully")
except Exception as e:
    print(f"⚠️ Error initializing ParkingMaster: {e}")
    # We might be running locally without I2C, creating a mock for testing if needed
    # For now, we assume it works or fails hard.
    master = None

@app.route('/')
def index():
    return send_from_directory('static', 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    return send_from_directory('static', path)

@app.route('/api/status')
def get_status():
    if master:
        try:
            status = master.get_all_status()
            if status:
                return jsonify(status)
            else:
                return jsonify({"error": "Failed to read status"}), 500
        except Exception as e:
            return jsonify({"error": str(e)}), 500
    else:
        # Mock data for testing if hardware not available
        return jsonify({
            'car_detected': False,
            'is_dark': False,
            'servo_angle': 0,
            'led_red': True,
            'led_green': False,
            'led_white': False,
            'release_counter': 0
        })

@app.route('/api/servo', methods=['POST'])
def set_servo():
    if not master:
        return jsonify({"error": "Hardware not connected"}), 503
        
    data = request.json
    angle = data.get('angle')
    
    if angle is None:
        return jsonify({"error": "Angle required"}), 400
        
    try:
        if master.set_servo_angle(int(angle)):
            return jsonify({"success": True, "angle": angle})
        else:
            return jsonify({"error": "Failed to set angle"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
