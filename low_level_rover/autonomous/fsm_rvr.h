#ifndef __FSM_RVR_MOD__
#define __FSM_RVR_MOD__


typedef enum  {
    READY,
    READ_FRAME,
    RUNNING,
    READ_SENSORS
}FSMState;

void init_fsm(void);

void encode_fsm();

#endif