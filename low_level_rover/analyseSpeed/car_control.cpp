#include "car_control.h"
#include <SoftPWM.h>
#include <math.h>

/*
function that rounds duty-cycles to multiples of 5
next step is to recieve speed and calculate duty-cycle
*/
int round_duty(String number) {
  float duty = number.toFloat();
  int ret = round(duty / 5.0) * 5;
  return min(ret, 100);
}

/*
acts on engines
*/
void move(SunFounder_cfg *motor0, SunFounder_cfg *motor1){
  //left wheel
  SoftPWMSetPercent(motor0->forward_pin, motor0->duty);
  SoftPWMSetPercent(motor0->backward_pin, 0);
  //right wheel
  SoftPWMSetPercent(motor1->forward_pin, motor1->duty);
  SoftPWMSetPercent(motor1->backward_pin, 0);

}

/*
checks header and updates movement
*/
void car_update(stringInfo *cmd, SunFounder_cfg *motor0, SunFounder_cfg *motor1){
  if(cmd->header == 's'){
    int duty_rounded_l = round_duty(cmd->speedLeft);
    int duty_rounded_r = round_duty(cmd->speedRight);

    for (int i = 0; i < NUM_PWMS; i++) {
      if (duty_rounded_l == pwm_LEFT[i]) {
        motor0->duty = pwm_LEFT[i];
        break;
      }
    }

    for (int i = 0; i < NUM_PWMS; i++) {
      if (duty_rounded_r == pwm_LEFT[i]) {
        motor1->duty = pwm_RIGHT[i];
        break;
      }
    }
    move(motor0, motor1);
  }
}

