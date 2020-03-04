#include <Arduino.h>
#include "PVP_Status.h"
#include "PVP_IO.h"
#include "PVP_Actuator.h"
#include "PVP_Timer.h"


//Switch-case for vault states
enum STATES_LID{STATE_UNLOCKING=0,STATE_LOCKING,STATE_OPENED,STATE_CLOSED,STATE_LOCKED,STATE_QUALIFIER};
int box_state = STATE_UNLOCKING; // Starting state for the vault at power up- current but due to change

int package = false;


void Actuator_Tasker(){
    switch (box_state){
		case STATE_UNLOCKING:
		// Unlocked with package
		if (package = true && TimeReached(tUnlock, RELAY_INTERVAL))
		{
			Serial.println("Ready for retrieval");
		// Stops actuator power
			RELAY_UNLOCK_OFF();
			changeState(STATE_QUALIFIER);
		break;
		}
		// Unlocked and no package
		else 
		{
			if (package == false && TimeReached(tUnlock, RELAY_INTERVAL)) 
			{
				Serial.println("Ready for delivery");
		// Stops actuator power
				RELAY_UNLOCK_OFF();
				Serial.println("Unlock relay turned off");
				changeState(STATE_CLOSED);
		break;
			}
		}

		if (TimeReached(tResponse, RESPONSE_INTERVAL)){
			Serial.println("Now Unlocking");
		}
		// Starts actuator power for unlock
		//Serial.println("Relay activated");
		RELAY_UNLOCK_ON();
		

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_UNLOCKING);
		
		break;

		case STATE_LOCKING:
		// Completely locked
		if (TimeReached(tLock, RELAY_INTERVAL))
		{
			// Stops actuator power
			RELAY_LOCK_OFF();
			Serial.println("Vault is locked!");
			changeState(STATE_LOCKED);
			break;
		}

		// Starts actuator power / keep locking
		RELAY_LOCK_ON();

		//Serial.println("Now Locking");

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_LOCKING);
		
		break;

		case STATE_CLOSED:
		//Serial.println("Entered STATE_CLOSED");
		// Just opened and debounced
		if (LID_SWITCH_ACTIVE() && TimeReached(tDebounce, DEBOUNCE_INTERVAL))
		{
			Serial.println("Lid Was Opened!");
			changeState(STATE_OPENED);
			break;
		}
		// Package arrived and lockout timer expired
		else if (package == true && TimeReached(tLockdown, LOCKDOWN_INTERVAL))
		{
			Serial.println("Lid closed and timed out with package");
			changeState(STATE_LOCKING);
			break;
		}

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_CLOSED_COUNTING);
		
  		break;

		case STATE_OPENED:
		// Just closed and debounced
		if (!LID_SWITCH_ACTIVE() && TimeReached(tDebounce, DEBOUNCE_INTERVAL))
		{
			Serial.println("Lid Was Closed!");
			changeState(STATE_CLOSED);
			break;
		}
		// Lid open for too long
		else if (TimeReached(tAjar, LID_OPEN_INTERVAL))
		{
			Serial.println("I've been left AJAR");
		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_AJAR_ERROR);
		break;
		}

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_OPEN);
		

		// Assume package arrived
		package = true;
		break;

		case STATE_LOCKED:
		// Unlock in case of PANIC tripped
		PanicSensorCheck();
		// Unlock in case of KEYPAD input tripped
		KeypadCheck();

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_LOCKED); // Red solid
		

		break;

		case STATE_QUALIFIER:
		Serial.println("Qualifier!");
		// Just opened and debounced with package
		if (TimeReached(tDebounce, DEBOUNCE_INTERVAL) && LID_SWITCH_ACTIVE()  && package == true)
		{
			Serial.println("_____________________________________________________________");
			Serial.println("Package Retrieved");
			Serial.println("Package state has been reset to false");
			Serial.println("When lid is closed, we will be ready for the next delivery!");
			Serial.println("_____________________________________________________________");
			package = false;
		}
		// Closed and debounced with no package
		else if (!LID_SWITCH_ACTIVE() && TimeReached(tDebounce, DEBOUNCE_INTERVAL) && package == false)
		{
			changeState(STATE_CLOSED);
		}

		// Lid open for too long
		if (TimeReached(tAjar, LID_OPEN_INTERVAL))
		{
			Serial.println("Lid was left AJAR");
			// Set NEOPixel status light
			changeStatus(FEEDBACK_STATUS_AJAR_ERROR); // Blue blinking
		
		}

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_READY_RETRIEVE); // Yellow blinking
		
		break;

	}



}

void PanicSensorCheck(){

bool pirState = 0;         // current state of the button
bool lastPIRState = 0;     // previous state of the button

	// read the pushbutton input pin:
  	pirState = digitalRead(PANIC_PIR_SNSR);
  	// compare the buttonState to its previous state
  	if (pirState != lastPIRState) {
    // if the state has changed, increment the counter
    if (pirState == 1 && TimeReached(tPircheck, DEBOUNCE_INTERVAL)) {
		changeState(STATE_UNLOCKING);
		Serial.println("PIR was triggered");
		}
    } 
  
  // save the current state as the last state, for next time through the loop
  lastPIRState = pirState;

}

void KeypadCheck(){

bool keypadState = 0;         // current state of the button
bool lastKEYPADState = 0;     // previous state of the button

	// read the pushbutton input pin:
  	keypadState = digitalRead(KEYPAD_TRIGGER);
  	// compare the buttonState to its previous state
  	if (keypadState != lastKEYPADState) {
    // if the state has changed, increment the counter
    if (keypadState == 1 && TimeReached(tKeycheck, DEBOUNCE_INTERVAL)) {
		changeState(STATE_UNLOCKING);
		Serial.println("KEYPAD was triggered");
		}
    } 

  // save the current state as the last state, for next time through the loop
  lastKEYPADState = keypadState;

}

/*****************************************************************************************************************************
* FSM Change State Function
**************************************************************************************************************************** */
void changeState(int new_state){
	
	box_state = new_state;
	//box_timer = 0;
	//debounce_timer = 0;


}
// Time elapsed function that updates the time when true
bool TimeReached(uint32_t tSaved, uint32_t ElapsedTime){
  if(abs(millis()-tSaved)>=ElapsedTime){ tSaved=millis();
    return true;
  }
  return false;
}