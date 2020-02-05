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

#include <Arduino.h>
#include <NeoPixelBus.h>

#include "PVP_Controller.h"


/*****************************************************************************************************************************
 * SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        
 *****************************************************************************************************************************
 */

void setup()
{	
	Serial.begin(115200);

	RELAY_LOCK_INIT(); RELAY_LOCK_START(); //Start LOW
	RELAY_UNLOCK_INIT(); RELAY_UNLOCK_START(); //Start LOW
	INTERIOR_LIGHTS_INIT(); INTERIOR_LIGHTS_START(); //Start LOW
	LID_SWITCH_INIT();
	PANIC_PIR_SNSR_INIT();
	KEYPAD_TRIGGER_INIT();
	NEO_PIN_INIT();

	Serial.println("Init colormap");
	init_Colormap();
	

    stripbus = new NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod>(PIXEL_COUNT,PIXEL_PIN);
	stripbus->Begin();
	stripbus->ClearTo(0);
  	stripbus->Show();  // Initialize all pixels to 'off'
	  Serial.println("Strip cleared...");

	Serial.println("Init show rainbow");  
	ShowRainbow();

}

/******************************************************************************************************************************
 * LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      
 ******************************************************************************************************************************
 ******************************************************************************************************************************
 */

void loop()
{
	/*// Forces a total restart of the board, timer interval every 24 hours				###Full Reset - commented out for now###
	// Used to make sure the system is cleared for debugging at this point
	if (GetTimer(total_reset_timer, TOTAL_RESET_INTERVAL))
	{
		ESP.restart();
	}
	*/

	NeoStatus_Tasker(); // called always without delay


	// Lets use this to trigger every 10 seconds. 
	// We will work on settings a notification pixel to blink for 6 seconds then turn itself off, 
	// repeating 4 seconds later when this fires again.
	if(TimeReached(&tSavedFeedbackDisplay,10000)){
		status = FEEDBACK_STATUS_UNLOCKING; // forcing mode
		NEO_Feedback_Display();		
		Serial.println("NeoStatus_Tasker timer timed out and reset...");
	}



// Get leds to work with method above to trigger code directly before getting the seciotn below working. I will look at this another time
/*
	switch (box_state){
		case STATE_UNLOCKING:
		// Unlocked with package
		if (package == true && GetTimer(timer, RELAY_INTERVAL))
		{
			Serial.println("Ready for retrieval");
		// Stops actuator power
			RELAY_UNLOCK_OFF();
			changeState(STATE_QUALIFIER);
		}
		// Unlocked and no package
		else 
		{
			if (GetTimer(timer, RELAY_INTERVAL	)) 
			{
				Serial.println("Ready for delivery");
		// Stops actuator power
				RELAY_UNLOCK_OFF();
				changeState(STATE_CLOSED);
			}
		}
		// Starts actuator power for unlock
		RELAY_UNLOCK_ON();
		Serial.println("Now Unlocking");

		// Set NEOPixel status light
		status = FEEDBACK_STATUS_UNLOCKING; //Green blinking
			break;

		case STATE_LOCKING:
		// Completely locked
		if (GetTimer(timer, RELAY_INTERVAL))
		{
			// Stops actuator power
			RELAY_LOCK_OFF();
			// Turns off interior INTERIOR_LIGHTS
			INTERIOR_LIGHTS_OFF();
			changeState(STATE_LOCKED);
		}

		// Starts actuator power / keep locking
		RELAY_LOCK_ON();
		// Turns interior INTERIOR_LIGHTS on for camera, 10 seconds
		INTERIOR_LIGHTS_ON();
		Serial.println("Now Locking");

		// Set NEOPixel status light
		status = FEEDBACK_STATUS_LOCKING; // Purple and Red alternating
			break;

		case STATE_CLOSED:
		Serial.println("Entered STATE_CLOSED");
		// Just opened and debounced
		if (LID_SWITCH_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL))
		{
			Serial.println("Lid Was Opened!");
			changeState(STATE_OPENED);
		}
		// Package arrived and lockout timer expired
		else if (package == true && GetTimer(timer, LOCKDOWN_INTERVAL))
		{
			INTERIOR_LIGHTS_ON();
			changeState(STATE_LOCKING);
		}

		// Set NEOPixel status light
		status = FEEDBACK_STATUS_CLOSED_COUNTING; // Purple blinking
  			break;

		case STATE_OPENED:
		// Just closed and debounced
		if (!LID_SWITCH_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL))
		{
			Serial.println("Lid Was Closed!");
			changeState(STATE_CLOSED);
		}
		// Lid open for too long
		else if (GetTimer(timer, LID_OPEN_INTERVAL))
		{
			Serial.println("I've been left AJAR");
		// Set NEOPixel status light
		status = FEEDBACK_STATUS_AJAR_ERROR; // Blue binking
		}

		// Set NEOPixel status light
		status = FEEDBACK_STATUS_OPEN; // Red blinking

		// Assume package arrived
		package = true;
			break;

		case STATE_LOCKED:
		// Unlock in case of PANIC_SENSOR tripped
		if (!PANIC_PIR_SNSR_ACTIVE());
		{
			Serial.println("PANIC_SW!");
			changeState(STATE_UNLOCKING, true);
			// Set NEOPixel status light
			status = FEEDBACK_STATUS_BLINKING_PANIC; // Purple and Red alternating
		}

		// Unlock when proper keypad code is given
		if (KEYPAD_TRIGGER_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL));
		{
			Serial.println("Keyad received correct code, unlocking now...");
			changeState(STATE_UNLOCKING, true);
		}

		// Set NEOPixel status light
		status = FEEDBACK_STATUS_LOCKED; // Red solid

		Serial.println("Vault is locked!");
			break;

		case STATE_QUALIFIER:
		Serial.println("Qualifier!");
		// Just opened and debounced with package
		if (LID_SWITCH_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL) && package == true)
		{
			Serial.println("_____________________________________________________________");
			Serial.println("Package Retrieved");
			Serial.println("Package state has been reset to false");
			Serial.println("When lid is closed, we will be ready for the next delivery!");
			Serial.println("_____________________________________________________________");
			package = false;
		}
		// Closed and debounced with no package
		else if (!LID_SWITCH_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL) && package == false)
		{
			changeState(STATE_CLOSED);
		}

		// Lid open for too long
		if (GetTimer(timer, LID_OPEN_INTERVAL))
		{
			Serial.println("Lid was left AJAR");
			// Set NEOPixel status light
			status = FEEDBACK_STATUS_AJAR_ERROR; // Blue blinking
		
		}

		// Set NEOPixel status light
		status = FEEDBACK_STATUS_READY_RETRIEVE; // Yellow blinking
			break;

	}

	*/

}

/*****************************************************************************************************************************
* FSM Change State Function
*****************************************************************************************************************************/
void changeState(int new_state, bool reset){
	
	box_state = new_state;
	timer = 0;
	debounce_timer = 0;

	if (reset)
	{
		package = false;
	}
}

/*****************************************************************************************************************************
*																										  					 *
*                                                                   TIMERS USED IN CODE										 *
*																															 *
*****************************************************************************************************************************/

bool GetTimer(unsigned long &timer, int interval){
	
	if (timer < 1){
		timer = millis();
	}

	if (millis() - timer >= interval){
		timer = 0;
		return true;
	}

	return false;
}


bool TimeReached(uint32_t* tSaved, uint32_t ElapsedTime){
  if(abs(millis()-*tSaved)>=ElapsedTime){ *tSaved=millis();
    return true;
  }
  return false;
}

/*****************************************************************************************************************************
*********************************************************************END TIMERS SECTION***************************************
*****************************************************************************************************************************/



/*****************************************************************************************************************************
*																										  					 *
*                                                                   NeoPixel Status LED's									 *
*																															 *
*****************************************************************************************************************************/


void init_Colormap(){
	Serial.println("Colormap Initialized");
	preset_color_map[COLOR_RED_INDEX]      	= HsbColor(Hue360toFloat(0),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_PURPLE_INDEX]	= HsbColor(Hue360toFloat(50),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_GREEN_INDEX]    	= HsbColor(Hue360toFloat(120),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_BLUE_INDEX]     	= HsbColor(Hue360toFloat(240),Sat100toFloat(100),Brt100toFloat(100));
	preset_color_map[COLOR_YELLOW_INDEX]   	= HsbColor(Hue360toFloat(300),Sat100toFloat(100),Brt100toFloat(100));
//	preset_color_map[COLOR_MAP_NONE_ID]     = HsbColor(Hue360toFloat(0),Sat100toFloat(0),Brt100toFloat(25)); // NONE does not exist, since none is 1 longer than the array since the array index starts at 0

}

void ShowRainbow(){
	Serial.println("In void showrainbow");

//	Spread the colours across all the pixels
// 10 pixels, would mean the hue "wheel" would be divided in 10
// you had this in tasker, which is wrong, it would overwrite it EACH call.. This is only for testing
	
	for(int i=0;i<PIXEL_COUNT;i++){
		//notif.fForceStatusUpdate = true; 
		//notif.pixel[i].mode = NOTIF_MODE_OFF_ID;
		notif.pixel[i].color.H = (i*30)/360.0f;
		notif.pixel[i].color.S = 1;
		notif.pixel[i].color.B = 1;
	}

}

void NeoStatus_Tasker(){

	switch(neo_mode){
        case ANIMATION_MODE_NOTIFICATIONS_ID:
          NeoStatus_SubTask();
		  Serial.println("In neo_mode case 0");
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
      }
      notif.fShowStatusUpdate = true;
	  Serial.println("End of pixel.mode");
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
	Serial.println("Strip update completed");  
  }
}
	

void NEO_Feedback_Display(){ //Sets color and pattern of NEO status indicator

	
	switch (status){
		default:
		case FEEDBACK_STATUS_OFF:
		case FEEDBACK_STATUS_READY:
			// notif.pixel[i].color = preset_color_map[COLOR_GREEN_INDEX];
			// notif.pixel[i].mode = NOTIF_MODE_STATIC_ON_ID;
		break;
		case FEEDBACK_STATUS_UNLOCKING:
		Serial.println("In feedback status unlocking");

			notif.pixel[0].period_ms = 1000; // 1 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];
			//notif.pixel[0].tRateUpdate ; = SET INTERNALLY, not directly
			notif.pixel[0].auto_time_off_secs = 6;
			Serial.println("Blinking pixel 0 green");
			
			notif.pixel[1].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[0].tRateUpdate ; = SET INTERNALLY, not directly
			notif.pixel[1].auto_time_off_secs = 8;
			Serial.println("blinking pixel 1 red");
			


			// notif.pixel[i].color = preset_color_map[COLOR_GREEN_INDEX];
			// notif.pixel[i].mode = NOTIF_MODE_BLINKING_ON_ID;
		break;
   		case FEEDBACK_STATUS_OPEN:
			// notif.pixel[i].color = preset_color_map[COLOR_RED_INDEX];
			// notif.pixel[i].mode = NOTIF_MODE_BLINKING_ON_ID;
		break;
		case FEEDBACK_STATUS_AJAR_ERROR:
			// notif.pixel[i].color = preset_color_map[COLOR_BLUE_INDEX];
			// notif.pixel[i].mode = NOTIF_MODE_BLINKING_ON_ID;
		break;
		case FEEDBACK_STATUS_CLOSED_COUNTING:
			// if (GetTimer(fast_blink_timer, FAST_BLINK_INTERVAL)){
			// 	notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			// 	notif.pixel[1].color = preset_color_map[COLOR_BLUE_INDEX];
			// 	stripbus->Show();
			// }else{
			// 	notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			// 	notif.pixel[0].color = preset_color_map[COLOR_BLUE_INDEX];
			// 	stripbus->Show();
			// }
			
		break;
		case FEEDBACK_STATUS_LOCKING:
			// if (GetTimer(fast_blink_timer, FAST_BLINK_INTERVAL)){
			// 	notif.pixel[0].color = preset_color_map[COLOR_PURPLE_INDEX];
			// 	notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			// 	stripbus->Show();
			// }else{
			// 	notif.pixel[1].color = preset_color_map[COLOR_PURPLE_INDEX];
			// 	notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			// 	stripbus->Show();
			// }
		break;
		case FEEDBACK_STATUS_LOCKED:
			// notif.pixel[i].color = preset_color_map[COLOR_RED_INDEX];
			// notif.pixel[i].mode = NOTIF_MODE_STATIC_ON_ID;
		break;
		case FEEDBACK_STATUS_READY_RETRIEVE:
			// notif.pixel[i].color = preset_color_map[COLOR_YELLOW_INDEX];
			// notif.pixel[i].mode = NOTIF_MODE_BLINKING_ON_ID;
		break;
		case FEEDBACK_STATUS_BLINKING_PANIC:
			// if (GetTimer(panic_blink_timer, PANIC_BLINK_INTERVAL)){
			// 	notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			// 	notif.pixel[1].color = preset_color_map[COLOR_BLUE_INDEX];
			// 	stripbus->Show();
			// }else{
			// 	notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			// 	notif.pixel[0].color = preset_color_map[COLOR_BLUE_INDEX];
			// 	stripbus->Show();
			// }
		break;
		Serial.println("End of status case");
	}
}


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

/*****************************************************************************************************************************
*********************************************************************END NEOPIXEL SECTION***************************************
*****************************************************************************************************************************/

