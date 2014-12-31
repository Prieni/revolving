/*
  Revolving bubbles
  watchface by R. Prien
  
  This is where I got the code to start from:
  Polar Clock watch (SDK 2.0)
  PolarClock2.0 by nirajsanghvi (Polar Clock watchface developed by Niraj Sanghvi)


 */

#include "pebble.h"


enum {
      KEY_24H = 0x0,
      KEY_DATE = 0x1,
      KEY_VIBE = 0x2,
		  KEY_INVERT = 0x3
};

static bool SHOW_24H = true;
static bool SHOW_TEXT_DATE = true;
static bool BT_VIBE = true;
static bool INVERT = true;


static GColor BACKGROUND_COLOR = GColorBlack;
static GColor FOREGROUND_COLOR = GColorWhite;

//static char time_text[] = "00:00";
static char date_text[] = "00 Xxx";
///static char date_row_text[] = "00 Xxx";
static char hour_text[] = "23";
static char hour_text_mod[] = "23";
static char min_text[] = "59";

Window *window;

Layer *minute_display_layer;
Layer *hour_display_layer;
Layer *battery_display_layer;

TextLayer *text_time_layer;
TextLayer *text_date_layer;
bool time_layer_exists = false;
bool date_layer_exists = false;


static void minute_display_layer_update_callback(Layer *layer, GContext* ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned int angle = t->tm_min * 6;

  GRect bounds = layer_get_bounds(layer);
  GRect min_rect;
  GPoint center = grect_center_point(&bounds);
  
  GPoint min_center; // center of the minute circle
  min_center.x = sin_lookup((TRIG_MAX_ANGLE / 360) * angle)*54/TRIG_MAX_RATIO + center.x;
  min_center.y = -cos_lookup((TRIG_MAX_ANGLE / 360) * angle)*54/TRIG_MAX_RATIO + center.y;
  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
  graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
  graphics_fill_circle(ctx, min_center, 17);
  
  strftime(min_text,sizeof(hour_text),"%M",t);

  min_rect.origin.x = min_center.x - 15;
  min_rect.origin.y = min_center.y - 18;
  min_rect.size.w = 30;
  min_rect.size.h = 15;
  graphics_context_set_text_color(ctx,BACKGROUND_COLOR);
  graphics_draw_text(ctx,min_text,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),min_rect,GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
}


static void hour_display_layer_update_callback(Layer *layer, GContext* ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  GRect bounds = layer_get_bounds(layer);
  GRect hrs_rect;
  GPoint center = grect_center_point(&bounds);
  GPoint am_pm;
  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
  graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
  graphics_fill_circle(ctx,center,31); // circle for hours

  if(SHOW_24H) {
      strftime(hour_text,sizeof(hour_text),"%H",t);
    if( t->tm_hour < 10 ){
      strcpy(hour_text_mod,hour_text+1);
    }
    else {
      strcpy(hour_text_mod,hour_text);
    }
  }
  else {
      strftime(hour_text,sizeof(hour_text),"%I",t);
      
     if(t->tm_hour >= 12){
       am_pm.x = center.x;
       am_pm.y = center.y + 23;
       graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
       graphics_fill_circle(ctx,am_pm,2); // pm indicator
     }
    if( ((t->tm_hour < 10) || ((t->tm_hour > 12)&&(t->tm_hour < 22))) &&(t->tm_hour != 0)){
      strcpy(hour_text_mod,hour_text+1);
    }
    else {
      strcpy(hour_text_mod,hour_text);
    }
  }
  hrs_rect.origin.x = center.x - 41;
  hrs_rect.origin.y = center.y - 26;
  hrs_rect.size.w = 80;
  hrs_rect.size.h = 60;
  graphics_context_set_text_color(ctx,BACKGROUND_COLOR);
  graphics_draw_text(ctx,hour_text_mod,fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS),hrs_rect,GTextOverflowModeWordWrap,GTextAlignmentCenter,NULL);
}

static void battery_display_layer_update_callback(Layer *layer, GContext* ctx) {
    
  GPoint p0,p1;
  BatteryChargeState bat_stat = battery_state_service_peek();
  
  p0.x=0;
  p0.y=167;
  p1.x= (int16_t) (1.4 * bat_stat.charge_percent); // 140 @ 100 percent
  p1.y=167;
  
  graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
  graphics_draw_line(ctx, p0, p1);
  if (INVERT)
    {
    // if inverted, then the battery bar is too thin
    // make it a bit wider (2 pixels)
    p0.y = 166;
    p1.y = 166;
    graphics_draw_line(ctx, p0, p1);
  }
 
}

static void handle_bluetooth_event(bool connected) {
  if(BT_VIBE){
  if (connected)
    vibes_double_pulse();
  else
    vibes_short_pulse();
  }
}
  
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  //(void)ctx;
	
  layer_mark_dirty(minute_display_layer);
  layer_mark_dirty(hour_display_layer);
  layer_mark_dirty(battery_display_layer);

  if (SHOW_TEXT_DATE && date_layer_exists)
  {
	    strftime(date_text, sizeof(date_text), "%d", tick_time);
		  text_layer_set_text(text_date_layer, date_text);
  }
}

static void setup_time_date_layers() {
	
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
  if(time_layer_exists) {
	  text_layer_destroy(text_time_layer);
	  time_layer_exists = false;	
  }
  
  if(date_layer_exists) {
	  text_layer_destroy(text_date_layer);
	  date_layer_exists = false;		
  }
	
  /*
  if (SHOW_TEXT_TIME)
  {
	  text_time_layer = text_layer_create(bounds);
	  time_layer_exists = true;
	  text_layer_set_text_color(text_time_layer, FOREGROUND_COLOR);
	  text_layer_set_background_color(text_time_layer, GColorClear);
	  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	  
	  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
	  	
	  if (SHOW_TEXT_DATE)
	  {
	  	layer_set_frame(text_layer_get_layer(text_time_layer), GRect(0, 57, 144, 168-57));
	  } else {
	  	layer_set_frame(text_layer_get_layer(text_time_layer), GRect(0, 70, 144, 168-70));
	  }
	  
  }*/

  if (SHOW_TEXT_DATE) // TODO: Do we need to do that even if SHOW_TEXT_DATE is false (as it could be set to true in settings)?
  {
	  text_date_layer = text_layer_create(bounds);
	  date_layer_exists = true;
	  text_layer_set_text_color(text_date_layer, FOREGROUND_COLOR);
	  text_layer_set_background_color(text_date_layer, GColorClear);
	  text_layer_set_text_alignment(text_date_layer, GTextAlignmentLeft);
	  
	  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
	  /*
	  if (SHOW_TEXT_TIME)
	  {
		  layer_set_frame(text_layer_get_layer(text_date_layer), GRect(0, 80, 144, 168-80));
	  } else */
    {
    int16_t rect_top = 143;
    if (INVERT) rect_top = 141;
		layer_set_frame(text_layer_get_layer(text_date_layer), GRect(120, rect_top, 66, 68));
	  }
	  
  }
}

 void in_received_handler(DictionaryIterator *received, void *context) {
	 Tuple *time_tuple = dict_find(received, KEY_24H);
	 Tuple *date_tuple = dict_find(received, KEY_DATE);
	 Tuple *vibe_tuple = dict_find(received, KEY_VIBE);
	 Tuple *invert_tuple = dict_find(received, KEY_INVERT);
	 
	 //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_TIME: %d, %d, %d", time_tuple->type, time_tuple->length, time_tuple->value->uint8 );
	 //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_DATE: %d, %d, %d", date_tuple->type, date_tuple->length, date_tuple->value->int8 );
	 //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_VIBE: %d, %d, %d", vibe_tuple->type, vibe_tuple->length, vibe_tuple->value->int8 );
	 //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_INVERT: %d, %d, %d", invert_tuple->type, invert_tuple->length, invert_tuple->value->uint8 );
	 
	 SHOW_24H = time_tuple->value->int8 ? true : false;
	 persist_write_bool(KEY_24H, SHOW_24H);
	 SHOW_TEXT_DATE = date_tuple->value->int8 ? true : false;
	 persist_write_bool(KEY_DATE, SHOW_TEXT_DATE);
	 BT_VIBE = vibe_tuple->value->int8 ? true : false;
	 persist_write_bool(KEY_VIBE, BT_VIBE);
	 INVERT = invert_tuple->value->int8 ? true : false;
	 persist_write_bool(KEY_INVERT, INVERT);
	 if(INVERT)
	 {
		BACKGROUND_COLOR = GColorWhite;
		FOREGROUND_COLOR = GColorBlack;
	 }
	 else
	 {
		BACKGROUND_COLOR = GColorBlack;
		FOREGROUND_COLOR = GColorWhite;		 		 
	 }
	 window_set_background_color(window, BACKGROUND_COLOR);
	 
	 setup_time_date_layers();
	 
	 //if (SHOW_TEXT_TIME && time_layer_exists) text_layer_set_text(text_time_layer, time_text);
	 if (SHOW_TEXT_DATE && date_layer_exists)
	 {
		 /*if (ROW_DATE)
		 {
		  text_layer_set_text(text_date_layer, date_row_text);
		 } else*/
     {
		  text_layer_set_text(text_date_layer, date_text);
		 }
	 }
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }


static void init(void) {
  //(void)ctx;
	
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_open(64, 0);

  if(persist_exists(KEY_24H)) SHOW_24H = persist_read_bool(KEY_24H);
  if(persist_exists(KEY_DATE)) SHOW_TEXT_DATE = persist_read_bool(KEY_DATE);
  if(persist_exists(KEY_VIBE)) BT_VIBE = persist_read_bool(KEY_VIBE);
  if(persist_exists(KEY_INVERT)) INVERT = persist_read_bool(KEY_INVERT);
	
  if(INVERT) {
	  BACKGROUND_COLOR = GColorWhite;
	  FOREGROUND_COLOR = GColorBlack;
  }
  else {
	  BACKGROUND_COLOR = GColorBlack;
	  FOREGROUND_COLOR = GColorWhite;		 		 
  }
	
  window = window_create();
  window_set_background_color(window, BACKGROUND_COLOR);
  window_stack_push(window, true);

 
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
  // Init the layer for the minute display
  minute_display_layer = layer_create(bounds);
  layer_set_update_proc(minute_display_layer, minute_display_layer_update_callback);
  layer_add_child(window_layer, minute_display_layer);

  // Init the layer for the hour display
  hour_display_layer = layer_create(bounds);
  layer_set_update_proc(hour_display_layer, hour_display_layer_update_callback);
  layer_add_child(window_layer, hour_display_layer);

  // Init the layer for the battery display
  battery_display_layer = layer_create(bounds);
  layer_set_update_proc(battery_display_layer, battery_display_layer_update_callback);
  layer_add_child(window_layer, battery_display_layer);
  
  setup_time_date_layers();
	
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  bluetooth_connection_service_subscribe(handle_bluetooth_event); 
}

static void deinit(void) {

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy(window);
  layer_destroy(minute_display_layer);
  layer_destroy(hour_display_layer);
  layer_destroy(battery_display_layer);
	
  if (time_layer_exists) text_layer_destroy(text_time_layer);
  if (date_layer_exists) text_layer_destroy(text_date_layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
} 