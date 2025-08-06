#include "SoftPWM.h"
#include "soft_servo.h"
#include "parser.h"
#include "analog_inputs.h"
#include "bmi323.h"
#include "fsm_rvr.h"
#include "PlatformOps.h"

SoftServo servo;



void setup() {
  pinMode(8, OUTPUT); 
  SoftPWMBegin();  
  servo.attach(SERVO_PIN);
  servo.write(90);
  timer1_setup();
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000);
    // Initialize LED for status indication
  pinMode(LED_BUILTIN, OUTPUT);
  init_fsm();

 //Initialize BMI323 sensor
     Serial.println("\nInitializing BMI323 sensor...");
    if (bmi323_init()) {
        Serial.println("✓ BMI323 sensor initialized successfully!");
        
        //Print sensor information
        Serial.print("Chip ID: 0x");
        Serial.println(bmi323_get_chip_id(), HEX);
        Serial.println("I2C Address:0x68");
        
    } else {
        Serial.println("Failed to initialize BMI323 sensor!");        
        //Halt execution with error indication
        while (1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
    

}

void loop() {


encode_fsm();

  // if (Serial.available()) {
  //   char c = Serial.read();
  //   serialIn += c;
    
  //   if (serialIn.endsWith("\r\n")) {
  //     //Removes \r\n
  //     Serial.println("Received");
  //     Serial.println(serialIn);
  //     serialIn = serialIn.substring(0, serialIn.length() - 2);
      
    
  //     if (serialIn.length() > 0) {
  //       if(!scanner(serialIn)) 
  //         parser ();  // valid data received
        
  //         serialIn = "";
  //     }
  //   }
  // }
  // if(sample){
  //   sample = false;
  //   state = !state;
   
  //   distance_check();
  //   //digitalWrite(8, HIGH);
  //   bmi323_read_data_burst();
  //   //digitalWrite(8, LOW);

      
  // Serial.print("<ir,3,");
  // Serial.print((Sensor[0].raw));
  // Serial.print(",");
  // Serial.print((Sensor[1].raw));
  // Serial.print(",");
  // Serial.print((Sensor[2].raw));
  // Serial.print("><acc,3,");
  // Serial.print(sensor_data.acc_x);
  // Serial.print(",");
  // Serial.print(sensor_data.acc_y);
  // Serial.print(",");
  // Serial.print(sensor_data.acc_z);
  // Serial.print("><gyr,3,");
  // Serial.print(sensor_data.gyr_x);
  // Serial.print(",");
  // Serial.print(sensor_data.gyr_y);
  // Serial.print(",");
  // Serial.print(sensor_data.gyr_z);
  // Serial.print("><temp,1,");
  // Serial.print(sensor_data.temperature);
  //   Serial.print("><time,1,");
  // Serial.print(sensor_data.sensor_time);
  // Serial.println(">");


  // }
}
