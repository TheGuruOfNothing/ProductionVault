#include <Arduino.h>

/* *******The Package Vault Project***********
* I'd like to give a giant thank you to @Sparkplug23 for all his gracious and ongoing assistance
* and collaboration in this project. The majority of the code is his and he deserves the credit 
* for all his patience invested as well, as I am not the easiest dog to teach new tricks to. 
*
* Additional credit is owed to @Chris Aitken for his efforts also. The original core code was 
* his. I have used that as the foundation to the project and though it is now only a small piece
* of the large puzzle, it is what took the vault from a passing whimsy to a functional device. LOTS of
* upgrades have been built onto that code.
*
* The Package Vault Project was started as a personal project to try and stop package thieves
* from taking my packages. After falling victim, I decided enough was enough. This device
* was born. It's popularity amongst the neighbors and local community has made it take on a life of it's own.
* It is my hope that you will take the code that has been created and figure out how to integrate it
* into something new and cool that has the same goal - to thwart package thieves. 

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
------------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <NeoPixelBus.h>

#define BUILD_NUMBER_CTR "v0_13"

// Levels of stability (from testing to functional)
#define STABILITY_LEVEL_NIGHTLY     "nightly"     // testing (new code -- bugs) <24 hour stability
//#define STABILITY_LEVEL_PRE_RELEASE "pre_release" // under consideration for release (bug checking) >24 stability
//#define STABILITY_LEVEL_RELEASE     "release"     // long term >7 days


unsigned long timer = 0;
unsigned long debounce_timer = 0;
unsigned long blink_timer = 0;
unsigned long total_reset_timer = 0;
bool reset = false;
bool package = false;


//FSM Enums - Defining Switch Case enumeration for each switch****************************************************
enum STATES_LID{STATE_UNLOCKING=0,STATE_LOCKING,STATE_OPENED,STATE_CLOSED,STATE_LOCKED,STATE_QUALIFIER};
int current_state = STATE_UNLOCKING; // Starting state for the vault at power up- current but due to change

enum FEEDBACK_STATUS{FEEDBACK_STATUS_READY=0, FEEDBACK_STATUS_UNLOCKING,FEEDBACK_STATUS_OPEN,FEEDBACK_STATUS_AJAR_ERROR,
FEEDBACK_STATUS_CLOSED_COUNTING,FEEDBACK_STATUS_LOCKED,FEEDBACK_STATUS_READY_RETRIEVE};
uint8_t status = FEEDBACK_STATUS_READY;

enum NOTIF_MODE{NOTIF_MODE_OFF_ID=0, NOTIF_MODE_STATIC_OFF_ID, NOTIF_MODE_STATIC_ON_ID, NOTIF_MODE_BLINKING_OFF_ID,
NOTIF_MODE_BLINKING_ON_ID, }; 

//Function Prototypes
void SetFeedbackStatus(uint8_t new_status );
void changeState(int new_state, bool reset = false);
bool GetTimer(unsigned long &timer, int interval);
bool TimeReached(uint32_t* tSaved, uint32_t ElapsedTime);
void Status_Update(void);
void NeoStatus_Tasker(void);
void init_NeoStatus(void);

	// Timer Intervals - ALL non-blocking timers
    #define BLINK_INTERVAL		500			// Fast blink interval, half second
    #define SLOW_BLINK_INTERVAL	1800		// Slow blink interval, 1.8 seconds
    #define LID_OPEN_INTERVAL	120000		// Lid ajar timer interval, sends ajar message if lid is left open
    #define LOCKDOWN_INTERVAL	15000		// Period of time before lockdown of vault after lid close, 15 seconds
    #define RELAY_INTERVAL		6000		// Lock/Unlock relay operation time for those functions, 6 seconds
    #define DEBOUNCE_INTERVAL	400			// Button Debounce
    #define TOTAL_RESET_INTERVAL 86400000	// Full board reset timer, fired evey 24 hours - debug only

    // I/O
    #define RELAY_LOCK_PIN		16	// Unlock Relay
    #define RELAY_UNLOCK_PIN	17	// Lock Relay
    #define INTERIOR_LIGHTS_PIN	4	// Interior lighting ring for camera and panic functions
	#define LID_SWITCH			18	// Mag switch on lid
    #define PANIC_PIR_SNSR		19	// Passive Infrared sensor for panic release
    #define KEYPAD_TRIGGER		14	// Latching pushbutton for  panic release, backlit
	#define LED_PIN        		3	// Control pin for the neopixel status indicators

	// NeoPixel Info
	#define PIXEL_PIN     19  // Digital IO pin 
   	#define PIXEL_COUNT    2   // Number of NeoPixels
	// Declare our NeoPixel strip object:
	NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PIXEL_COUNT, PIXEL_PIN);

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

	#define NEO_PIN_INIT() 				pinMode(PIXEL_PIN, OUTPUT)//Neopixel status indicators

	//RELAYS
    #define ON_LOGIC_LEVEL HIGH  //Opened when LOW

      #define RELAY_LOCK_INIT()      	pinMode(RELAY_LOCK_PIN,OUTPUT)
	  #define RELAY_LOCK_START()	 	digitalWrite(RELAY_LOCK_PIN,LOW)
      //#define RELAY_LOCK_ONOFF()     	!digitalRead(RELAY_LOCK_PIN) //opened when LOW
      #define RELAY_LOCK_ON()       	digitalWrite(RELAY_LOCK_PIN,ON_LOGIC_LEVEL) //opened when LOW
      #define RELAY_LOCK_OFF()      	digitalWrite(RELAY_LOCK_PIN,!ON_LOGIC_LEVEL) //opened when LOW

	  #define RELAY_UNLOCK_INIT()      	pinMode(RELAY_UNLOCK_PIN,OUTPUT)
	  #define RELAY_UNLOCK_START()	   	digitalWrite(RELAY_LOCK_PIN,LOW)
      //#define RELAY_UNLOCK_ONOFF()    !digitalRead(RELAY_UNLOCK_PIN) //opened when LOW
      #define RELAY_UNLOCK_ON()        	digitalWrite(RELAY_UNLOCK_PIN,ON_LOGIC_LEVEL) //opened when LOW
      #define RELAY_UNLOCK_OFF()       	digitalWrite(RELAY_UNLOCK_PIN,!ON_LOGIC_LEVEL) //opened when LOW

	  #define INTERIOR_LIGHTS_INIT()    pinMode(INTERIOR_LIGHTS_PIN,OUTPUT)
	  #define INTERIOR_LIGHTS_START()	digitalWrite(RELAY_LOCK_PIN,LOW)
      //#define INTERIOR_LIGHTS_ONOFF() !digitalRead(INTERIOR_LIGHTS_PIN) //opened when LOW
      #define INTERIOR_LIGHTS_ON()      digitalWrite(INTERIOR_LIGHTS_PIN,ON_LOGIC_LEVEL) //opened when LOW
      #define INTERIOR_LIGHTS_OFF()     digitalWrite(INTERIOR_LIGHTS_PIN,!ON_LOGIC_LEVEL) //opened when LOW
      
/*****************************************************************************************************************************
 * SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        SETUP        
 *****************************************************************************************************************************
 */

void setup()
{	Serial.begin(115200);

	RELAY_LOCK_INIT(); RELAY_LOCK_START(); //Start LOW
	RELAY_UNLOCK_INIT(); RELAY_UNLOCK_START(); //Start LOW
	INTERIOR_LIGHTS_INIT(); INTERIOR_LIGHTS_START(); //Start LOW
	LID_SWITCH_INIT();
	PANIC_PIR_SNSR_INIT();
	KEYPAD_TRIGGER_INIT();
	NEO_PIN_INIT();

	strip.Begin(); // Initialize NeoPixel strip object (REQUIRED)
  	strip.Show();  // Initialize all pixels to 'off'

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

	switch (current_state)
	{
		
		case STATE_UNLOCKING:
			// Unlocked with package
			if (package == true && GetTimer(timer, RELAY_INTERVAL))
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
				if (GetTimer(timer, RELAY_INTERVAL	)) 
				{
					Serial.println("Ready for delivery");

					// Stops actuator power
					RELAY_UNLOCK_OFF();

					changeState(STATE_CLOSED);
					break;
				}
			}
			
			// Starts actuator power for unlock
			RELAY_UNLOCK_ON();
			Serial.println("Now Unlocking");

			// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);
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
				break;
			}

			// Starts actuator power / keep locking
			RELAY_LOCK_ON();
			// Turns interior INTERIOR_LIGHTS on for camera, 10 seconds
			INTERIOR_LIGHTS_ON();
			Serial.println("Now Locking");

			// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);
			

			break;


		case STATE_CLOSED:
			Serial.println("Entered STATE_CLOSED");
			// Just opened and debounced
			if (LID_SWITCH_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL))
			{
				Serial.println("Lid Was Opened!");
				changeState(STATE_OPENED);
				break;
			}
			// Package arrived and lockout timer expired
			else if (package == true && GetTimer(timer, LOCKDOWN_INTERVAL))
			{
				INTERIOR_LIGHTS_ON();
				changeState(STATE_LOCKING);
				break;
			}

			// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);
  			
			break;


		case STATE_OPENED:
			// Just closed and debounced
			if (!LID_SWITCH_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL))
			{
				Serial.println("Lid Was Closed!");
				changeState(STATE_CLOSED);
				break;
			}
			// Lid open for too long
			else if (GetTimer(timer, LID_OPEN_INTERVAL))
			{
				Serial.println("I've been left AJAR");
				// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);
			}

			// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);

			// Assume package arrived
			package = true;
			
			break;


		case STATE_LOCKED:
			// Unlock in case of PANIC_SENSOR tripped
			if (!PANIC_PIR_SNSR_ACTIVE());
			{
				Serial.println("PANIC_SW!");
				changeState(STATE_UNLOCKING);
				break;
			}

			// Unlock when proper keypad code is given
			if (KEYPAD_TRIGGER_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL));
			{
				Serial.println("PANIC_SW!");
				changeState(STATE_UNLOCKING);
				break;
			}

			// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);

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
				break;
			}

			// Lid open for too long
			if (GetTimer(timer, LID_OPEN_INTERVAL))
			{
				Serial.println("Lid was left AJAR");
				// Set status light
				SetFeedbackStatus(FEEDBACK_STATUS_READY);
		
			}

			// Set status light
			SetFeedbackStatus(FEEDBACK_STATUS_READY);
			break;
	}

}



/*****************************************************************************************************************************
* FSM Change State Function(s)
*****************************************************************************************************************************/
void changeState(int new_state, bool reset){
	
	current_state = new_state;
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
	
	if (timer < 1)
	{
		timer = millis();
	}

	if (millis() - timer >= interval)
	{
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




/* ***************************************************************************************************************************
*
*
*                                                 QUARANTINE
*
*
*****************************************************************************************************************************/


    struct NOTIF{
      uint8_t fForceStatusUpdate = false;
      uint8_t fShowStatusUpdate  = false;
      struct TSAVED{
        uint32_t ForceUpdate = millis();
      }tSaved;
      struct PIXELN{
        uint8_t  mode = NOTIF_MODE_STATIC_ON_ID; // Type of light pattern
        uint16_t period_ms = 1000; // Time between fully on and off
        HsbColor colour; // colour... just because it's spelled funny :-)
        uint32_t tSavedUpdate; // millis last updated
        uint16_t tRateUpdate = 10; // time between updating, used for blink (mode)
      }pixel[PIXEL_COUNT];
    }notif;
    //int8_t parsesub_NotificationPanel();




void loop() {
 NeoStatus_Tasker();
  
}

void init_NeoStatus(){

  for(int i=0;i<PIXEL_COUNT;i++){
    notif.fForceStatusUpdate = true; //clear presets
    notif.pixel[i].mode = NOTIF_MODE_OFF_ID;
    notif.pixel[i].colour.H = (i*30)/360.0f;
    notif.pixel[i].colour.S = 1;
    notif.pixel[i].colour.B = 1;
  }

} //end "init_NeoStatus"

void NeoStatus_Tasker(){

  // Updates NEO's at 2 min interval OR if force update requested
  if(TimeReached(&notif.tSaved.ForceUpdate,120000)||(notif.fForceStatusUpdate)){
    notif.fForceStatusUpdate = true;
    Serial.println("Status Has been updated by Tasker");
  }
  //Animation Types
  for(int i=0;i<PIXEL_COUNT;i++){
    if(TimeReached(&notif.pixel[i].tSavedUpdate,notif.pixel[i].tRateUpdate)||(notif.fForceStatusUpdate)){ notif.fForceStatusUpdate = false;
      switch(notif.pixel[i].mode){
        default:
        case NOTIF_MODE_OFF_ID:
        case NOTIF_MODE_STATIC_OFF_ID:
          strip.SetPixelColor(i,0);
        break;
        case NOTIF_MODE_STATIC_ON_ID:
          strip.SetPixelColor(i,HsbColor(notif.pixel[i].colour.H,notif.pixel[i].colour.S,notif.pixel[i].colour.B));    
        break;
        case NOTIF_MODE_BLINKING_OFF_ID:
          strip.SetPixelColor(i,0);
          notif.pixel[i].mode = NOTIF_MODE_BLINKING_ON_ID;
          notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/2);
        break;
        case NOTIF_MODE_BLINKING_ON_ID:
          strip.SetPixelColor(i,HsbColor(notif.pixel[i].colour.H,notif.pixel[i].colour.S,notif.pixel[i].colour.B));    
          notif.pixel[i].mode = NOTIF_MODE_BLINKING_OFF_ID;
          notif.pixel[i].tRateUpdate = (notif.pixel[i].period_ms/2);
          break;
      }
      notif.fShowStatusUpdate = true;
    } //end switch case
  } //end timer check

  // Update
  if(notif.fShowStatusUpdate){notif.fShowStatusUpdate=false;
    strip.Show();
    notif.tSaved.ForceUpdate = millis(); // so we dont have flasher clashes
  }

}

// Time elapsed function that updates the time when true
bool TimeReached(uint32_t* tSaved, uint32_t ElapsedTime){
  if(abs(millis()-*tSaved)>=ElapsedTime){ *tSaved=millis();
    return true;
  }
  return false;
}