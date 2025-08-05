from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
import uvicorn
import zmq.asyncio
import json
import time
import math 
import asyncio

context = zmq.asyncio.Context()

sensors_socket = context.socket(zmq.SUB)
motors_socket = context.socket(zmq.PUB)
sensors_socket.connect("tcp://lunar-rover-0.eec:5555")
motors_socket.connect("tcp://lunar-rover-0.eec:5556")

# Subscribe to SLI Lunar Rover
sensorsTopicfilter = b"lunar-rover-sensors"
sensors_socket.setsockopt(zmq.SUBSCRIBE, sensorsTopicfilter)

# Motors publisher
motorsTopic = b"motors-commands"
tiltMotorTopic = b"tilt-motor-command"
setupTopic = b"motors-setup"

app = FastAPI()

#event_mask = motors_socket.poll(10000, zmq.POLLOUT)
#if event_mask == zmq.POLLOUT:
#    print("pullout ready") 

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"]
)

# Frontend para o rover
@app.get("/")
def serve_html():
    return FileResponse("rover-page.html")

async def rover_sensors_waitable(ws: WebSocket):
    while True:
        full_data = await sensors_socket.recv_multipart();
        if len(full_data) == 2 and full_data[0] == sensorsTopicfilter:
            data = full_data[1]
            # JSON data structure
            ddd = json.loads(data)
            #print(json.dumps(ddd, indent=4))
            await ws.send_text(data.decode('utf-8'))

async def remote_cli_waitable(ws: WebSocket):
    while True:
        data = await ws.receive_text()
        # JSON data structure
        ddd = json.loads(data)
        if 'v' in ddd:
            await motors_socket.send_multipart([motorsTopic, json.dumps(ddd).encode('utf-8')])
        if 'alpha' in ddd: 
            await motors_socket.send_multipart([tiltMotorTopic, json.dumps({"alpha": ddd["alpha"]}).encode('utf-8')])

# Websocket para obter dados em real-time
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    await motors_socket.send_multipart([setupTopic, json.dumps({"state": "run"}).encode('utf-8')])
    await motors_socket.send_multipart([tiltMotorTopic, json.dumps({"alpha": 30*math.pi/180}).encode('utf-8')]) # tilt angle in radians

    rover_sensors_task = asyncio.create_task(rover_sensors_waitable(websocket))
    remote_cli_task = asyncio.create_task(remote_cli_waitable(websocket))
    tasks = [rover_sensors_task, remote_cli_task]
    
    for earliest_ in asyncio.as_completed(tasks):
        # earliest_ is done. The result can be obtained by
        # awaiting it or calling earliest_.result()
        await earliest_

        if earliest_ is rover_sensors_task:
            print("Rover connection broken.")
        else:
            print("Websocket connection broken.")
    await motors_socket.send_multipart([setupTopic, json.dumps({"state": "stop"}).encode('utf-8')])
            

# serve app
if __name__ == "__main__":
    uvicorn.run("lunar-rover-web:app", host="0.0.0.0", port=8000, reload=True)