#ifndef __SEND_RASP_H__
#define __SEND_RASP_H__

typedef enum {IR = 0, GIR, ACL} commands_send;

int send_to_rasp (commands_send command, long int* data, int len);

#endif // __SEND_RASP_H__