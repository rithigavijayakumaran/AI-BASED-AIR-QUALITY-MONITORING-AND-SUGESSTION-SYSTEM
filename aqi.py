import serial
import time
import numpy as np
import pickle

# Load the trained models
with open('C:/Users/RITHIGA/Documents/aqi/models/RandomForest_regressor.pkl', 'rb') as f:
    rf_regressor = pickle.load(f)

with open('C:/Users/RITHIGA/Documents/aqi/models/RandomForest_classifier.pkl', 'rb') as f:
    rf_classifier = pickle.load(f)

with open('C:/Users/RITHIGA/Documents/aqi/models/label_encoder.pkl', 'rb') as f:
    encoder = pickle.load(f)

# Set up serial communication with Maixduino (adjust 'COM15' to your port)
ser = serial.Serial('COM15', 115200, timeout=1)  # Replace COM15 with your actual port
time.sleep(2)  # Allow time for serial connection

def get_aqi_category(aqi_class):
    return encoder.inverse_transform([aqi_class])[0]  
while True:
    if ser.in_waiting > 0:
        data = ser.readline().decode().strip()
        
        if data.startswith("DATA:"):
            try:
                # Parse the sensor values (assuming the order is PM2.5, PM10, H2S, SO2, CO, O3)
                values = list(map(float, data[5:].split(',')))  # Exclude 'DATA:' prefix
                
                if len(values) == 6:  # Ensure we have 6 values
                    # Convert to NumPy array and reshape for the model
                    input_data = np.array(values).reshape(1, -1)

                    # Make AQI prediction
                    aqi_prediction = rf_regressor.predict(input_data)
                    aqi = int(aqi_prediction[0])  # Get the predicted AQI

                    # Predict AQI category
                    aqi_class = rf_classifier.predict(input_data)[0]
                    category = get_aqi_category(aqi_class)

                    # Send back the AQI and category to Maixduino
                    response = f"{aqi},{category}\n"
                    ser.write(response.encode())

                    # Print for debugging
                    print(f"Sent to Maixduino: {response}")

                else:
                    print(f"Received data does not contain 6 values: {data}")

            except Exception as e:
                print(f"Error processing data: {e}")

    time.sleep(2)
