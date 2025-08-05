#include "car_control.h"
#include "driverRasp.h"

#include <SoftPWM.h>
#include <math.h>


/*
Scans the message received and splits them in token
to verify if the package is corret and to extract the speeds
from left and right engine
*/
void scanner(String str, stringInfo *cmd) {
  str.trim();  // remove spaces in the beginning and end
  
  String parts[3];
  int start = 0;
  int index = str.indexOf(' ');
  int i = 0;
  
  while (index != -1 && i < 3) {  
    String parte = str.substring(start, index);
    if (parte.length() > 0) {  
      parts[i] = parte;
      i++;
    }
    start = index + 1;
    index = str.indexOf(' ', start);
  }
  
  //last part
  String ultimaParte = str.substring(start);
  if (ultimaParte.length() > 0) {
    parts[i] = ultimaParte;
    i++;
  }
  cmd->header = parts[0].charAt(0);
  cmd->speedLeft = parts[1];
  cmd->speedRight = parts[2];
}



