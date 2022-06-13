import paho.mqtt.client as mqtt
import time

broker="localhost"
client = mqtt.Client("P1")                                # Start MQTT Client
client.connect(broker, 1883, 60)                 # Connect to server

client.loop_start()                                       # initial start before loop

while True:
    file = open("flower.jpeg", "rb")         # open the file, note r = read, b = binary
    imagestring = file.read()                                            # read the file
    byteArray = bytes(imagestring)                                       # convert to byte string
    client.publish(topic="highmount/camera", payload= byteArray ,qos=0)  # publish it to the MQ queue
    time.sleep(20)                                                       # wait for next image