import paho.mqtt.client as mqtt
import time
from csv import reader

def check_user_input(input):
    try:
        # Convert it into integer
        val = int(input)
        return 0
    except ValueError:
        try:
            # Convert it into float
            val = float(input)
            return 0
        except ValueError:
            return 1

broker = "localhost"
port = 1883
# username = "pranav"
# password ="12345"

publisher = mqtt.Client("P1")
publisher2 = mqtt.Client("P2")
# publisher.username_pw_set(username,password)
publisher.connect(broker,port)
publisher2.connect("mqtt.eclipseprojects.io",port)

temp=0
with open('data.csv', 'r') as read_obj:
    csv_reader = reader(read_obj)
    for row in csv_reader:
        print(row[0])
        val = check_user_input(row[0])
        if val:
            publisher.publish("house/Room_Temp",row[0])
        else:
            publisher2.publish("house/Room_Temp",float(row[0]))
        time.sleep(5)