import paho.mqtt.client as mqtt
import time

status_sender ="false"
def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    if str(message.payload.decode("utf-8")) == "true":
        status_sender = "true"

broker = "localhost"
port = 1883


receiver = mqtt.Client("R1")
receiver.connect(broker,port)
receiver.on_message = on_message


receiver.publish("status/receiver","true")
receiver.subscribe("status/sender")
while True:
    if status_sender =="true":
        break
receiver.subscribe("data")

