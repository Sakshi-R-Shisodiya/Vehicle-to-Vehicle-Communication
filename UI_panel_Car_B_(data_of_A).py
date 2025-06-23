import tkinter as tk
from tkinter import messagebox
import paho.mqtt.client as mqtt
from PIL import Image, ImageTk
import requests
from io import BytesIO

# MQTT Broker details
mqtt_broker = "broker.emqx.io"
mqtt_port = 1883
mqtt_topic_temperature = "robot/carA/temperature"
mqtt_topic_humidity = "robot/carA/humidity"
mqtt_topic_gas = "robot/carA/gas"  # New topic for MQ sensor

# Global variables to store sensor data
temperature = 0.0
humidity = 0.0
gas = 0.0

# Function to handle MQTT messages
def on_message(client, userdata, message):
    global temperature, humidity, gas
    payload = message.payload.decode("utf-8")
    if message.topic == mqtt_topic_temperature:
        temperature = float(payload)
        temperature_label.config(text=f"Temperature: {temperature} °C")
    elif message.topic == mqtt_topic_humidity:
        humidity = float(payload)
        humidity_label.config(text=f"Humidity: {humidity} %")
    elif message.topic == mqtt_topic_gas:
        gas = float(payload)
        gas_label.config(text=f"Gas Concentration: {gas} ppm")
    
    # Check for alerts
    if temperature > 50:
        messagebox.showwarning("High Temperature Alert", f"Temperature is too high: {temperature} °C")
    if humidity < 30:
        messagebox.showwarning("Low Humidity Alert", f"Humidity is too low: {humidity} %")
    if gas > 500:  # Example threshold for gas concentration
        messagebox.showwarning("High Gas Alert", f"Gas concentration is too high: {gas} ppm")

# Function to connect to MQTT broker
def connect_mqtt():
    client.connect(mqtt_broker, mqtt_port)
    client.subscribe(mqtt_topic_temperature)
    client.subscribe(mqtt_topic_humidity)
    client.subscribe(mqtt_topic_gas)
    client.loop_start()

# Initialize MQTT client
client = mqtt.Client()
client.on_message = on_message

# Create the main window
root = tk.Tk()
root.title("Vehicle 2 Vehicle Communication - Data received from Vehicle A")
root.geometry("800x600")

# Load background image from URL
def load_background_image():
    try:
        # Use the provided image URL
        url = "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTMKDDEXgTMwVO1D6-f1M8Ybm5L48UXJQJxsw&s"
        response = requests.get(url)
        response.raise_for_status()  # Check for HTTP errors
        image_data = response.content
        image = Image.open(BytesIO(image_data))
        image = image.resize((800, 600), Image.LANCZOS)
        return ImageTk.PhotoImage(image)
    except Exception as e:
        print(f"Error loading background image: {e}")
        return None

background_image = load_background_image()

# Add background image to the UI
if background_image:
    background_label = tk.Label(root, image=background_image)
    background_label.place(relwidth=1, relheight=1)
else:
    # Fallback: Use a solid color if the image fails to load
    root.configure(bg="lightgray")

# Add a title label at the top
title_label = tk.Label(
    root,
    text="Vehicle 2 Vehicle Communication - Data received from Vehicle A",
    font=("Arial", 18, "bold"),
    bg="white",
    fg="black"
)
title_label.pack(pady=10)

# Create labels for temperature, humidity, and gas
temperature_label = tk.Label(root, text="Temperature: -- °C", font=("Arial", 24), bg="white")
temperature_label.place(relx=0.1, rely=0.2, anchor="w")

humidity_label = tk.Label(root, text="Humidity: -- %", font=("Arial", 24), bg="white")
humidity_label.place(relx=0.1, rely=0.3, anchor="w")

gas_label = tk.Label(root, text="Gas Concentration: -- ppm", font=("Arial", 24), bg="white")
gas_label.place(relx=0.1, rely=0.4, anchor="w")

# Connect to MQTT broker
connect_mqtt()

# Start the Tkinter main loop
root.mainloop()
