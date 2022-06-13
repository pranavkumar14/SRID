import paho.mqtt.client as mqtt
import time

def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    # print("message topic=",message.topic)
    # print("message qos=",message.qos)
    # print("message retain flag=",message.retain) 

broker = "localhost"
port = 1883
topic = "$SYS/broker/clients/active"

subs = mqtt.Client("S1")
print("smdv jfs")

subs.connect(broker,port)
subs.subscribe(topic)
subs.on_message = on_message

subs.loop_forever()