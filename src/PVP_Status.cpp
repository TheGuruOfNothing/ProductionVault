#include <Arduino.h>
#include <NeoPixelBus.h>
#include "PVP_Status.h"

void init_Colormap(){
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
        break;
        case ANIMATION_MODE_NONE: default: break; // resting position call be called EVERY loop without doing anything, optional, disable "NeoStatus_Tasker" with a flag
    }
} 


// This function will be called EVERY LOOP because it checks "tRateUpdate" for if each pixel needs to be updated, so dont add anything else in here, how this works is set elsewhere
void NeoStatus_SubTask(){

   // Updates NEO's at 2 min interval OR if force update requested
  if(TimeReached(notif.tSaved.ForceUpdate,120000)||(notif.fForceStatusUpdate)){
    notif.fForceStatusUpdate = true;
    Serial.println("OPTIONAL FORCING, I USE IT TO MAKE SURE PIXELS NEVER GET STUCK ON SOMETHING NOT INTENDED");
  }
  
  //Animation Types
  for(int i=0;i<PIXEL_COUNT;i++){
    if(TimeReached(notif.pixel[i].tSavedUpdate,notif.pixel[i].tRateUpdate)||(notif.fForceStatusUpdate)){ notif.fForceStatusUpdate = false;
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
  if(TimeReached(notif.tSaved.AutoOff,1000)){// if 1 second past
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
		Serial.println("FS Ready");
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];

			notif.pixel[1].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_GREEN_INDEX];
			
		break;
		case FEEDBACK_STATUS_UNLOCKING:
			Serial.println("FS Unlocking");
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
		   Serial.println("FS Open");
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
			Serial.println("FS AJAR");
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
			Serial.println("FS Counting");
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
		Serial.println("FS Locking");
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
			Serial.println("FS Locked");
			//notif.pixel[1].period_ms = 1000; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;
			
			notif.pixel[1].mode = NOTIF_MODE_STATIC_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_RED_INDEX];
			//notif.pixel[1].auto_time_off_secs = 8;

		break;
		case FEEDBACK_STATUS_READY_RETRIEVE:
			Serial.println("FS Ready Retrieve");
			notif.pixel[0].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[0].color = preset_color_map[COLOR_GREEN_INDEX];

			notif.pixel[1].period_ms = 500; // 0.5 second between "on"s, so half second toggling
			notif.pixel[1].mode = NOTIF_MODE_BLINKING_ON_ID;
			notif.pixel[1].color = preset_color_map[COLOR_GREEN_INDEX];

		break;
		case FEEDBACK_STATUS_BLINKING_PANIC:
			Serial.println("FS Panic");
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

/* EXAMPLE CODE BLOCK THAT COVERS ALL ITERATIONS OF MODE

notif.pixel[0].period_ms = 4000; // 1 second between "on"s, so half second toggling
			notif.pixel[0].mode = NOTIF_MODE_ALTERNATE_COLOR_1;
			notif.pixel[0].alternate_colors[0] = preset_color_map[COLOR_YELLOW_INDEX];
			notif.pixel[0].alternate_colors[1] = preset_color_map[COLOR_PURPLE_INDEX];
			//notif.pixel[0].tRateUpdate ; = SET INTERNALLY, not directly
			notif.pixel[0].auto_time_off_secs = 10;
			Serial.println("Blinking pixel 0 green");*/

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

	uint32_t tSavedFeedbackDisplay = millis();


// Time elapsed function that updates the time when true
bool TimeReached(uint32_t tSaved, uint32_t ElapsedTime){
  if(abs(millis()-tSaved)>=ElapsedTime){ tSaved=millis();
    return true;
  }
  return false;
}

void changeStatus(int new_status){
	
	status = new_status;
	
}
