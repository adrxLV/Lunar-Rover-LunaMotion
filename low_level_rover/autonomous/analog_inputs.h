#ifndef ANALOG_INPUTS_H
#define ANALOG_INPUTS_H

typedef struct{
  uint8_t pin;
  long int raw;

} analog_sense_t;
void distance_check(void);

#endif
