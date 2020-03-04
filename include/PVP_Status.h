#ifndef PVP_STATUS_H
#define PVP_STATUS_H


#include <Arduino.h>
#include <stdint.h>
#include <NeoPixelBus.h>
    
// NeoPixel Info
#define PIXEL_PIN     23  // Digital IO pin 
#define PIXEL_COUNT    2   // Number of NeoPixels

// Declare our NeoPixel strip object
NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> *stripbus = nullptr;
#define NEO_PIN_INIT() 				pinMode(PIXEL_PIN, OUTPUT)//Neopixel status indicators

float    Hue360toFloat(uint16_t hue);
float    Sat100toFloat(uint8_t sat);
float    Brt100toFloat(uint8_t brt);
uint16_t HueFloatto360(float hue);
uint8_t  SatFloatto100(float sat);
uint8_t  BrtFloatto100(float brt);


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



//Function Prototypes for Status_Tasker.cpp
void NeoStatus_Tasker(void);
void NeoStatus_SubTask();
void init_Colormap(void);
void NEO_Feedback_Display(void);
bool TimeReached(uint32_t tSaved, uint32_t ElapsedTime);
void changeStatus(int new_status);

    

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

#endif //PVP_Status.h
