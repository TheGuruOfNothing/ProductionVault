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

#define BUILD_NUMBER_CTR "v0_3"

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
//Switch-case for vault states
enum STATES_LID{STATE_UNLOCKING=0,STATE_LOCKING,STATE_OPENED,STATE_CLOSED,STATE_LOCKED,STATE_QUALIFIER};
int current_state = STATE_UNLOCKING; // Starting state for the vault at power up- current but due to change

//Switch-case for NEOpixel Status indicator lights
enum FEEDBACK_STATUS{FEEDBACK_STATUS_OFF=0, FEEDBACK_STATUS_READY, FEEDBACK_STATUS_UNLOCKING,FEEDBACK_STATUS_OPEN,FEEDBACK_STATUS_AJAR_ERROR,
FEEDBACK_STATUS_CLOSED_COUNTING,FEEDBACK_STATUS_LOCKED,FEEDBACK_STATUS_READY_RETRIEVE, FEEDBACK_STATUS_BLINKING_PANIC};
uint8_t status = FEEDBACK_STATUS_OFF;

//Switch-case for NEOpixel status (BLINK/SOLID) mode
enum NOTIF_MODE{NOTIF_MODE_OFF_ID=0, NOTIF_MODE_STATIC_OFF_ID, NOTIF_MODE_STATIC_ON_ID, NOTIF_MODE_BLINKING_OFF_ID,
NOTIF_MODE_BLINKING_ON_ID }; 

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
	//Sets NeoPixel function
	#define USE_RGB_NEO_STATUS
	

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

	init_NeoStatus();

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
			//************************Put a BLINKING green light on here**********************************************************
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
			//*********************Put on a BLINKING red light here
			

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
			//**********************Put a BLINKING red light here****************************************
  			
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
			//**********************Put a BLINKING blue light here****************************************
			}

			// Set status light
			//***************************************Put a SOLID green light here*************************************

			// Assume package arrived
			package = true;
			
			break;


		case STATE_LOCKED:
			// Unlock in case of PANIC_SENSOR tripped
			if (!PANIC_PIR_SNSR_ACTIVE());
			{
				Serial.println("PANIC_SW!");
				changeState(STATE_UNLOCKING);
				//********************Make a BLINKING RED AND BLUE light flash for 10 minutes unless box is reset****************
				break;
			}

			// Unlock when proper keypad code is given
			if (KEYPAD_TRIGGER_ACTIVE() && GetTimer(debounce_timer, DEBOUNCE_INTERVAL));
			{
				Serial.println("Keyad received correct code, unlocking now...");
				changeState(STATE_UNLOCKING);
				break;
			}

			// Set status light
			//***********************Put a BLINKING red light here

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
				//**********************Put a BLINKING blue light here****************************************
		
			}

			// Set status light
			//*************************Put a BLINKING green light here****************************************
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



/*****************************************************************************************************************************
*																										  					 *
*                                                                   NeoPixel Status LED's									 *
*																															 *
*****************************************************************************************************************************/
#ifdef USE_RGB_NEO_STATUS

//defines like this are not valid "define = x" its "define X"
// This might have worked, I am not sure, since its functions calling functions
	// #define color_map_red 		= HsbColor(Hue360toFloat(0),Sat100toFloat(100),Brt100toFloat(100));
	// #define color_map_green 	= HsbColor(Hue360toFloat(120),Sat100toFloat(100),Brt100toFloat(100));
	// #define color_map_blue 		= HsbColor(Hue360toFloat(240),Sat100toFloat(100),Brt100toFloat(100));
	// #define color_map_purple 	= HsbColor(Hue360toFloat(300),Sat100toFloat(100),Brt100toFloat(100));
	// #define color_map_yellow 	= HsbColor(Hue360toFloat(50),Sat100toFloat(100),Brt100toFloat(100));	


	// You also forgot to add my functions "Hue360.." etc



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
   
#endif



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
    notif.tSaved.ForceUpdate = millis(); // RESETS UPDATE TIMER
  }

// WRONG, see below
	// /*Sets how status NEOpixels display depending on intended color and pattern
	// *	COLORS AS DEFINED ABOVE... HERE FOR SAME PAGE REFERENCE ONLY...
	// *	#define color_map_red 		= HsbColor(Hue360toFloat(0),Sat100toFloat(100),Brt100toFloat(100));
	// *	#define color_map_green 	= HsbColor(Hue360toFloat(120),Sat100toFloat(100),Brt100toFloat(100));
	// *	#define color_map_blue 		= HsbColor(Hue360toFloat(240),Sat100toFloat(100),Brt100toFloat(100));
	// *	#define color_map_purple 	= HsbColor(Hue360toFloat(300),Sat100toFloat(100),Brt100toFloat(100));
	// *	#define color_map_yellow 	= HsbColor(Hue360toFloat(50),Sat100toFloat(100),Brt100toFloat(100));
	// */




// pixel 0 as red
notif.pixel[0].colour.H = 0;
notif.pixel[0].colour.S = 1;
notif.pixel[0].colour.B = 1;

HsbColor hsb_red = HsbColor(0,1,1);
notif.pixel[0].colour = hsb_red; // also red, as "colour" I have is a hsbcolour type, see the struct definition I made

//^^ This stores ONE colour, so lets store multiple colours in an array of type hsbcolor
HsbColor preset_colour_map[3];

//lets fill this array with red, green and blue
preset_colour_map[0] = HsbColor(0,1,1);
preset_colour_map[1] = HsbColor(120/360.0f,1,1);
preset_colour_map[2] = HsbColor(240/360.0f,1,1);

//so back to setting the pixels, you can now do this
notif.pixel[0].colour = preset_colour_map[0]; // set pixel 0, to the colour stored in the array space 0 which is red

// Now you have to remember what each index colour is, thats not nice
#define COLOUR_RED_INDEX 	0
#define COLOUR_GREEN_INDEX 	1
#define COLOUR_BLUE_INDEX 	2

//so back to setting the pixels, you can now do this
notif.pixel[0].colour = preset_colour_map[COLOUR_RED_INDEX]; // SAME AS ABOVE, but much nicer! you dont need to remember 0 now

//defines are great, but what if you have 20 or more, and they are just increasing in numbers... ENUMS!

enum COLOUR_MAP_INDEXS{COLOUR_RED_INDEX2,COLOUR_GREEN_INDEX2,COLOUR_BLUE_INDEX2};
//equates to
//enum COLOUR_MAP_INDEXS{0,				1,					2				};

//SIDE NOTE, the default starting number is 0, but you can set it and any of them directly, it just counts up after
//enum COLOUR_MAP_INDEXS{COLOUR_RED_INDEX2=1000,COLOUR_GREEN_INDEX2,COLOUR_BLUE_INDEX2}; ==> 1000,1001,1002 etc
 
//now, HSbcolour using floats, where hsbcolour(h,s,b), 
	//h = hue/colour, a float between 0 and 1... not human useful...we want the full circle (0-360)
	//s = saturation, a float between 0 and 1... not human useful... "whiteness" 0-100%
	//b = brightnes, a float between 0 and 1... not human useful... 0-100%
// Hsbcolour(h/360.0f,s/100.0f,b/100.0f) will convert these from "human" numbers to floats... its ugly... 
/// Check my project for those functions "Hue360toFloat(0),Sat100toFloat(100),Brt100toFloat(100)"

// putting to together a little more, you will see some "smart/lazy" code of mine

	// enum PRESET_COLOUR_MAP_INDEXES{
	// 	// Red
	// 	COLOUR_MAP_RED_ID=0,                                      // starts at 0 (which is what we want the array index to start at)
	// 	COLOUR_MAP_ORANGE_ID,										//goes on by the amount of colours you need
	// 	COLOUR_MAP_GREEN_ID,
	// 	// 
	// 	COLOUR_MAP_NONE_ID											//think of this as "count++", here you would have indexs 0,1,2,3 where this is 3, at the end... coincidently, this counted that you have THREE colours above it, if enum starts at 0, then you will always have the last one as a "count"
	// };
	// #define PRESET_COLOUR_MAP_INDEXES_MAX COLOUR_MAP_NONE_ID   	//LOOK, order is imporant, the code above was created, and so count is known, now I can make an array big enough to fit them all in
	// HsbColor preset_colour_map[PRESET_COLOUR_MAP_INDEXES_MAX];

	// preset_colour_map[COLOUR_MAP_RED_ID]      = HsbColor(Hue360toFloat(0),Sat100toFloat(100),Brt100toFloat(100));
	// preset_colour_map[COLOUR_MAP_GREEN_ID]   = HsbColor(Hue360toFloat(120),Sat100toFloat(100),Brt100toFloat(100));
	// preset_colour_map[COLOUR_MAP_BLUE_ID]   = HsbColor(Hue360toFloat(240),Sat100toFloat(100),Brt100toFloat(100));
	
	/// Where is "COLOUR_MAP_NONE_ID" alsu useful??
	/*
	for(int colour_id;colour_id<COLOUR_MAP_NONE_ID;colour_id++){
		//this will loop for 0,1,2 BUT NOT 3
	}


	*/


	//switch (FEEDBACK_STATUS){ //this is like saying "#define "FEEDBACK"", its never going to change



//YOU SHOULDN'T BE CALLING THIS PART OF THE CODE ALL THE TIME, ONLY SET WHEN A COLOUR CHANGES
	switch (status){



		default:
		case FEEDBACK_STATUS_OFF:
		case FEEDBACK_STATUS_READY:
			//status = NOTIF_MODE_STATIC_ON_ID(color_map_green); // enum is just a define, not a function, I assume this was not what you thought

			//Now if you implemented the above
			notif.pixel[0].colour = preset_colour_map[COLOUR_GREEN_INDEX2];
			notif.pixel[1].colour = preset_colour_map[COLOUR_GREEN_INDEX2];

			//to make this more clear what its doing, its set the entire hsbcolour, which if you follow into the libs is literally
			
			notif.pixel[1].colour.H = preset_colour_map[COLOUR_GREEN_INDEX2].H;
			notif.pixel[1].colour.S = preset_colour_map[COLOUR_GREEN_INDEX2].S;
			notif.pixel[1].colour.B = preset_colour_map[COLOUR_GREEN_INDEX2].B;

			//sometimes, you will see me set the whole colour
			notif.pixel[1].colour = preset_colour_map[COLOUR_GREEN_INDEX2];
			uint8_t new_brightness_as_percentage = 50;
			notif.pixel[1].colour.B = Brt100toFloat(new_brightness_as_percentage);
			// but instead override the brightness value to my own... 
			// toggle brightness 1 to 0, or 100 to 0, blinks,

			//ramping it up and down, pulses... colour stays the same :)







		break;
		// case FEEDBACK_STATUS_UNLOCKING:
		// 	status = NOTIF_MODE_BLINKING_ON_ID(color_map_green);
		// break;
   		// case FEEDBACK_STATUS_OPEN:
		// 	status = NOTIF_MODE_BLINKING_ON_ID(color_map_red);    ///////////////////// BROKEN   BROKEN    BROKEN    BROKEN    BROKEN    BROKEN
		// break;
		// case FEEDBACK_STATUS_AJAR_ERROR:
		// 	status = NOTIF_MODE_BLINKING_ON_ID(color_map_blue);
		// break;
		// case FEEDBACK_STATUS_CLOSED_COUNTING:
		// 	status = NOTIF_MODE_BLINKING_ON_ID(color_map_purple);
		// break;
		// case FEEDBACK_STATUS_LOCKED:
		// 	status = NOTIF_MODE_STATIC_ON_ID(color_map_red);
		// break;
		// case FEEDBACK_STATUS_READY_RETRIEVE:
		// 	status = NOTIF_MODE_BLINKING_ON_ID(color_map_yellow);
		// break;
		// case FEEDBACK_STATUS_BLINKING_PANIC;
		// 	status = NOTIF_MODE_BLINKING_ON_ID(red and blue goes here... somehow);  //FIX THIS!!!
		// break;
	}

/*****************************************************************************************************************************
*********************************************************************END NEOPIXEL SECTION***************************************
*****************************************************************************************************************************/

}
