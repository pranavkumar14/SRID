import paho.mqtt.client as mqtt
import time

status_sender = False

def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    global status_sender
    # print(status_sender)
    if str(message.payload.decode("utf-8"))=="1":
        status_sender = True
    elif str(message.payload.decode("utf-8"))=="0":
        status_sender = False


broker = "localhost"
port = 1883


receiver = mqtt.Client("R1")
receiver.connect(broker,port)
receiver.on_message = on_message

receiver.loop_start() 
receiver.publish("status/receiver","1")
receiver.subscribe("status/sender")

while True:
    receiver.publish("status/receiver","1")
    receiver.subscribe("status/sender")
    time.sleep(2)
    # print(status_sender)
    if status_sender ==True:
        break
while True:    
    receiver.subscribe("csv/data")
    time.sleep(1)
    if status_sender == False:
        break
receiver.publish("status/receiver","0")
receiver.loop_stop() 
receiver.disconnect()


