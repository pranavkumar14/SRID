import paho.mqtt.client as mqtt
import time


def on_message(mosq, obj, msg):                            
  with open('receive_img.jpeg', 'wb') as fd:
      fd.write(msg.payload)


client = mqtt.Client("S1")                               
client.connect("localhost", 1883, 60)               
client.subscribe("highmount/camera",0)

client.on_message = on_message                          

while True:                                             
   client.loop(20)