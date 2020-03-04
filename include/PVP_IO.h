#ifndef PVP_IO_H
#define PVP_IO_H

#include <Arduino.h>

// I/O
#define RELAY_LOCK_PIN		26	// Unlock Relay
#define RELAY_UNLOCK_PIN	25	// Lock Relay
#define INTERIOR_LIGHTS_PIN	27	// Interior lighting ring for camera and panic functions USING NEOPIXEL LIGHT ON CONTROLLER LID FOR INTERIOR LIGHTING FOR NOW
#define AUX_RELAY_PIN		12  // Aux relay - spare, for future development?
#define LID_SWITCH			4	// Mag switch on lid
#define PANIC_PIR_SNSR		22	// Passive Infrared sensor for panic release **************************change this pin, is not on this pin***************************
#define KEYPAD_TRIGGER		14	// Latching pushbutton for  panic release, backlit

// ****INPUTS ****INPUTS  ****INPUTS  ****INPUTS  ****INPUTS  ****INPUTS
//_______________________________________________________________________

//Switches or Buttons -  
#define LID_SWITCH_INIT() 		 	pinMode(LID_SWITCH, INPUT_PULLUP)
#define LID_SWITCH_ACTIVE()      	digitalRead(LID_SWITCH)
#define PANIC_PIR_SNSR_INIT()	 	pinMode(PANIC_PIR_SNSR, INPUT)
#define PANIC_PIR_SNSR_ACTIVE()  	digitalRead(PANIC_PIR_SNSR)
#define KEYPAD_TRIGGER_INIT()	 	pinMode(KEYPAD_TRIGGER, INPUT_PULLUP)
#define KEYPAD_TRIGGER_ACTIVE()  	digitalRead(KEYPAD_TRIGGER)
			     	

// ****OUTPUTS  ****OUTPUTS  ****OUTPUTS  ****OUTPUTS  ****OUTPUTS  ****OUTPUTS
//______________________________________________________________________________


//RELAYS
//#define ON_LOGIC_LEVEL HIGH  //Opened when LOW

#define RELAY_LOCK_INIT()      	pinMode(RELAY_LOCK_PIN,OUTPUT)
#define RELAY_LOCK_START()	 	digitalWrite(RELAY_LOCK_PIN,LOW)
//#define RELAY_LOCK_ONOFF()     	!digitalRead(RELAY_LOCK_PIN) //opened when LOW
#define RELAY_LOCK_ON()       	digitalWrite(RELAY_LOCK_PIN,HIGH) //opened when LOW
#define RELAY_LOCK_OFF()      	digitalWrite(RELAY_LOCK_PIN,LOW) //opened when LOW

#define RELAY_UNLOCK_INIT()      	pinMode(RELAY_UNLOCK_PIN,OUTPUT)
#define RELAY_UNLOCK_START()	   	digitalWrite(RELAY_LOCK_PIN,LOW)
//#define RELAY_UNLOCK_ONOFF()    !digitalRead(RELAY_UNLOCK_PIN) //opened when LOW
#define RELAY_UNLOCK_ON()        	digitalWrite(RELAY_UNLOCK_PIN,HIGH) //opened when LOW
#define RELAY_UNLOCK_OFF()       	digitalWrite(RELAY_UNLOCK_PIN,LOW) //opened when LOW

#define INTERIOR_LIGHTS_INIT()    pinMode(INTERIOR_LIGHTS_PIN,OUTPUT)
#define INTERIOR_LIGHTS_START()	  digitalWrite(RELAY_LOCK_PIN,LOW)
//#define INTERIOR_LIGHTS_ONOFF() !digitalRead(INTERIOR_LIGHTS_PIN) //opened when LOW
#define INTERIOR_LIGHTS_ON()      digitalWrite(INTERIOR_LIGHTS_PIN,HIGH) //opened when LOW
#define INTERIOR_LIGHTS_OFF()     digitalWrite(INTERIOR_LIGHTS_PIN,LOW) //opened when LOW


#define AUX_RELAY_PIN_INIT()    pinMode(AUX_RELAY_PIN,OUTPUT)
#define AUX_RELAY_PIN_START()	  digitalWrite(RELAY_LOCK_PIN,LOW)
//#define AUX_RELAY_PIN_ONOFF() !digitalRead(AUX_RELAY_PIN) //opened when LOW
#define AUX_RELAY_PIN_ON()      digitalWrite(AUX_RELAY_PIN,HIGH) //opened when LOW
#define AUX_RELAY_PIN_OFF()     digitalWrite(AUX_RELAY_PIN,LOW) //opened when LOW


#endif //PVP_IO.h
