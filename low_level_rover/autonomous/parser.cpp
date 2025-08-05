#include "HardwareSerial.h"
#include "parser.h"
#include "car_control.h"
#include "servo_control.h"
#include <math.h>
#include "fsm_rvr.h"
//#include "rvr_fsm.h"

/*
* INPUT: YY xxx xxx\r\n
*/

#define COMMAND_INDEX 0 // command index on the received data
#define COMMANDS 2  // number of commands

typedef enum {MOTORS = 0, SERVO} commands_receive;
String headers [] = {"ws", "ss"};   // YY

String data [MAX_PAYLOAD_NUM+2];

void clear_data (void);

extern bool StatusRunning;

typedef int (*func_ptr)(String*);

func_ptr parser_funcs [COMMANDS] = {
   parser_motors,
   parser_servo
};




/*
* Scans the message received and splits them in token
* to verify if the package is corret and to extract the speeds
* from left and right motors
*/
int scanner(String str) {
  str.trim();  

  int start = 0;
  int index = str.indexOf(' ');
  int i = 0;


  while ((i < MAX_PAYLOAD_NUM) && (index != -1)) {
    String parte = str.substring(start, index);
    if (parte.length() > 0) {
      data[i] = parte;
      i++;
    }
    start = index + 1;
    index = str.indexOf(' ', start);
  }

  // Last item (or only)
  if ((i < MAX_PAYLOAD_NUM) && (start < str.length())) {
    String lastItem = str.substring(start);
    if (lastItem.length() > 0) {
      data[i] = lastItem;
      i++;
    }
  }

  // Check if first element is a command
  for (int a = 0; a < COMMANDS; a++) {
    if (data[0] == headers[a]) {
       Serial.print("0: "); Serial.println(data[0]);
       Serial.print("1: "); Serial.println(data[1]);
       Serial.print("2: "); Serial.println(data[2]);
      return 0;
    }
  }
   
  // Comando inválido
  clear_data();
  Serial.println("err inv cmd");
  return -1;
}



int parser (void){
  int a;
  for (a = 0; a < COMMANDS; a++){
    if(data[0] == headers[a])
      break;
  }
  switch (a){
    case MOTORS:
      parser_funcs[MOTORS](data);
      clear_data();
      return 0;
    case SERVO:
      parser_funcs[SERVO](data);
      clear_data();
      return 0;
    default:
      clear_data();
      return -1;
  }
}

// Clear data string array
void clear_data (void){
  for (int a = 0; ((a < MAX_PAYLOAD_NUM+2-1) && data[a] != ""); a++)
    data[a]= "";
}

void print_frame(Frame* p_frm)
{
    Serial.println("Frame Parsed Successfully!");

    Serial.println("Frame Parsed:");
    Serial.print("CMD: "); Serial.println(p_frm->cmd);
    printCommandType(p_frm->type);
    Serial.print("LEN: "); Serial.println(p_frm->length);
    if (p_frm->length > 0) {
        Serial.print("VAL1: "); Serial.println(p_frm->val1);
        if (p_frm->length > 1) {
            Serial.print("VAL2: "); Serial.println(p_frm->val2);
        }

    }
   
    Serial.println();
}



String removeCRLF(const String& s) {
    String str = s;
    while (str.length() > 0 && (str.charAt(str.length() - 1) == '\n' || str.charAt(str.length() - 1) == '\r'))
        str.remove(str.length() - 1);
    return str;
}

String getToken(String& input, char delimiter) {
    int index = input.indexOf(delimiter);
    if (index == -1) { String token = input; input = ""; return token; }
    String token = input.substring(0, index);
    input = input.substring(index + 1);
    return token;
}

CommandType commandFromString(const String& cmd) {
    if (cmd.equalsIgnoreCase("start")) return CMD_START;
    if (cmd.equalsIgnoreCase("stop"))  return CMD_STOP;
    if (cmd.equalsIgnoreCase("ws"))    return CMD_WS;
    if (cmd.equalsIgnoreCase("ss"))    return CMD_SS;
    return CMD_UNKNOWN;
}

bool parseFrame(const String& rawInput, Frame& frame) {
    String inputFrame = removeCRLF(rawInput);
    if (inputFrame.length() < 4 || inputFrame.charAt(0) != '<' || inputFrame.charAt(inputFrame.length() - 1) != '>')
        return false;

    String content = inputFrame.substring(1, inputFrame.length() - 1);
    frame.cmd = getToken(content, ',');
    frame.type = commandFromString(frame.cmd);
    frame.length = getToken(content, ',').toInt();
    frame.val1 = (content.length() > 0) ? getToken(content, ',').toInt() : -1;
    frame.val2 = (content.length() > 0) ? content.toInt() : -1;
    return true;
}

int executeCommand(const Frame& f) {
    int ret = 0;
    Frame temp;
    float ms;
    // uint16_t reg;
    //Serial.println("Execute Command");
    switch (f.type) {
    case CMD_START: 
        TCCR1B=0;
        // Serial.println("CMD START");
        // Serial.println(f.length);
        // Serial.println(f.val1);

        if(f.length == 1){
          if(f.val1>=20 && f.val1 <=1000){
            ms = f.val1;
            ms = 15625 * ms/1000 -1;
            OCR1A = (uint16_t)ms;
            Serial.println(ms);
          }
        }
        //Serial.println("Action: System STARTED"); 
        ret = (int)RUNNING;
          // Modo CTC
        TCCR1B |= (1 << WGM12);
        // Prescaler 1024
        TCCR1B |= (1 << CS12) | (1 << CS10);
        StatusRunning = true;
        break;
    case CMD_STOP:  
        //Serial.println("Action: System STOPPED"); 
        ret = (int)READY;
        TCCR1B = 0;
        temp.cmd = CMD_WS;
        temp.val1=0;
        temp.val2=0;
        parser_motors(&temp);
        StatusRunning = true;
        break;

    case CMD_WS:    
        //Serial.print("Action: WS val1="); 
        //Serial.print(f.val1); 
        //Serial.print(" val2="); 
        //Serial.println(f.val2);
        if(StatusRunning){
          parser_motors(&f);
          ret = RUNNING;
        }
        else
        ret =READY;
        break;
    case CMD_SS:    
        Serial.print("Action: SS val1="); 
        Serial.println(f.val1); 
        ret = RUNNING;
        if(StatusRunning){
          parser_servo_cam(&f);
          ret=RUNNING;
        }else
        ret=READY;
        break;
    default:        
        Serial.println("Action: UNKNOWN command");
    }

    return ret;
}

// --- Helper to print command type ---
void printCommandType(CommandType type) {
    switch (type) {
    case CMD_START: Serial.println("Type: START"); break;
    case CMD_STOP:  Serial.println("Type: STOP");  break;
    case CMD_WS:    Serial.println("Type: WS");    break;
    case CMD_SS:    Serial.println("Type: SS");    break;
    default:        Serial.println("Type: UNKNOWN");
    }
}
