#ifndef __CAR_CONTROL_H__
#define __CAR_CONTROL_H__

#include <Arduino.h>
#include <stdint.h>

#include "parser.h"
/*
 *  [0]--|||--[1]
 *   |         |
 *   |         |
 *  [0]       [1]
 *   |         |
 *   |         |
 *  [0]-------[1]
 */

int parser_motors(String* );
int parser_motors(Frame * p_frm);
#endif // __CAR_CONTROL_H__

