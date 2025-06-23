import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt
from PIL import Image, ImageTk
import requests
from io import BytesIO

# MQTT Broker details
MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883
MQTT_TOPIC_TEMPERATURE = "robot/carB/temperature"
MQTT_TOPIC_HUMIDITY = "robot/carB/humidity"
MQTT_TOPIC_OBSTACLE = "robot/carB/obstacle"

# Global variables to store MQTT data
temperature_data = "N/A"
humidity_data = "N/A"
obstacle_data = "No Obstacle"

# Function to update the UI
def update_ui():
    temperature_entry.delete(0, tk.END)
    temperature_entry.insert(0, f"{temperature_data} °C")
    
    humidity_entry.delete(0, tk.END)
    humidity_entry.insert(0, f"{humidity_data} %")
    
    obstacle_entry.delete(0, tk.END)
    obstacle_entry.insert(0, obstacle_data)
    
    root.after(1000, update_ui)  # Refresh every 1 second

# MQTT Callbacks
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    client.subscribe(MQTT_TOPIC_TEMPERATURE)
    client.subscribe(MQTT_TOPIC_HUMIDITY)
    client.subscribe(MQTT_TOPIC_OBSTACLE)

def on_message(client, userdata, msg):
    global temperature_data, humidity_data, obstacle_data
    payload = msg.payload.decode("utf-8")
    if msg.topic == MQTT_TOPIC_TEMPERATURE:
        temperature_data = payload
    elif msg.topic == MQTT_TOPIC_HUMIDITY:
        humidity_data = payload
    elif msg.topic == MQTT_TOPIC_OBSTACLE:
        obstacle_data = payload
    print(f"Received: {msg.topic} -> {payload}")

# Initialize MQTT Client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT Broker
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_start()

# Create Tkinter Window
root = tk.Tk()
root.title("Car Dashboard")
root.geometry("800x600")  # Increased window size

# Add Heading
heading_label = ttk.Label(root, text="Vehicle 2 Vehicle Communication - Data received from Vehicle B", font=("Arial", 20, "bold"))
heading_label.pack(pady=10)

# Load Car Dashboard Image from URL
def load_image_from_url(url):
    response = requests.get(url)
    image_data = response.content
    image = Image.open(BytesIO(image_data))
    image = image.resize((600, 300), Image.LANCZOS)  # Increased image size
    return ImageTk.PhotoImage(image)

# Replace with your HD car dashboard image URL
image_url = "https://plus.unsplash.com/premium_photo-1664360971083-3ac22f8d7cc8?fm=jpg&q=60&w=3000&ixlib=rb-4.0.3&ixid=M3wxMjA3fDB8MHxzZWFyY2h8MXx8Y2FyJTIwZGFzaGJvYXJkfGVufDB8fDB8fHww"  # Replace with a valid image URL
car_image = load_image_from_url(image_url)

# Create a Frame for the Image
image_frame = ttk.Frame(root)
image_frame.pack(pady=10)

# Add Image to the Frame
image_label = ttk.Label(image_frame, image=car_image)
image_label.pack()

# Create a Frame for Data Display
data_frame = ttk.Frame(root)
data_frame.pack(pady=10)

# Temperature Entry
ttk.Label(data_frame, text="Temperature:", font=("Arial", 14)).grid(row=0, column=0, padx=10, pady=10)
temperature_entry = ttk.Entry(data_frame, font=("Arial", 14), width=20)
temperature_entry.grid(row=0, column=1, padx=10, pady=10)

# Humidity Entry
ttk.Label(data_frame, text="Humidity:", font=("Arial", 14)).grid(row=1, column=0, padx=10, pady=10)
humidity_entry = ttk.Entry(data_frame, font=("Arial", 14), width=20)
humidity_entry.grid(row=1, column=1, padx=10, pady=10)

# Obstacle Entry
ttk.Label(data_frame, text="Obstacle:", font=("Arial", 14)).grid(row=2, column=0, padx=10, pady=10)
obstacle_entry = ttk.Entry(data_frame, font=("Arial", 14), width=20)
obstacle_entry.grid(row=2, column=1, padx=10, pady=10)

# Start UI Update Loop
update_ui()

# Run Tkinter Main Loop
root.mainloop()

# Disconnect MQTT Client on Exit
client.loop_stop()
client.disconnect()
