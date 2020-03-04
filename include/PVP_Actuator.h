
#ifndef PVP_ACTUATOR.H
#define PVP_ACTUATOR.H

#include <Arduino.h>


//Function prototypes for Actuator_Tasker.cpp
void changeState(int new_state);
void Actuator_Tasker(void);
void PanicSensorCheck();
void KeypadCheck();
void changeStatus(int new_status);
bool TimeReached(uint32_t tSaved, uint32_t ElapsedTime);

uint32_t tUnlock = 0;
uint32_t tLock = 0;
uint32_t tResponse = 0;
uint32_t tDebounce = 0;
uint32_t tLockdown = 0;
uint32_t tAjar = 0;
uint32_t tKeycheck = 0;
uint32_t tPircheck = 0;

#endif //PVP_Actuator.h






