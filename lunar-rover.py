import zmq
import json
import time
import math 

context = zmq.Context()

sensors_socket = context.socket(zmq.SUB)
motors_socket = context.socket(zmq.PUB)
sensors_socket.connect("tcp://lunar-rover-0:5555")
motors_socket.connect("tcp://lunar-rover-0:5556")

# Subscribe to SLI Lunar Rover
sensorsTopicfilter = b"lunar-rover-sensors"
sensors_socket.setsockopt(zmq.SUBSCRIBE, sensorsTopicfilter)

# Motors publisher
motorsTopic = b"motors-commands"
tiltMotorTopic = b"tilt-motor-command"
setupTopic = b"motors-setup"

#event_mask = motors_socket.poll(10000, zmq.POLLOUT)
#if event_mask == zmq.POLLOUT:
#    print("pullout ready") 
time.sleep(1)

motors_socket.send_multipart([setupTopic, json.dumps({"state": "run"}).encode('utf-8')])
motors_socket.send_multipart([tiltMotorTopic, json.dumps({"alpha": 30*math.pi/180}).encode('utf-8')]) # tilt angle in radians
#time.sleep(10)
step = 0
while True:
    full_data = sensors_socket.recv_multipart();
    if len(full_data) == 2 and full_data[0] == sensorsTopicfilter:
        data = full_data[1]
        ddd = json.loads(data)
        print(json.dumps(ddd, indent=4))
    
    commands_body = {
        "v": 0, # [-0.44, -0.162] [0.162, 0.44]
        "w": 0 # [-3.5, -1.48] [1.48, 3.5]
    }
    motors_socket.send_multipart([motorsTopic, json.dumps(commands_body).encode('utf-8')])
    motors_socket.send_multipart([tiltMotorTopic, json.dumps({"alpha": 110*math.pi/180}).encode('utf-8')])
    #motors_socket.send_multipart([setupTopic, json.dumps({"state": "stop"}).encode('utf-8')])

