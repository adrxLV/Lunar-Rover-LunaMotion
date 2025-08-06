#ifndef __PARSER_MOD__
#define __PARSER_MOD__
#include "Arduino.h"
#include <stdint.h>


// --- Enum for known commands ---
typedef enum  {
    CMD_UNKNOWN,
    CMD_START,
    CMD_STOP,
    CMD_WS,
    CMD_SS   
}CommandType;


// --- Frame structure ---
typedef struct  {
    CommandType type;
    String cmd;
    int length;
    int val1;
    int val2;
}Frame;

#define MAX_PAYLOAD_NUM 3

int scanner(String str);

int parser (void);


bool parseFrame(const String& inputFrame, Frame& frame);

int executeCommand(const Frame& f);

void print_frame(Frame* p_frm);
void printCommandType(CommandType type);

#endif
