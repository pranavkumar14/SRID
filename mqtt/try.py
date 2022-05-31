import paho.mqtt.client as mqtt #import the client1
import time
############
def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
########################################
#broker_address="192.168.1.184"
broker_address="mqtt.eclipseprojects.io"

client = mqtt.Client("P1") #create new instance
# client_pub = mqtt.Client("P2") #create new instance
client.on_message=on_message #attach function to callback
# client_pub.on_message=on_message

client.connect(broker_address,1883,60) #connect to broker
# client_pub.connect(broker_address,1883,60)

client.loop_start() #start the loop
client.subscribe("house/bulbs/bulb12")
client.publish("house/bulbs/bulb12","hhhhhh")
time.sleep(1) # wait
client.loop_stop() #stop the loop
client.disconnect()

# client_pub.loop_start() #start the loop
# client_pub.publish("house/bulbs/bulb12","Helooonvks jhjdj  dhsjdh vhsodjn")
# time.sleep(1) # wait
# client_pub.loop_stop() #stop the loop

