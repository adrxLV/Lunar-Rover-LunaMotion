#ifndef __DRIVERRASP_H__
#define __DRIVERRASP_H__

#include <Arduino.h>
#include <stdint.h>

typedef struct{
  char header;
  String speedLeft;
  String speedRight;
}stringInfo;

void scanner(String str, stringInfo *cmd);
#endif // __DRIVERRASP_H__
