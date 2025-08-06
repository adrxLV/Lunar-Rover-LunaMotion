#include "motors_rvr.h"
#include <math.h>
#include "parser.h"
#include "PlatformOps.h"

#define NUM_PWMS 13
#define MOTORS_PAYLOAD_NUM 2
#define PWM_MIN 0
#define PWM_MAX 255

typedef struct{
  float pwm;
  int8_t forward_pin;
  int8_t backward_pin;
}SunFounder_cfg;

typedef struct{
  float speedLeft;
  float speedRight;
}payload_motors;
static bool MotorRunning;
payload_motors payload;

SunFounder_cfg left_Motor = {.pwm = 0.0,
                            .forward_pin = 2,
                            .backward_pin = 3};
SunFounder_cfg right_Motor = {.pwm = 0.0,
                             .forward_pin = 5,
                             .backward_pin = 4};

/*
* <<<<<<<<<<<<<<<<<<<<<<<<< IMPORTANT INFORMATION >>>>>>>>>>>>>>>>>>>>>>>>>>
* Values received for duty-cycle can be floats and must be between 0 and 255
*/

/*
* Updates motor Variables and updates movement
*/
void car_update(void){
  left_Motor.pwm = payload.speedLeft;
  right_Motor.pwm = payload.speedRight;
  /*
  Serial.print("pwm L: "); Serial.println(payload.speedLeft);
  Serial.print("pwm R: "); Serial.println(payload.speedRight);
  */

  //left wheel
  if(left_Motor.pwm >= 0) {
   SoftPWMSet(left_Motor.forward_pin, left_Motor.pwm);
   SoftPWMSet(left_Motor.backward_pin, 0);
  } else {
   SoftPWMSet(left_Motor.forward_pin, 0);
   SoftPWMSet(left_Motor.backward_pin, abs(left_Motor.pwm));
  }
  //right wheel
  if(right_Motor.pwm > 0) {
   SoftPWMSet(right_Motor.forward_pin, abs(right_Motor.pwm));
   SoftPWMSet(right_Motor.backward_pin, 0);
  } else {
   SoftPWMSet(right_Motor.forward_pin, 0);
   SoftPWMSet(right_Motor.backward_pin, abs(right_Motor.pwm));
  }
  
}

int parser_motors(String *data){
  if(data[MOTORS_PAYLOAD_NUM+1] == ""){ // Check the payload length
    payload.speedLeft = data[1].toFloat();
    payload.speedRight = data[2].toFloat();
    // Check if received data is between the limits accepted
    if((payload.speedLeft < PWM_MIN) || (payload.speedLeft > PWM_MAX ) ||
      (payload.speedRight < PWM_MIN) || (payload.speedRight > PWM_MAX))
        return -1;
      
    car_update();
    return 0;
  }else
    return -1;    // more payloads than required
}

int parser_motors(const Frame * p_frm){

    if(abs(p_frm->val1) > PWM_MAX || abs(p_frm->val2) > PWM_MAX )
        return -1;
    payload.speedLeft = (float)p_frm->val1;//.toFloat();
    payload.speedRight = (float) p_frm->val2;//.toFloat();

    if (p_frm->val1 > 0 || p_frm->val2 > 0) {
        MotorRunning = true;
    }
    else
        MotorRunning = false;

    car_update();
    return 0;

}

bool motors_in_motion(void)
{
    return MotorRunning;
}
