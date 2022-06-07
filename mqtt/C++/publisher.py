import paho.mqtt.client as mqtt
import time

broker = "localhost"
port = 1883
# username = "pranav"
# password ="12345"

publisher = mqtt.Client("P1")

# publisher.username_pw_set(username,password)
publisher.connect(broker,port)


temp=0
while temp<20:
    publisher.publish("house/Room_Temp",temp)
    time.sleep(5)
    temp+=1