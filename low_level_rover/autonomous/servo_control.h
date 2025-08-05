#ifndef __SERVO_CONTROL_H__
#define __SERVO_CONTROL_H__
#include "Arduino.h"
#include "soft_servo.h"

#include "parser.h"
extern SoftServo servo;

int parser_servo(String* data);

int parser_servo_cam(Frame * p_frm);

#endif // __SERVO_CONTROL_H__

