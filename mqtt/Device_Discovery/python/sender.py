import paho.mqtt.client as mqtt 
import time
from csv import reader

broker_address="localhost"
status_receiver = False




############
def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    global status_receiver
    if str(message.payload.decode("utf-8"))=="1":
        status_receiver = True
########################################




client = mqtt.Client("S1")
client.on_message=on_message 
client.connect(broker_address,1883,60)


client.loop_start() 
client.publish("status/sender","1")
client.subscribe("status/receiver")
while True:
    time.sleep(2)
    client.publish("status/sender","1")
    client.subscribe("status/receiver")
    client.publish("status/sender","1")
    if status_receiver == True:
        break

with open('data.csv', 'r') as read_obj:
    csv_reader = reader(read_obj)
    for row in csv_reader:
        print(row[0])
        client.publish("csv/data",row[0])
        time.sleep(2)


time.sleep(1)
client.publish("status/sender","0")
client.loop_stop() 
client.disconnect()

