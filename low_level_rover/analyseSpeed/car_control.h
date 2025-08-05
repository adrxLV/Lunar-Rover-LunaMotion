#ifndef __CAR_CONTROL_H__
#define __CAR_CONTROL_H__

#include <Arduino.h>
#include <stdint.h>

#include "driverRasp.h"
/*
 *  [0]--|||--[1]
 *   |         |
 *   |         |
 *  [0]       [1]
 *   |         |
 *   |         |
 *  [0]-------[1]
 */

#define NUM_PWMS 13

typedef struct{
  uint8_t duty;
  int8_t forward_pin;
  int8_t backward_pin;
}SunFounder_cfg;

void move(SunFounder_cfg *motor0, SunFounder_cfg *motor1);
int round_duty(String number);
void car_update(stringInfo *cmd, SunFounder_cfg *motor0, SunFounder_cfg *motor1);
const uint8_t pwm_LEFT [NUM_PWMS] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60 };  //add duty_cycles
const uint8_t pwm_RIGHT [NUM_PWMS] = {0, 5, 10, 21, 27, 33, 40, 44, 48, 56, 63, 72, 76};  //add duty_cycles

#endif // __CAR_CONTROL_H__

