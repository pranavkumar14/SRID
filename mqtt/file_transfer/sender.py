import paho.mqtt.client as mqtt
import time

broker="localhost"
client = mqtt.Client("P1")                                
client.connect(broker, 1883, 60)                 

client.loop_start()                                      

while True:
    file = open("flower.jpeg", "rb")         
    imagestring = file.read()                                          
    byteArray = bytes(imagestring)                                     
    client.publish(topic="highmount/camera", payload= byteArray ,qos=0) 
    time.sleep(20)                                                       