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

#define BUILD_NUMBER_CTR "v0_4"

// Levels of stability (from testing to functional)
#define STABILITY_LEVEL_NIGHTLY     "nightly"     // testing (new code -- bugs) <24 hour stability
//#define STABILITY_LEVEL_PRE_RELEASE "pre_release" // under consideration for release (bug checking) >24 stability
//#define STABILITY_LEVEL_RELEASE     "release"     // long term >7 days


unsigned long timer = 0;
unsigned long debounce_timer = 0;
unsigned long total_reset_timer = 0;
bool reset = false;
bool package = false;
int blinkFlag = 0;


//Switch-case for vault states
enum STATES_LID{STATE_UNLOCKING=0,STATE_LOCKING,STATE_OPENED,STATE_CLOSED,STATE_LOCKED,STATE_QUALIFIER};
int box_state = STATE_UNLOCKING; // Starting state for the vault at power up- current but due to change

//Switch-case for NEOpixel Status indicator lights
enum FEEDBACK_STATUS{FEEDBACK_STATUS_OFF=0, FEEDBACK_STATUS_READY, FEEDBACK_STATUS_UNLOCKING,FEEDBACK_STATUS_OPEN,FEEDBACK_STATUS_AJAR_ERROR,
FEEDBACK_STATUS_CLOSED_COUNTING,FEEDBACK_STATUS_LOCKING,FEEDBACK_STATUS_LOCKED,FEEDBACK_STATUS_READY_RETRIEVE, FEEDBACK_STATUS_BLINKING_PANIC,STATUS_NONE_ID};
uint8_t status = FEEDBACK_STATUS_OFF;

//Defines for NEOpixel status (BLINK/SOLID) mode
enum NOTIF_MODE{NOTIF_MODE_OFF_ID=0, NOTIF_MODE_STATIC_OFF_ID, NOTIF_MODE_STATIC_ON_ID, NOTIF_MODE_BLINKING_OFF_ID,
NOTIF_MODE_BLINKING_ON_ID,NOTIF_MODE_TOGGLE_COLORS_ON,NOTIF_MODE_TOGGLE_COLORS_OFF,MODE_NONE_ID}; 

//Defines for Mapping colors by name
enum COLOR_MAP_INDEXES{COLOR_RED_INDEX=0,COLOR_PURPLE_INDEX,COLOR_GREEN_INDEX,COLOR_BLUE_INDEX,COLOR_YELLOW_INDEX,COLOR_MAP_NONE_ID};
#define PRESET_COLOR_MAP_INDEXES_MAX COLOR_MAP_NONE_ID   
HsbColor preset_color_map[PRESET_COLOR_MAP_INDEXES_MAX];

//Function Prototypes
void SetFeedbackStatus(uint8_t new_status );
float    Hue360toFloat(uint16_t hue);
float    Sat100toFloat(uint8_t sat);
float    Brt100toFloat(uint8_t brt);
uint16_t HueFloatto360(float hue);
uint8_t  SatFloatto100(float sat);
uint8_t  BrtFloatto100(float brt);
void changeState(int new_state, bool reset = false);
bool GetTimer(unsigned long &timer, int interval);
bool TimeReached(uint32_t* tSaved, uint32_t ElapsedTime);
void Status_Update(void);
void NeoStatus_Tasker(void);
void init_Colormap(void);
void NEO_Feedback_Display();
void ShowRainbow(void);


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
#define PANIC_PIR_SNSR		32	// Passive Infrared sensor for panic release **************************change this pin, is not on this pin***************************
#define KEYPAD_TRIGGER		14	// Latching pushbutton for  panic release, backlit
//#define LED_PIN        		3	// Control pin for the neopixel status indicators

// NeoPixel Info
#define PIXEL_PIN     19  // Digital IO pin 
#define PIXEL_COUNT    2   // Number of NeoPixels

// Declare our NeoPixel strip object

NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> *stripbus = nullptr;



	

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
#define INTERIOR_LIGHTS_START()	  digitalWrite(RELAY_LOCK_PIN,LOW)
//#define INTERIOR_LIGHTS_ONOFF() !digitalRead(INTERIOR_LIGHTS_PIN) //opened when LOW
#define INTERIOR_LIGHTS_ON()      digitalWrite(INTERIOR_LIGHTS_PIN,ON_LOGIC_LEVEL) //opened when LOW
#define INTERIOR_LIGHTS_OFF()     digitalWrite(INTERIOR_LIGHTS_PIN,!ON_LOGIC_LEVEL) //opened when LOW
      
// /*****************************************************************************************************************************
// * FSM Change State Function
// *****************************************************************************************************************************/
// void changeState(int new_state, bool reset);

// /*****************************************************************************************************************************
// *																										  					 *
// *                                                                   TIMERS USED IN CODE										 *
// *																															 *
// *****************************************************************************************************************************/

// bool GetTimer(unsigned long &timer, int interval);
// bool TimeReached(uint32_t* tSaved, uint32_t ElapsedTime);

// /*****************************************************************************************************************************
// *********************************************************************END TIMERS SECTION***************************************
// *****************************************************************************************************************************/



// /*****************************************************************************************************************************
// *																										  					 *
// *                                                                   NeoPixel Status LED's									 *
// *																															 *
// *****************************************************************************************************************************/

	//unsigned long slow_blink_timer = 0;
	unsigned long fast_blink_timer = 0;
	unsigned long panic_blink_timer = 0;
	//#define SLOW_BLINK_INTERVAL		1000	// Slow blink interval, ONE second
	#define FAST_BLINK_INTERVAL		500		// Fast blink interval, HALF second
	#define PANIC_BLINK_INTERVAL	250		// Fast blink interval, QUARTER second


	uint32_t tSavedFeedbackDisplay = millis();

	void NeoStatus_SubTask();
	
// void init_Colormap();

	struct NOTIF{
    	uint8_t fForceStatusUpdate = false;
    	uint8_t fShowStatusUpdate  = false;
    	struct TSAVED{
    		uint32_t ForceUpdate = millis();
			uint32_t AutoOff = millis();
    	}tSaved;
    	struct PIXELN{
			uint8_t  mode = NOTIF_MODE_STATIC_ON_ID; // Type of light pattern
			uint16_t period_ms = 1000; // Time between fully on and off
			HsbColor color; 
			uint32_t tSavedUpdate; // millis last updated
			uint16_t tRateUpdate = 10; // time between updating, used for blink (mode)
			uint16_t auto_time_off_secs = 0; // reset pixel to off
    	}pixel[PIXEL_COUNT];
	}notif;




    enum ANIMATION_MODE{
      ANIMATION_MODE_PRESETS_ID,
      ANIMATION_MODE_AMBILIGHT_ID,
      ANIMATION_MODE_SCENE_ID,
        ANIMATION_MODE_NOTIFICATIONS_ID,
      ANIMATION_MODE_FLASHER_ID,
      ANIMATION_MODE_NONE
    }; 

	uint8_t neo_mode = ANIMATION_MODE_NONE;



// void NeoStatus_Tasker();


// void NEO_Feedback_Display();


// float Hue360toFloat(uint16_t hue);
// float Sat100toFloat(uint8_t sat);
// float Brt100toFloat(uint8_t brt);
// uint16_t HueFloatto360(float hue);
// uint8_t SatFloatto100(float sat);
// uint8_t BrtFloatto100(float brt);

// /*****************************************************************************************************************************
// *********************************************************************END NEOPIXEL SECTION***************************************
// *****************************************************************************************************************************/

