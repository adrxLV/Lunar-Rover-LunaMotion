#include <SoftPWM.h>
#include "car_control.h"
#include "driverRasp.h"

String serialIn = "";

SunFounder_cfg left_Motor = {.duty = 0,
                             .forward_pin = 2,
                             .backward_pin = 3};
SunFounder_cfg right_Motor = {.duty = 0,
                              .forward_pin = 5,
                              .backward_pin = 4};
stringInfo package= {.header = 's',
                     .speedLeft = "0",
                     .speedRight = "0"};

void setup() {
  SoftPWMBegin();  
  Serial.begin(115200);
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    serialIn += c;
    
    if (serialIn.endsWith("\r\n")) {
      // Removes \r\n
      serialIn = serialIn.substring(0, serialIn.length() - 6);
      
      if (serialIn.length() > 0) {
        scanner(serialIn, &package);
        car_update(&package, &left_Motor, &right_Motor);
        serialIn = "";
      }
    }
  }
}
