#include "fsm_rvr.h"
#include "parser.h"
#include <Arduino.h>
#include "bmi323.h"


#include "SoftPWM.h"
#include "soft_servo.h"
#include "car_control.h"
#include "analog_inputs.h"

bool StatusRunning;
extern analog_sense_t Sensor[];
extern bmi323_data_t sensor_data;
FSMState currentState;
Frame currentFrame;
extern String serialIn;
extern bool sample;
void init_fsm(void) {
    currentState = READY;
 
    StatusRunning = false;
}
bool handleSerialInput() {
    bool frameReady = false;
    //Serial.println("handle Serial");
    if (Serial.available()) {

        char c = Serial.read();
        serialIn += c;
        //Serial.print(c);
        if (c=='\n') {
          //Serial.println(serialIn);
            frameReady = true; 
        }
    }
     return frameReady;
}



// --- Simulated sensor reading ---
void readSensors() {
    

    distance_check();
    //digitalWrite(8, HIGH);
    bmi323_read_data_burst();
    //digitalWrite(8, LOW);

      
  Serial.print("<ir,3,");
  Serial.print((Sensor[0].raw));
  Serial.print(",");
  Serial.print((Sensor[1].raw));
  Serial.print(",");
  Serial.print((Sensor[2].raw));
  Serial.print("><acc,3,");
  Serial.print(sensor_data.acc_x);
  Serial.print(",");
  Serial.print(sensor_data.acc_y);
  Serial.print(",");
  Serial.print(sensor_data.acc_z);
  Serial.print("><gyr,3,");
  Serial.print(sensor_data.gyr_x);
  Serial.print(",");
  Serial.print(sensor_data.gyr_y);
  Serial.print(",");
  Serial.print(sensor_data.gyr_z);
  Serial.print("><temp,1,");
  Serial.print(sensor_data.temperature);
  Serial.print("><time,1,");
  Serial.print(sensor_data.sensor_time);
  Serial.println(">");


}
void encode_fsm() {

    switch (currentState) {
    case READY:
        //Serial.println("STATE: READY");
        //serialIn ="";
        StatusRunning = false;
        if (Serial.available())
            currentState = READ_FRAME;
        break;

    case READ_FRAME:
        //Serial.println("STATE: READ_FRAME");
        if(handleSerialInput()){
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
          
        if (Serial.available()) {
            //Serial.println("New frame detected during RUNNING");
            currentState = READ_FRAME;
            break;
        }

        if (sample) {
            //Serial.println("Timeout occurred! Switching to READ_SENSORS");
            sample = false;
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