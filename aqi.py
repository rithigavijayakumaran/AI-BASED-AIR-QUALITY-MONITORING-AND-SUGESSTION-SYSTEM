import serial
import time
import numpy as np
from tensorflow.keras.models import load_model

# Load the trained model
model = load_model('C:/Users/RITHIGA/Documents/aqi/6final.h5')  # Using forward slashes
 # Make sure 6final.h5 is in the same directory

# Set up serial communication with Arduino (adjust 'COM15' to your port)
ser = serial.Serial('COM15', 115200, timeout=1)  # Replace COM15 with your actual port
time.sleep(2)  # Allow time for serial connection

def get_aqi_category(prediction):
    # Define AQI category based on prediction
    if prediction < 50:
        return "Healthy"
    elif prediction < 100:
        return "Moderate"
    elif prediction < 150:
        return "Unhealthy for Sensitive Groups"
    elif prediction < 200:
        return "Unhealthy"
    elif prediction < 300:
        return "Very Unhealthy"
    else:
        return "Hazardous"

while True:
    if ser.in_waiting > 0:
        data = ser.readline().decode().strip()
        
        if data.startswith("DATA:"):
            try:
                # Parse the sensor values (assuming the order is MQ-7, MQ-135, MQ-136, PM2.5, PM10, Methane)
                values = list(map(int, data[5:-1].split(',')))  # Exclude 'DATA:' prefix and ';' suffix
                
                if len(values) == 6:  # Ensure we have 6 values
                    # Convert to NumPy array and reshape for the model
                    input_data = np.array(values).reshape(1, -1)  # Adjust shape as per model input

                    # Make a prediction
                    aqi_prediction = model.predict(input_data)
                    aqi = int(aqi_prediction[0][0])  # Get the predicted AQI

                    # Classify the AQI
                    category = get_aqi_category(aqi)

                    # Send back the AQI and category to Arduino
                    response = f"{aqi},{category}\n"
                    ser.write(response.encode())

                    # Print for debugging
                    print(f"Sent to Maixduino: {response}")

                else:
                    print(f"Received data does not contain 6 values: {data}")

            except Exception as e:
                print(f"Error processing data: {e}")

    time.sleep(2)
