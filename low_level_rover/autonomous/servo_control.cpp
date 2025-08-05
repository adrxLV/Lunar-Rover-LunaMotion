#include "servo_control.h"

#define SERVO_PAYLOAD_NUM 1
#define MIN_ANGLE 10
#define MAX_ANGLE 130

int parser_servo_cam(Frame * p_frm){
  if(p_frm->val1 >=MIN_ANGLE && p_frm->val1 < MAX_ANGLE){
    servo.write((uint8_t)p_frm->val1);
    return 0;
  
}else{
  Serial.println("<err,1,ang>");
  return -1;
}
}

int parser_servo(String* data){
  if(data[SERVO_PAYLOAD_NUM+1] == ""){ // Check the payload length
    uint8_t angle =data[1].toInt();
    //Serial.print("Angle is ");
    //Serial.print(angle);
    //Serial.println();
    if(angle >= MIN_ANGLE && angle <= MAX_ANGLE){
      servo.write(angle);
      return 0;
    }else{
      // Serial.print("Angle offlimits: <");
      // Serial.print(MIN_ANGLE);
      // Serial.print(",");
      // Serial.print(MAX_ANGLE);
      // Serial.println(">");
      Serial.print("<err 1 ang>");
      Serial.print(MIN_ANGLE);
      Serial.print(" ");
      Serial.println(MAX_ANGLE);
    }
  }
  return -1;    // more payloads than required
}