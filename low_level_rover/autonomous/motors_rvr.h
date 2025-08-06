#ifndef __CAR_CONTROL_H__
#define __CAR_CONTROL_H__

#include "PlatformOps.h"
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
int parser_motors(const Frame * p_frm);
bool motors_in_motion(void);
#endif // __CAR_CONTROL_H__

