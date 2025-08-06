#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "SoftPWM.h"

void timer1_setup (void);

void timer1_start(void);

void timer1_start(unsigned int time_ms);

void timer1_stop(void);

void readSensors();

bool handleSerialInput(String& serialIn);

void check_receiving_speed_frames(void);

void ack_receiving_speed_frames(void);

extern bool SampleSensors;
