#include "Arduino.h"
#include "sendRasp.h"

#define MAX_COMMANDS_SEND 3

String header_send [] = {"ir", "gir", "acl"};

int send_to_rasp(commands_send command, long int* data, int len) {
  if (command < MAX_COMMANDS_SEND) {
    Serial.print(header_send[command]); 
    Serial.print(" ");
    for (int i = 0; i < len; i++) {
      Serial.print(data[i]);
      Serial.print(" ");
    }
    Serial.println();
    return 1;
  }
  return 0; // Invalid command
}


