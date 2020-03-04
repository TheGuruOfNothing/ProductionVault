#ifndef PVP_STATUS_H
#define PVP_STATUS_H
#endif
#ifndef PVP_IO_H
#define PVP_IO_H
#endif
#ifndef PVP_ACTUATOR.H
#define PVP_ACTUATOR.H
#endif
#ifndef PVP_TIMER.H
#define PVP_TIMER.H
#endif

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






