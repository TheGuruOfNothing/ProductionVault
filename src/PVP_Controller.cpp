#include <Arduino.h>
#include "PVP_Controller.h"



/* >>>>>>>>>>>>>>>>>>>>>>>>>>     MAIN CODE HERE      >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
//*******************************************************************************************************/

void setup() {
Serial.begin(74480);

	//lid_init(); not used at the moment, commented out 
	box_init();

}

void loop() {

	Tasker_Lid();

}

/* >>>>>>>>>>>>>>>>>>>>>>>>>     FUNCTIONS HERE      >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
//*******************************************************************************************************/

void box_init(){


	// Examples of new logging method


	AddSerialLog_P(LOG_LEVEL_INFO, PSTR("Initializing Box"));    //PSTR saves the string in progmem, since it doesnt change, good save spacing, get into this habit



	AddSerialLog_P(LOG_LEVEL_DEBUG, PSTR("Initializing Box version %d"),2);    //PSTR saves the string in progmem, since it doesnt change, good save spacing, get into this habit
	AddSerialLog_P(LOG_LEVEL_DEBUG, PSTR("Initializing Box version %c"),'2');    //PSTR saves the string in progmem, since it doesnt change, good save spacing, get into this habit
	AddSerialLog_P(LOG_LEVEL_DEBUG, PSTR("Initializing Box version %s %s $d"),"or like", "this", 3);    //PSTR saves the string in progmem, since it doesnt change, good save spacing, get into this habit

	// If you look at my function, the "..." means it accepts AS MANY extra terms as you want eg "or like", "this", comma with each


															// But never FLOATS, they wont work on esp due to lib restrictions
	//AddSerialLog_P(LOG_LEVEL_DEBUG, PSTR("Initializing Box version %f"),WRONG);    //PSTR saves the string in progmem, since it doesnt change, good save spacing, get into this habit


	// IF you use the wrong indentifier .... the program WILL compile and flash and WILL CRASH!!!!
	/***
	 * %s = string eg "hello"
	 * %c = char and only one eg 'c'    // notice single quotes
	 * %d = number (but not float)
	 * */


	STATE_ONE_LED_INIT(); STATE_ONE_LED_START();
	STATE_TWO_LED_INIT(); STATE_TWO_LED_START();
	STATE_THREE_LED_INIT(); STATE_THREE_LED_START();
	STATE_FOUR_LED_INIT(); STATE_FOUR_LED_START();
	//STATE_FIVE_LED_INIT(); STATE_FIVE_LED_START();
	
}

void lid_init(){
	Serial.println("Initializing Lid Swich");
	LID_SWITCH_INIT();
}


// Read lid position and set output
void Tasker_Lid(){

	// Read the state, named like this, because a future idea could be a linear distance measurement ie not completely closed or open, but transition state with position
	SubTask_ReadLidState_OpenClosed();
	SubTask_TimerLidState(); // Add timers that tick down regardless of state for security reasons


	switch (box_state){
		case STATE_CLOSED: 
			// if(unlocked)
				// lock it
			// if (locked)
				// perhaps flash neoled every 10 seconds to show its locked
		break;
		case STATE_OPENING:

			// example, set open time to max of 60 seconds .... you could add a button/motion detector inside the box that reset this again 
			lid_opened_timeout_secs = 60;

			//set neopixel led			
			// notif.pixel[0].period_ms = 500; // 0.25 second between "on"s, so quarter second toggling
			// notif.pixel[0].mode = NOTIF_MODE_PULSING_ON_ID;
			// notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];

		
		break;
		case STATE_UNKNOWN: // auto close if we don't know, you can set this as a "timeout" option.. note, not "break" so this automatically runs into closing
		case STATE_CLOSING: 

			lid_opened_timeout_secs = 0; // disable/reset
		
		
		break;
		case STATE_OPENED:



			
		break;
		default:
			Serial.println("You screwed up big time buddy... fix your friggen actuator code!");
		break;
	}
}

// void Actuator_Subtask(){
// 	if(box_state!=old_box_state){ //if the two don't match
//     Serial.print("Currently in -"); Serial.println(box_state); //Then print "currently in - X box state" in serial
//     old_box_state = box_state; //change old box state to match current state - which SHOULD stop a serial print loop because the statement is no longer true, i.e. box state and old box state are now the same
// 	}

// 	if(box_state==STATE_LOCKING){
// 		STATE_TWO_LED_ON();
// 	}else{
// 		STATE_TWO_LED_OFF();
// 	}

// 	if(box_state==STATE_OPENED){
// 		STATE_THREE_LED_ON();
// 	}else{
// 		STATE_THREE_LED_OFF();
// 	}

// 	if(box_state==STATE_UNLOCKING){
// 		STATE_FOUR_LED_ON();
// 	}else{
// 		STATE_FOUR_LED_OFF();
// 	}
// }

void SubTask_ReadLidState_OpenClosed(){

	if ((LID_SWITCH_ONOFF()!=lid_switch.state)&&(TimeReached(&tDebounce, DEBOUNCE_INTERVAL)))
	{
		lid_switch.state = LID_SWITCH_ONOFF();
		if(lid_switch.state)
		{
			Serial.print("Active high lid_switch");
			lid_switch.isactive = true;
			lid_switch.tDetectTimeforDebounce = millis();
		}else
		{  
			Serial.print("Active low lid_switch");
			lid_switch.isactive = false;
		}
	lid_switch.ischanged = true;
	}

}

void SubTask_TimerLidState(){

	// you can use this 1 second timer for many different tickers (1 second timeouts)
  if(TimeReached(&tSavedTimerLidStateTicker,1000)){

	  //optional idea
	//#define ENABLE_NEO_TIMEOUT_INDICATOR //just here as I test
	#ifdef ENABLE_NEO_TIMEOUT_INDICATOR
	if(lid_opened_timeout_secs < 10){
		notif.pixel[0].period_ms = map(lid_opened_timeout_secs,0,10,20,200); // maps the last 10 seconds into 20-200ms blinking .. play with the numbers 
		notif.pixel[0].mode = NOTIF_MODE_BLINKING_ON_ID;
		notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
	}
	#endif

	
	// count down
    if(lid_opened_timeout_secs>1){ 
      lid_opened_timeout_secs--;
      AddSerialLog_P(LOG_LEVEL_INFO, PSTR("lid_opened_timeout_secs = %d"),lid_opened_timeout_secs);
    }else 
	if(lid_opened_timeout_secs==1){ // at one second left, react by closing or set warning or whatever
      lid_opened_timeout_secs = 0; // setting to 0 will disable the timer
	  
      AddSerialLog_P(LOG_LEVEL_ERROR, PSTR("lid_opened_timeout_secs = %d, TIMEOUT reached"),lid_opened_timeout_secs);

	  box_state = STATE_CLOSING;

	  
    }

  }


}



// // Read lid position and set output
// void Tasker_Lid(){

// 	// Read the state, named like this, because a future idea could be a linear distance measurement ie not completely closed or open, but transition state with position
// 	SubTask_ReadLidState_OpenClosed();


// 	switch (box_state){
// 		case STATE_CLOSED: 
// 		// if (TimeReached(&tShift, SHIFT_INTERVAL)){
			
// 		// 	box_state = STATE_OPENED;
// 		// }
// 		break;
// 		case STATE_OPENED:
// 		// if (TimeReached(&tShift, SHIFT_INTERVAL)){
// 		// 	box_state = STATE_LOCKING;
// 		// }
// 		break;
// 		case STATE_LOCKING: 
// 		// if (TimeReached(&tShift, SHIFT_INTERVAL)){
// 		// 	box_state = STATE_UNLOCKING;
// 		// }
// 		break;
// 		case STATE_UNLOCKING:
// 		// if (TimeReached(&tShift, SHIFT_INTERVAL)){
// 		// 	box_state = STATE_CLOSED;
// 		// }
// 		break;
// 		default:
// 			Serial.println("You screwed up big time buddy... fix your friggen actuator code!");
// 		break;
// 	}
// }


// void Lid_Tasker(){ 

// 	switch (lid_state){
// 		case LID_CHECK_LOOP:
// 		break;
// 		case LID_AJAR:
// 		//DO SOMETHING HERE TO CHECK IF THE LID WAS LEFT OPEN TOO LONG THEN RESPOND TO IT
// 		if (lid_switch.isactive==true && TimeReached(&tAjar, LID_AJAR_INTERVAL)){
// 			//ASSUME THAT I NEED TO RESET tAjar TO MILLIS() HERE
// 			//BLINK BETWEEN BLUE AND RED RAPIDLY
// 		}else{
// 			//not sure if this is needed yet or what to do with it
// 		}
// 		break;
// 		default:
// 			Serial.println("You screwed up big time buddy... fix your friggen lid code!");
// 		break;
		
// 	}
// }

// Time elapsed function that updates the time when 
uint8_t TimeReached(TIMER_HANDLER* tSaved, uint32_t ElapsedTime){
  if(
    (abs(millis()-tSaved->millis)>=ElapsedTime)
    ||(tSaved->run == true)    
    ){ 
      tSaved->millis=millis();
      tSaved->run = false;
    return true;
  }
  return false;
}

/*****************************************************************************************************************************
* Switch Case Function for Tasker_Lid 
**************************************************************************************************************************** *
void ChangeState(int new_state){
	box_state = new_state;
	//Serial.println(new_state);
}
***************************************************************************************************************************** */






















































/* *******The Package Vault Project***********
 I'd like to give a giant thank you to @Sparkplug23 for all his gracious and ongoing assistance
 and collaboration in this project. The majority of the code is his and he deserves the credit 
 for all his patience invested as well, as I am not the easiest dog to teach new tricks to. 

 Additional credit is owed to @Chris Aitken for his efforts also. The original core code was 
 his. I have used that as the foundation to the project and though it is now only a small piece
 of the large puzzle, it is what took the vault from a passing whimsy to a functional device. LOTS of
 upgrades have been built onto that code.

 The Package Vault Project was started as a personal project to try and stop package thieves
 from taking my packages. After falling victim, I decided enough was enough. This device
 was born. It's popularity amongst the neighbors and local community has made it take on a life of it's own.
 It is my hope that you will take the code that has been created and figure out how to integrate it
 into something new and cool that has the same goal - to thwart package thieves. 

You can get more information on this project or on this code at www.packagevaultproject.org
-------------------------------------------------------------------------------------------------------------
VaultController is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

VaultController is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with VaultController.  If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------ 
*/

/*#include <Arduino.h>
#include <NeoPixelBus.h>
#include "PVP_Controller.h"

#define BUILD_NUMBER_CTR "v0_53"

// Levels of stability (from testing to functional)
#define STABILITY_LEVEL_NIGHTLY     "nightly"     // testing (new code -- bugs) <24 hour stability
//#define STABILITY_LEVEL_PRE_RELEASE "pre_release" // under consideration for release (bug checking) >24 stability
//#define STABILITY_LEVEL_RELEASE     "release"     // long term >7 days

*/

/*****************************************************************************************************************************
 * SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        
 *****************************************************************************************************************************
 */

/*void setup()
{	
	Serial.begin(115200);

	RELAY_LOCK_INIT(); RELAY_LOCK_START(); //Start LOW
	RELAY_UNLOCK_INIT(); RELAY_UNLOCK_START(); //Start LOW
	INTERIOR_LIGHTS_INIT(); INTERIOR_LIGHTS_START(); //Start LOW
	AUX_RELAY_PIN_INIT(); AUX_RELAY_PIN_START(); //Start LOW
	LID_SWITCH_INIT();
	PANIC_PIR_SNSR_INIT();
	KEYPAD_TRIGGER_INIT();
	NEO_PIN_INIT();

	Serial.println("Init colormap in Setup");
	init_Colormap();
	

    stripbus = new NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod>(PIXEL_COUNT,PIXEL_PIN);
	stripbus->Begin();
	stripbus->ClearTo(0);
  	stripbus->Show();  // Initialize all pixels to 'off'


	notif.fForceStatusUpdate = true;
	  Serial.println("Strip cleared in setup...");

//NO LONGER A STRUCT
	// memset(&timervals,0,sizeof(timervals)); //Instantiating the timer struct
}*/

/******************************************************************************************************************************
 * LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      
 ******************************************************************************************************************************
 ******************************************************************************************************************************
 */

/*void loop(){

	//NeoStatus_Tasker(); // called always without delay
	Tasker_Lid();
	


    //Lets use this to trigger every 10 seconds. 
	// We will work on settings a notification pixel to blink for 6 seconds then turn itself off, 
	// repeating 4 seconds later when this fires again.
	//	if(TimeReached(&tSavedFeedbackDisplay,10000)){
	//	status = FEEDBACK_STATUS_READY; // forcing mode
	//	NEO_Feedback_Display();		
	//	Serial.println("NeoStatus_Tasker timer timed out and reset...");
	//}


}







void Tasker_Lid(){
	
	uint8_t package = false;

	if (TimeReached(&tStatusCheck, RESPONSE_INTERVAL3)){
			Serial.print("Package Status is... ");
			Serial.println(package);
			Serial.print("We're in box state...");
			Serial.println(box_state);
		}


    switch (box_state){
		case STATE_UNLOCKING: // %%%%%%%%% 0000000000000000000000000000000000000000000000000000000000000000000000000000000
		// Unlocked with package
		if (TimeReached(&tUnlock0, RELAY_INTERVAL) && (package == true))
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
			if (TimeReached(&tUnlock1, RELAY_INTERVAL)) 
			{
		// Stops actuator power
			RELAY_UNLOCK_OFF();
			Serial.println("Unlock relay turned off");
			Serial.println("Ready for delivery");
			changeState(STATE_CLOSED); // state 2
		break;
			}
		}

		
		if (TimeReached(&tResponse, RESPONSE_INTERVAL)){
			Serial.println("Now Unlocking");
		}
	

		// Starts actuator power for unlock
		RELAY_UNLOCK_ON();
		
			
		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_UNLOCKING);
		
		break;

		case STATE_LOCKING: //%%%%%%%%% 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
		// Completely locked
		if (TimeReached(&tLock0, RELAY_INTERVAL))
		{
			// Stops actuator power
			RELAY_LOCK_OFF();
			Serial.println("Vault is locked!");
			changeState(STATE_LOCKED);
			break;
		}

		// Starts actuator power / keep locking
		RELAY_LOCK_ON();

		if (TimeReached(&tResponse, RESPONSE_INTERVAL)){
			Serial.println("Now Locking");
		}

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_LOCKING);
		
		break;

		case STATE_CLOSED: //%%%%%%%%%% 22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222
		// Just opened and debounced
		if (LID_SWITCH_ONOFF() && TimeReached(&tDebounce, DEBOUNCE_INTERVAL))
		{
			Serial.println("Lid Was Opened!");
			changeState(STATE_OPENED); //state 3
			break;
		}
		// Package arrived and lockout timer expired
		else if (package == true && TimeReached (&tLockdown, LOCKDOWN_INTERVAL))
		{
			Serial.println("Lid closed and timed out with package");
			changeState(STATE_LOCKING);
			break;
		}

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_CLOSED_COUNTING);
		
  		break;

		case STATE_OPENED:// %%%%%%%%%%%% 3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333
		// Just closed and debounced
		if (!LID_SWITCH_ONOFF() && TimeReached(&tDebounce, DEBOUNCE_INTERVAL))
		{
			Serial.println("Lid Was Closed!");
			changeState(STATE_CLOSED); // state 2
			break;
		}
		// Lid open for too long
		else if (TimeReached(&tAjar, LID_OPEN_INTERVAL))
		{
			Serial.println("I've been left AJAR");
		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_AJAR_ERROR);
		
		}
			
		// Assume package arrived
		package = true;
		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_OPEN);

		break;

		case STATE_LOCKED: // %%%%%%%%% 444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444
		// Unlock in case of PANIC tripped
		PanicSensorCheck();
		// Unlock in case of KEYPAD input tripped
		KeypadCheck();

		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_LOCKED); // Red solid
		

		break;

		case STATE_QUALIFIER: // %%%%%%% 55555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555
		// Just opened and debounced with package
		if (TimeReached(&tDebounce, DEBOUNCE_INTERVAL) && LID_SWITCH_ONOFF()  && package == true)
		{
			Serial.println("_____________________________________________________________");
			Serial.println("Package Retrieved");
			Serial.println("Package state has been reset to false");
			Serial.println("When lid is closed, we will be ready for the next delivery!");
			Serial.println("_____________________________________________________________");
			package = false;
		}
		// Closed and debounced with no package
		else if (!LID_SWITCH_ONOFF() && TimeReached(&tDebounce, DEBOUNCE_INTERVAL) && package == false)
		{
			changeState(STATE_CLOSED);
		}

		// Lid open for too long
		if (TimeReached(&tAjar, LID_OPEN_INTERVAL))
		{
			Serial.println("Lid was left AJAR");
			// Set NEOPixel status light
			changeStatus(FEEDBACK_STATUS_AJAR_ERROR); // Blue blinking
		
		}


		// Set NEOPixel status light
		changeStatus(FEEDBACK_STATUS_READY_RETRIEVE); // Yellow blinking
		
		break;

		case STATE_READY: //%%%%%% 666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666

		if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
			Serial.println("VAULT HAS STARTED UP...");
			package = false;
			changeState(STATE_UNLOCKING);
		}
			

		break;

	}



}

void PanicSensorCheck(){

uint8_t pirState = 0;         // current state of the button
uint8_t lastPIRState = 0;     // previous state of the button

	// read the pushbutton input pin:
  	pirState = digitalRead(PANIC_PIR_SNSR);
  	// compare the buttonState to its previous state
  	if (pirState != lastPIRState) {
    // if the state has changed, increment the counter
    if (pirState == 1 && TimeReached(&tPircheck, DEBOUNCE_INTERVAL)) {
		changeState(STATE_UNLOCKING);
		Serial.println("PIR was triggered");
		}
    } 
  
  // save the current state as the last state, for next time through the loop
  lastPIRState = pirState;

}

void KeypadCheck(){

uint8_t keypadState = 0;         // current state of the button
uint8_t lastKEYPADState = 0;     // previous state of the button

	// read the pushbutton input pin:
  	keypadState = digitalRead(KEYPAD_TRIGGER);
  	// compare the buttonState to its previous state
  	if (keypadState != lastKEYPADState) {
    // if the state has changed, increment the counter
    if (keypadState == 1 && TimeReached(&tKeycheck, DEBOUNCE_INTERVAL)) {
		changeState(STATE_UNLOCKING);
		Serial.println("KEYPAD was triggered");
		}
    } 

  // save the current state as the last state, for next time through the loop
  lastKEYPADState = keypadState;

}

void CheckLidState(){
  
 if ((LID_SWITCH_ONOFF()!=lid_switch.state)
  &&(TimeReached(&tDebounce, DEBOUNCE_INTERVAL)) 
 ){
        
    lid_switch.state = LID_SWITCH_ONOFF(); //tDetectTime = millis();
    if(lid_switch.state){ Serial.print("Active high lid_switch");
      lid_switch.isactive = true;
      lid_switch.tDetectTimeforDebounce = millis();
   

    }else{ Serial.print("Active low lid_switch");
        lid_switch.isactive = false;

    }
    lid_switch.ischanged = true;
 }
}
/*****************************************************************************************************************************
* FSM Change State Function for ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**************************************************************************************************************************** */
/*oid changeState(int new_state){
	box_state = new_state;
	//Serial.println(new_state);
}*/
/***************************************************************************************************************************** */


/*void init_Colormap(){
	Serial.println("VOID Colormap function message");
	preset_color_map[COLOR_RED_INDEX]      	= HsbColor(Hue360toFloat(0),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_PURPLE_INDEX]	= HsbColor(Hue360toFloat(280),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_CYAN_INDEX]    	= HsbColor(Hue360toFloat(180),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_GREEN_INDEX]    	= HsbColor(Hue360toFloat(120),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_BLUE_INDEX]     	= HsbColor(Hue360toFloat(240),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_YELLOW_INDEX]   	= HsbColor(Hue360toFloat(30),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_WHITE_INDEX]     = HsbColor(Hue360toFloat(0),Sat100toFloat(0),Brt100toFloat(100));

}

void NeoStatus_Tasker(){

	switch(neo_mode){
        case ANIMATION_MODE_NOTIFICATIONS_ID:
        NeoStatus_SubTask();
		NEO_Feedback_Display();
        break;
        case ANIMATION_MODE_NONE: default: break; // resting position call be called EVERY loop without doing anything, optional, disable "NeoStatus_Tasker" with a flag
    }
} 

// This function will be called EVERY LOOP because it checks "tRateUpdate" for if each pixel needs to be updated, so dont add anything else in here, how this works is set elsewhere
void NeoStatus_SubTask(){

   // Updates NEO's at 2 min interval OR if force update requested
  if(TimeReached(&notif.tSaved.ForceUpdate,120000)||(notif.fForceStatusUpdate)){
    notif.fForceStatusUpdate = true;
    Serial.println("OPTIONAL FORCING, I USE IT TO MAKE SURE PIXELS NEVER GET STUCK ON SOMETHING NOT INTENDED");
  }
  
  //Animation Types
  for(int i=0;i<PIXEL_COUNT;i++){
    if(TimeReached(&notif.pixel[i].tSavedUpdate,notif.pixel[i].tRateUpdate)||(notif.fForceStatusUpdate)){ notif.fForceStatusUpdate = false;
      switch(notif.pixel[i].mode){
        default:
        case NOTIF_MODE_OFF_ID:
        case NOTIF_MODE_STATIC_OFF_ID:
        	stripbus->SetPixelColor(i,0);
        break;
        case NOTIF_MODE_STATIC_ON_ID:
        	stripbus->SetPixelColor(i,HsbColor(notif.pixel[i].color.H,notif.pixel[i].color.S,notif.pixel[i].color.B));    
        break;
        case NOTIF_MODE_BLINKING_OFF_ID:
        	stripbus->SetPixelColor(i,0);
        	notif.pixel[i].mode = NOTIF_MODE_BLINKING_ON_ID;
        	notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/2);
        break;
        case NOTIF_MODE_BLINKING_ON_ID:
        	stripbus->SetPixelColor(i,HsbColor(notif.pixel[i].color.H,notif.pixel[i].color.S,notif.pixel[i].color.B));    
        	notif.pixel[i].mode = NOTIF_MODE_BLINKING_OFF_ID;
        	notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/2);
        break;
        case NOTIF_MODE_ALTERNATE_COLOR_1:
        	stripbus->SetPixelColor(i,notif.pixel[i].alternate_colors[0]);
        	notif.pixel[i].mode = NOTIF_MODE_ALTERNATE_COLOR_2;
        	notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/2);
        break;
        case NOTIF_MODE_ALTERNATE_COLOR_2:
        	stripbus->SetPixelColor(i,notif.pixel[i].alternate_colors[1]);   
        	notif.pixel[i].mode = NOTIF_MODE_BLINKING_OFF_ID;
        	notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/2);
        break;
        case NOTIF_MODE_PULSING_OFF_ID:
          if(notif.pixel[i].pulse_progess<100){
            notif.pixel[i].pulse_progess++;
          }else{
            notif.pixel[i].mode = NOTIF_MODE_PULSING_ON_ID;
          }
          notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/200);
          //AddLog_mP2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_NEO "PULSING progress [%d]"),notif.pixel[i].pulse_progess); 
          notif.pixel[i].color.B = notif.pixel[i].pulse_progess/100.0f;
          stripbus->SetPixelColor(i,HsbColor(notif.pixel[i].color.H,notif.pixel[i].color.S,notif.pixel[i].color.B));    
        break;
        case NOTIF_MODE_PULSING_ON_ID:
          if(notif.pixel[i].pulse_progess>0){
            notif.pixel[i].pulse_progess--;
          }else{
            notif.pixel[i].mode = NOTIF_MODE_PULSING_OFF_ID;
          }
          notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/200);
          //AddLog_mP2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_NEO "PULSING progress [%d]"),notif.pixel[i].pulse_progess); 
          notif.pixel[i].color.B = notif.pixel[i].pulse_progess/100.0f;
          stripbus->SetPixelColor(i,HsbColor(notif.pixel[i].color.H,notif.pixel[i].color.S,notif.pixel[i].color.B));     
        break;
      }
      notif.fShowStatusUpdate = true;
	  //Serial.println("End of pixel.mode");
    } //end switch case
  } //end timer check

	

  //Auto turn off
  if(TimeReached(&notif.tSaved.AutoOff,1000)){// if 1 second past
    for(int i=0;i<PIXEL_COUNT;i++){ //check all
      if(notif.pixel[i].auto_time_off_secs==1){ //if =1 then turn off and clear to 0
        stripbus->SetPixelColor(i,0);
        notif.pixel[i].auto_time_off_secs = 0;
        notif.pixel[i].mode = NOTIF_MODE_OFF_ID;
     }else
      if(notif.pixel[i].auto_time_off_secs>1){ //if =1 then turn off and clear to 0
        notif.pixel[i].auto_time_off_secs--; //decrease
      }
    }// END for
  }



  // Update == if we change something in the switch above, then actually output onto the string, but only once...
  if(notif.fShowStatusUpdate){
	notif.fShowStatusUpdate=false; //.... hence reset
    stripbus->Show();
    notif.tSaved.ForceUpdate = millis(); // RESETS UPDATE TIMER
	//Serial.println("Strip update completed");  
  }
}
	

void NEO_Feedback_Display(){ //Sets color and pattern of NEO status indicator
	switch (status){
		default:
		case FEEDBACK_STATUS_OFF:
		case FEEDBACK_STATUS_READY:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Ready");
			}
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];

			notif.pixel[1].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_GREEN_INDEX];
	
		break;
		case FEEDBACK_STATUS_UNLOCKING:
		if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Unlocking");
			}
			
			notif.pixel[0].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_PULSING_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];
			//notif.pixel[0].auto_time_off_secs = 8;

			notif.pixel[1].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_PULSING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_GREEN_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
			
		break;
   		case FEEDBACK_STATUS_OPEN:
		   if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
		   		Serial.println("FS Open");
			}
			//notif.pixel[0].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[0].auto_time_off_secs = 8;

		   	//notif.pixel[1].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
		
		break;
		case FEEDBACK_STATUS_AJAR_ERROR:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS AJAR");
			}
			notif.pixel[0].period_ms = 500; // 0.25 second between "on"s, so quarter second toggling
			notif.pixel[0].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[0].auto_time_off_secs = 8;

			notif.pixel[1].period_ms = 750; 
			notif.pixel[1].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_BLUE_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
	
		break;
		case FEEDBACK_STATUS_CLOSED_COUNTING:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Counting");
			}
			//notif.pixel[0].period_ms = 750; // three quarter second between "on"s, so three quarter second toggling
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_WHITE_INDEX];
			//notif.pixel[0].auto_time_off_secs = 8;

			notif.pixel[1].period_ms = 750; // three quarter second between "on"s, so three quarter second toggling
			notif.pixel[1].mode = NOTIF_MODE_PULSING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_YELLOW_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
		
		break;
		case FEEDBACK_STATUS_LOCKING:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Locking");
			}
			//notif.pixel[0].period_ms = 1000; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_WHITE_INDEX];
			//notif.pixel[0].auto_time_off_secs = 8;

			notif.pixel[1].period_ms = 1000; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_PULSING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_PURPLE_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
		
		break;
		case FEEDBACK_STATUS_LOCKED:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Locked");
			}
			//notif.pixel[1].period_ms = 1000; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
			
			notif.pixel[1].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;

		break;
		case FEEDBACK_STATUS_READY_RETRIEVE:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Ready Retrieve");
			}
			notif.pixel[0].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];

			notif.pixel[1].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_GREEN_INDEX];

		break;
		case FEEDBACK_STATUS_BLINKING_PANIC:
			if (TimeReached(&tResponse, RESPONSE_INTERVAL2)){
				Serial.println("FS Panic");
			}
			notif.pixel[0].period_ms = 150; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_WHITE_INDEX];
			//notif.pixel[0].auto_time_off_secs = 2;

			notif.pixel[1].period_ms = 150; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_BLUE_INDEX];
	
		break;
	}
}

 EXAMPLE CODE BLOCK THAT COVERS ALL ITERATIONS OF MODE

notif.pixel[0].period_ms = 4000; // 1 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_ALTERNATE_COLOR_1;
			notif.pixel[0].alternate_colors[0] = preset_color_map[COLOR_YELLOW_INDEX];
			notif.pixel[0].alternate_colors[1] = preset_color_map[COLOR_PURPLE_INDEX];
			//notif.pixel[0].tRateUpdate ; = SET INTERNALLY, not directly
			notif.pixel[0].auto_time_off_secs = 10;
			Serial.println("Blinking pixel 0 green");

// Helper functions

float Hue360toFloat(uint16_t hue){
	return hue/360.0f;
}
float Sat100toFloat(uint8_t sat){
	return sat/100.0f;
}
float Brt100toFloat(uint8_t brt){
	return brt/100.0f;
}
uint16_t HueFloatto360(float hue){
	return round(hue*360.0f);
}
uint8_t SatFloatto100(float sat){
	return round(sat*100.0f);
}
uint8_t BrtFloatto100(float brt){
	return round(brt*100.0f);
}

	//uint32_t tSavedFeedbackDisplay = millis();


// Time elapsed function that updates the time when true
uint8_t TimeReached(TIMER_HANDLER* tSaved, uint32_t ElapsedTime){
  if(
    (abs(millis()-tSaved->millis)>=ElapsedTime)
    ||(tSaved->run == true)    
    ){ 
      tSaved->millis=millis();
      tSaved->run = false;
    return true;
  }
  return false;
}

uint8_t TimeReached(uint32_t* tSaved, uint32_t ElapsedTime){
  if(abs(millis()-*tSaved)>=ElapsedTime){ 
      *tSaved=millis();
    return true;
  }
  return false;
}

void changeStatus(int new_status){
	
	status = new_status;
	//Serial.println(new_status);
	
}
*/





// For sending without network during uploads
void AddSerialLog_P(uint8_t loglevel, PGM_P formatP, ...)
{

  // Speed/stability improvements, check log level and return early if it doesnt apply to any log events
  if(loglevel>seriallog_level){
    return;
  }
  
  // Filtering
  if(enable_serial_logging_filtering){ // if true, only permit exact log level and not all above
    if(loglevel == seriallog_level){
      //permit messages
    }else{
      return;
    }
  }

  va_list arg;
  va_start(arg, formatP);
  vsnprintf_P(log_data, sizeof(log_data), formatP, arg);
  va_end(arg);

  // LOG : SERIAL
  if (loglevel <= seriallog_level) {
    Serial.printf("%s %s\r\n", GetLogLevelNameShortbyID(loglevel),  log_data);
    //To stop asynchronous serial prints, flush it, but remove this under normal operation so code runs better (sends serial after the fact)
	// IMPORTANT!!! The code will pause here if flush is set, only for ms until the serial print has been sent
	// Normally, serial is passed to hardware internal the the chip, and serial is printed in the background. However, if a problem/bug with forced reseting exists,
	// you want to print all serial BEFORE tripping the reset, so only enable when fault tracing
	#ifdef ENABLE_SERIAL_DEBUG_FLUSH
    	Serial.flush();
	#endif
  }

}


const char* GetLogLevelNameShortbyID(uint8_t id){
    return (id == LOG_LEVEL_NONE ? PSTR("NON") :
        (id == LOG_LEVEL_ERROR ?   PSTR("ERR") :
        (id == LOG_LEVEL_WARN ?   PSTR("WRN") :
        (id == LOG_LEVEL_TEST ?   PSTR("TST") :
        (id == LOG_LEVEL_INFO ?    PSTR("INF") :
        (id == LOG_LEVEL_DEBUG ?   PSTR("DBG") :
        (id == LOG_LEVEL_DEBUG_MORE ? PSTR("DBM") :
        (id == LOG_LEVEL_DEBUG_LOWLEVEL ? PSTR("DBL") :
        (id == LOG_LEVEL_ALL ? PSTR("ALL") :
        PSTR("unk"))))))))));
}

