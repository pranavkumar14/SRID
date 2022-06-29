import paho.mqtt.client as mqtt 
import time


broker_address="localhost"
status_receiver = "false"




############
def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    if str(message.payload.decode("utf-8")) == "true":
        status_receiver = "true"
########################################




client = mqtt.Client("S1")
client.on_message=on_message 
client.connect(broker_address,1883,60)


client.loop_start() 
client.publish("status/sender","true")
client.subscribe("status/receiver")
while True:
    if status_receiver == "true":
        break

with open('data.csv', 'r') as read_obj:
    csv_reader = reader(read_obj)
    for row in csv_reader:
        print(row[0])
        val = check_user_input(row[0])
        client.publish("data",row[0])
        time.sleep(2)


time.sleep(1)
client.publish("status/sender","false")
client.loop_stop() 
client.disconnect()

