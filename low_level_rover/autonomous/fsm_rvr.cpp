#include "fsm_rvr.h"
#include "parser.h"
#include "PlatformOps.h"

bool StatusRunning;
FSMState currentState;
Frame currentFrame;
String serialIn;



void init_fsm(void) {
    currentState = READY;
    StatusRunning = false;
}

void encode_fsm() {

    switch (currentState) {
    case READY:
        //Serial.println("STATE: READY");
        StatusRunning = false;
        if (Serial.available())
            currentState = READ_FRAME;
        break;

    case READ_FRAME:
        //Serial.println("STATE: READ_FRAME");
        if(handleSerialInput(serialIn)){ 
            //Serial.println(serialIn);
            if (parseFrame(serialIn, currentFrame)) {
                //Serial.println("Frame parsed OK");
                currentState = (FSMState)executeCommand(currentFrame);
               
            }
            else {
                Serial.println("<err,1,frm>");
                currentState = READY;
            }
             serialIn = "";
        }
        else {
            if (StatusRunning) {
                currentState = RUNNING;
            }
            else
                currentState = READY;
        }
        
        
        break;

    case RUNNING:
        
       //Serial.println("STATE: RUNNING");

       check_receiving_speed_frames();

        if (Serial.available()) {
            //Serial.println("New frame detected during RUNNING");
            currentState = READ_FRAME;
            break;
        }

        if (SampleSensors) {
            SampleSensors  = false;
            //Serial.println("Timeout occurred! Switching to READ_SENSORS");
            currentState = READ_SENSORS;
        }
        else {
            currentState = RUNNING;
        }
        break;

    case READ_SENSORS:
        //Serial.println("STATE: READ_SENSORS");
        readSensors();

        currentState =RUNNING;
        break;
    }
}