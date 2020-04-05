
#include <Arduino.h>
#include <NeoPixelBus.h>

// I/O
#define RELAY_LOCK_PIN		26	// Unlock Relay
#define RELAY_UNLOCK_PIN	25	// Lock Relay
#define INTERIOR_LIGHTS_PIN	27	// Interior lighting ring for camera and panic functions USING NEOPIXEL LIGHT ON CONTROLLER LID FOR INTERIOR LIGHTING FOR NOW
#define AUX_RELAY_PIN		12  // Aux relay - spare, for future development?
#define LID_SWITCH			4	// Mag switch on lid
#define PANIC_PIR_SNSR		22	// Passive Infrared sensor for panic release **************************change this pin, is not on this pin***************************
#define KEYPAD_TRIGGER		14	// Latching pushbutton for  panic release, backlit

// NeoPixel Info
#define PIXEL_PIN     23  // Digital IO pin 
#define PIXEL_COUNT    2   // Number of NeoPixels

// Declare our NeoPixel strip object
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> *stripbus = nullptr;
#define NEO_PIN_INIT() 				pinMode(PIXEL_PIN, OUTPUT)//Neopixel status indicators



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


//Defines for Mapping colors by name
enum COLOR_MAP_INDEXES{COLOR_RED_INDEX=0,COLOR_PURPLE_INDEX,COLOR_GREEN_INDEX,COLOR_CYAN_INDEX,COLOR_BLUE_INDEX,COLOR_YELLOW_INDEX,COLOR_WHITE_INDEX,COLOR_MAP_NONE_ID};
#define PRESET_COLOR_MAP_INDEXES_MAX COLOR_MAP_NONE_ID   
HsbColor preset_color_map[PRESET_COLOR_MAP_INDEXES_MAX];

enum NOTIFICATIONS{ANIMATION_MODE_NOTIFICATIONS_ID=0, ANIMATION_MODE_NONE};
uint16_t neo_mode = ANIMATION_MODE_NOTIFICATIONS_ID;

//Defines for NEOpixel status (BLINK/SOLID) mode
enum NOTIF_MODE{NOTIF_MODE_OFF_ID=0, NOTIF_MODE_STATIC_OFF_ID, NOTIF_MODE_STATIC_ON_ID, NOTIF_MODE_BLINKING_OFF_ID,
NOTIF_MODE_BLINKING_ON_ID,NOTIF_MODE_TOGGLE_COLORS_ON,NOTIF_MODE_TOGGLE_COLORS_OFF,
NOTIF_MODE_ALTERNATE_COLOR_1,NOTIF_MODE_ALTERNATE_COLOR_2,
NOTIF_MODE_PULSING_ON_ID,NOTIF_MODE_PULSING_OFF_ID,MODE_NONE_ID}; 

//Switch-case for NEOpixel Status indicator lights
enum FEEDBACK_STATUS{FEEDBACK_STATUS_OFF=0, FEEDBACK_STATUS_READY, FEEDBACK_STATUS_UNLOCKING,FEEDBACK_STATUS_OPEN,FEEDBACK_STATUS_AJAR_ERROR,
FEEDBACK_STATUS_CLOSED_COUNTING,FEEDBACK_STATUS_LOCKING,FEEDBACK_STATUS_LOCKED,FEEDBACK_STATUS_READY_RETRIEVE, FEEDBACK_STATUS_BLINKING_PANIC,STATUS_NONE_ID};
uint8_t status = FEEDBACK_STATUS_UNLOCKING;

//Switch-case for vault states
enum STATES_LID{STATE_UNLOCKING=0,	// 0
				STATE_LOCKING,		// 1
				STATE_CLOSED,		// 2
				STATE_OPENED,		// 3
				STATE_LOCKED,		// 4
				STATE_QUALIFIER,	// 5
				STATE_READY};		// 6
int box_state = STATE_READY; // Starting state for the vault at power up

    #define STATE_UNLOCKING_CTR           		"UNLOCKING"
    #define STATE_LOCKING_CTR    				"LOCKING"
    #define STATE_OPENED_CTR     				"OPENED"
    #define STATE_CLOSED_CTR  					"CLOSED"
    #define STATE_LOCKED_CTR   					"LOCKED"
    #define STATE_QUALIFIER_CTR   				"QUALIFYING"
	
    #define FEEDBACK_STATUS_OFF_CTR           	"FS OFF"
    #define FEEDBACK_STATUS_READY_CTR    		"FS READY"
    #define FEEDBACK_STATUS_UNLOCKING_CTR     	"FS UNLOCKING"
    #define FEEDBACK_STATUS_OPEN_CTR  			"FS OPEN"
    #define FEEDBACK_STATUS_AJAR_ERROR_CTR    	"FS AJAR"
    #define FEEDBACK_STATUS_CLOSED_COUNTING_CTR "FS COUNTING"
    #define FEEDBACK_STATUS_LOCKING_CTR    		"FS LOCKING"
	#define FEEDBACK_STATUS_LOCKED_CTR    		"FS LOCKED"
	#define FEEDBACK_STATUS_READY_RETRIEVE_CTR  "FS RETRIEVE READY"
	#define FEEDBACK_STATUS_BLINKING_PANIC_CTR  "FS PANIC"


//BASE VAULT FSM TIMER RELATED ITEMS...............................................................................................................................

// Timer Intervals - ALL non-blocking timers
#define LID_OPEN_INTERVAL	120000		// Lid ajar timer interval, sends ajar message if lid is left open
#define LOCKDOWN_INTERVAL	10000		// Period of time before lockdown of vault after lid close, 10 seconds
#define RELAY_INTERVAL		6000		// Lock/Unlock relay operation time for those functions, 6 seconds
#define DEBOUNCE_INTERVAL	200			// Button Debounce
#define RESPONSE_INTERVAL	500			// Timed response for debug serial.print
#define RESPONSE_INTERVAL2	2000		// Timed response for debug serial.print
#define RESPONSE_INTERVAL3	3000		// Timed response for debug serial.print

// DONT TOUCH THIS AT ALL
// ITS CREATING YOUR OWN "INT", ITS CREATING A VARIABLE TYPE (EG char, int, unit8_t etc)
typedef struct TIMER_HANDLER{
  uint32_t millis = 0;
  uint8_t run = false; // run immediately
}timereached_t;

	//Individual State Timers
	timereached_t tUnlock0;
	timereached_t tUnlock1;
	timereached_t tLock0;
	timereached_t tLock1;
	timereached_t tResponse;
	timereached_t tDebounce;
	timereached_t tLockdown;
	timereached_t tAjar;
	timereached_t tKeycheck;
	timereached_t tPircheck;
	timereached_t tStatusCheck;
	timereached_t tSavedFeedbackDisplay;




// NEOPIXEL RELATED STUFF.......................................................................................................................

float    Hue360toFloat(uint16_t hue);
float    Sat100toFloat(uint8_t sat);
float    Brt100toFloat(uint8_t brt);
uint16_t HueFloatto360(float hue);
uint8_t  SatFloatto100(float sat);
uint8_t  BrtFloatto100(float brt);

struct NOTIF{
    	uint8_t fForceStatusUpdate = false;
    	uint8_t fShowStatusUpdate  = false;
    	struct TSAVED{
    		uint32_t ForceUpdate = millis();
			uint32_t AutoOff = millis();
    	}tSaved;
    	struct PIXELN{
			uint8_t  mode = NOTIF_MODE_PULSING_ON_ID; // Type of light pattern
			uint16_t period_ms = 1000; // Time between fully on and off
			uint8_t pulse_progess = 0; // Used for pulsing only
			HsbColor color; // colour of the led
			HsbColor alternate_colors[2];
			uint32_t tSavedUpdate; // millis last updated
			uint16_t tRateUpdate = 10; // time between updating
			uint16_t auto_time_off_secs = 0; // reset pixel to off
    	}pixel[PIXEL_COUNT];
	}notif;




//Function prototypes
void changeState(int new_state);
void Actuator_Tasker(void);
void PanicSensorCheck();
void KeypadCheck();
void changeStatus(int new_status);
uint8_t TimeReached(TIMER_HANDLER* tSaved, uint32_t ElapsedTime);
uint8_t TimeReached(uint32_t* tSaved, uint32_t ElapsedTime);
void NeoStatus_Tasker(void);
void NeoStatus_SubTask();
void init_Colormap(void);
void NEO_Feedback_Display(void);




