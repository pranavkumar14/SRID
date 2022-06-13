import paho.mqtt.client as mqtt
import time


def on_message(mosq, obj, msg):                             # Function to retrieve file when received
  with open('imagenow.jpeg', 'wb') as fd:
      fd.write(msg.payload)


client = mqtt.Client("S1")                                # Start MQTT Client, NB changed to P2
client.connect("localhost", 1883, 60)                 # Connect to server
client.subscribe("highmount/camera",0)

client.on_message = on_message                            # This is key - it calls the function

while True:                                               # Loop and wait for next image
   client.loop(20)