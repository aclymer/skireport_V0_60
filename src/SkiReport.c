#include "pebble.h"

static Window *resort;
static TextLayer *resort_top_text_layer;
static TextLayer *resort_center_text_layer;
static TextLayer *resort_bottom_text_layer;
ActionBarLayer *resort_action_bar;
static GBitmap *up_arrow_icon;
static GBitmap *right_arrow_icon;
static GBitmap *down_arrow_icon;

static Window *window;
ActionBarLayer *window_action_bar;
static TextLayer *weather_layer;
static TextLayer *wind_layer;
static TextLayer *temps_layer;
static TextLayer *snowfall_layer;
static TextLayer *update_layer;
static TextLayer *area_name_layer;
static GBitmap *settings_icon;

static Window *settings;
ActionBarLayer *settings_action_bar;
static TextLayer *settings_US_text_layer;
static TextLayer *settings_METRIC_text_layer;
static GBitmap *ok_icon;

int units = 0;

int index = 0;
static AppSync sync;
static uint8_t sync_buffer[1536];

enum key {
	INDEX_KEY = 0X0,			// TUPLE_INTEGER
	AREA_NAME_KEY = 0x1,		// TUPLE_CSTRING
	WIND_KEY = 0x2,				// TUPLE_CSTRING
	AREA_TEMPS_KEY = 0x3,		// TUPLE_CSTRING
	AREA_SNOWFALL_KEY = 0x4,	// TUPLE_CSTRING
	UPDATE_TIME_KEY = 0x5,		// TUPLE_CSTRING
	WEATHER_DESC_KEY = 0x6,		// TUPLE_CSTRING
	UNITS_KEY = 0x7				// TUPLE_INTEGER
};

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
	switch (key) {
		
		case AREA_NAME_KEY:
		text_layer_set_text(area_name_layer, new_tuple->value->cstring);
		break;
		
		case WIND_KEY:
		text_layer_set_text(wind_layer, new_tuple->value->cstring);
		break;
		
		case AREA_TEMPS_KEY:
		text_layer_set_text(temps_layer, new_tuple->value->cstring);
		break;
		
		case AREA_SNOWFALL_KEY:
		text_layer_set_text(snowfall_layer, new_tuple->value->cstring);
		break;
		
		case UPDATE_TIME_KEY:
		text_layer_set_text(update_layer, new_tuple->value->cstring);
		break;	  
		
		case WEATHER_DESC_KEY:
		text_layer_set_text(weather_layer, new_tuple->value->cstring);
		break;
	}
}

static void send_cmd(void) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
	
	Tuplet value = TupletInteger(0, index);	
	dict_write_tuplet(iter, &value);
	Tuplet load = TupletCString(1, "Loading...");
	dict_write_tuplet(iter, &load);
	Tuplet snow = TupletCString(4, "Snow(24h): ---  ");
	dict_write_tuplet(iter, &snow);
	Tuplet unit = TupletInteger(7, units);
	dict_write_tuplet(iter, &unit);
	dict_write_end(iter);
	
	app_message_outbox_send();
}

void resort_up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	
}

void resort_down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	
}

void resort_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	// For Select Feature - Select Resort and show data
	send_cmd();
	window_stack_push(window, true);
}

void settings_up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (units == 1) {
		text_layer_set_font(settings_US_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
		text_layer_set_font(settings_METRIC_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	}
	units = 0;
}

void settings_down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (units == 0) {
		text_layer_set_font(settings_US_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
		text_layer_set_font(settings_METRIC_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	}
	units = 1;
}

void settings_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	// For Select Feature - Set Settings & return to main Window
	persist_write_int(0, units);
	send_cmd();
	window_stack_pop(true);
	window_stack_push(window, true);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (index > 0) {
		index += 1;
	}
	else {index = 9;}
	send_cmd();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (index < 9) {
		index += 1;
	}
	else {index = 0;}
	send_cmd();
}
void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	// Push settings window onto stack
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	// For Select Feature
	window_stack_push(settings, true);	
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
	if (app_message_error == 64) {send_cmd();}
}

void resort_click_config_provider(void *context) {
	
	//	Single Click Up Action
	window_single_click_subscribe(BUTTON_ID_UP, resort_up_single_click_handler);
	
	//	Single Click Down
	window_single_click_subscribe(BUTTON_ID_DOWN, resort_down_single_click_handler);
	
	//	Single Click Select
	window_single_click_subscribe(BUTTON_ID_SELECT, resort_select_single_click_handler);
}

void click_config_provider(void *context) {
	
	//	Single Click Up Action
	window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
	
	//	Single Click Down
	window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
	
	//	Single Click Select
	window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
	
	//	Long Click Select
	window_long_click_subscribe(BUTTON_ID_SELECT, (uint16_t) 500, select_long_click_handler, NULL);
}

void settings_click_config_provider(void *context) {
	
	//	Single Click Up Action
	window_single_click_subscribe(BUTTON_ID_UP, settings_up_single_click_handler);
	
	//	Single Click Down
	window_single_click_subscribe(BUTTON_ID_DOWN, settings_down_single_click_handler);
	
	//	Single Click Select
	window_single_click_subscribe(BUTTON_ID_SELECT, settings_select_single_click_handler);
}

static void resort_load(Window *resort) {
	Layer *resort_layer = window_get_root_layer(resort);
	
	//Set up Settings Action Bar
	up_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP_ARROW);
	down_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN_ARROW);
	right_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_RIGHT_ARROW);
	resort_action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(resort_action_bar, resort);
	action_bar_layer_set_background_color(resort_action_bar, GColorWhite);
	action_bar_layer_set_click_config_provider(resort_action_bar, resort_click_config_provider);
	action_bar_layer_set_icon(resort_action_bar, BUTTON_ID_UP, up_arrow_icon);
	action_bar_layer_set_icon(resort_action_bar, BUTTON_ID_DOWN, down_arrow_icon);
	action_bar_layer_set_icon(resort_action_bar, BUTTON_ID_SELECT, right_arrow_icon);
	
	// Resort Top Text Layer
	resort_top_text_layer = text_layer_create(GRect(20, 32, 80, 30));
	text_layer_set_text_color(resort_top_text_layer, GColorWhite);
	text_layer_set_background_color(resort_top_text_layer, GColorClear);
	text_layer_set_font(resort_top_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(resort_top_text_layer, GTextAlignmentCenter);
	text_layer_set_text(resort_top_text_layer, "*         *");
	layer_add_child(resort_layer, text_layer_get_layer(resort_top_text_layer));
	
	// Resort Center Text Layer
	resort_center_text_layer = text_layer_create(GRect(20, 82, 80, 30));
	text_layer_set_text_color(resort_center_text_layer, GColorWhite);
	text_layer_set_background_color(resort_center_text_layer, GColorClear);
	text_layer_set_font(resort_center_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(resort_center_text_layer, GTextAlignmentCenter);
	text_layer_set_text(resort_center_text_layer, "Loading...");
	layer_add_child(resort_layer, text_layer_get_layer(resort_center_text_layer));
	
	// Resort Bottom Text Layer
	resort_bottom_text_layer = text_layer_create(GRect(20, 32, 80, 30));
	text_layer_set_text_color(resort_bottom_text_layer, GColorWhite);
	text_layer_set_background_color(resort_bottom_text_layer, GColorClear);
	text_layer_set_font(resort_bottom_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(resort_bottom_text_layer, GTextAlignmentCenter);
	text_layer_set_text(resort_bottom_text_layer, "*         *");
	layer_add_child(resort_layer, text_layer_get_layer(resort_bottom_text_layer));}

static void settings_load(Window *settings) {
	Layer *settings_layer = window_get_root_layer(settings);
	
	//Set up Settings Action Bar
	up_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP_ARROW);
	down_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN_ARROW);
	ok_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_OK);
	settings_action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(settings_action_bar, settings);
	action_bar_layer_set_background_color(settings_action_bar, GColorWhite);
	action_bar_layer_set_click_config_provider(settings_action_bar, settings_click_config_provider);
	action_bar_layer_set_icon(settings_action_bar, BUTTON_ID_UP, up_arrow_icon);
	action_bar_layer_set_icon(settings_action_bar, BUTTON_ID_DOWN, down_arrow_icon);
	action_bar_layer_set_icon(settings_action_bar, BUTTON_ID_SELECT, ok_icon);
	
	// Settings US Text Layer
	settings_US_text_layer = text_layer_create(GRect(20, 32, 80, 30));
	text_layer_set_text_color(settings_US_text_layer, GColorWhite);
	text_layer_set_background_color(settings_US_text_layer, GColorClear);
	if (units == 0) {
		text_layer_set_font(settings_US_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	}
	else {
		text_layer_set_font(settings_US_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	}
	text_layer_set_text_alignment(settings_US_text_layer, GTextAlignmentCenter);
	text_layer_set_text(settings_US_text_layer, "US");
	layer_add_child(settings_layer, text_layer_get_layer(settings_US_text_layer));
	
	// Settings Metric Text Layer
	settings_METRIC_text_layer = text_layer_create(GRect(20, 82, 80, 30));
	text_layer_set_text_color(settings_METRIC_text_layer, GColorWhite);
	text_layer_set_background_color(settings_METRIC_text_layer, GColorClear);
	if (units == 0) {
		text_layer_set_font(settings_METRIC_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	}
	else {
		text_layer_set_font(settings_METRIC_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	}
	text_layer_set_text_alignment(settings_METRIC_text_layer, GTextAlignmentCenter);
	text_layer_set_text(settings_METRIC_text_layer, "METRIC");
	layer_add_child(settings_layer, text_layer_get_layer(settings_METRIC_text_layer));
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	static int row_height = 22;
	int it = 2;
	const int button_bar_width = 20;
	const int text_layer_width = 144 - button_bar_width;
	
	//Set up Window Action Bar
	up_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP_ARROW);
	down_arrow_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN_ARROW);
	settings_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_SETTINGS);
	window_action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(window_action_bar, window);
	action_bar_layer_set_background_color(window_action_bar, GColorBlack);
	action_bar_layer_set_click_config_provider(window_action_bar, click_config_provider);
	action_bar_layer_set_icon(window_action_bar, BUTTON_ID_UP, up_arrow_icon);
	action_bar_layer_set_icon(window_action_bar, BUTTON_ID_DOWN, down_arrow_icon);
	action_bar_layer_set_icon(window_action_bar, BUTTON_ID_SELECT, settings_icon);

	// Area Name Layer	
	area_name_layer = text_layer_create(GRect(0, -2, text_layer_width, 48));
	text_layer_set_text_color(area_name_layer, GColorWhite);
	text_layer_set_background_color(area_name_layer, GColorClear);
	text_layer_set_font(area_name_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(area_name_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(area_name_layer, GTextOverflowModeWordWrap);
	layer_add_child(window_layer, text_layer_get_layer(area_name_layer));
	
	// Weather Description Layer
	weather_layer = text_layer_create(GRect(0, it++ * row_height, text_layer_width, row_height));
	text_layer_set_text_color(weather_layer, GColorWhite);
	text_layer_set_background_color(weather_layer, GColorClear);
	text_layer_set_font(weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(weather_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(weather_layer));
	
	// Snowfall Layer	
	snowfall_layer = text_layer_create(GRect(0, it++ * row_height, text_layer_width, row_height));
	text_layer_set_text_color(snowfall_layer, GColorWhite);
	text_layer_set_background_color(snowfall_layer, GColorClear);
	text_layer_set_font(snowfall_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(snowfall_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(snowfall_layer));
	
	// Temperature Layer
	temps_layer = text_layer_create(GRect(0, it++ * row_height, text_layer_width, row_height));
	text_layer_set_text_color(temps_layer, GColorWhite);
	text_layer_set_background_color(temps_layer, GColorClear);
	text_layer_set_font(temps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(temps_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(temps_layer));
	
	// Wind Layer	
	wind_layer = text_layer_create(GRect(0, it++ * row_height, text_layer_width, row_height));
	text_layer_set_text_color(wind_layer, GColorWhite);
	text_layer_set_background_color(wind_layer, GColorClear);
	text_layer_set_font(wind_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(wind_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(wind_layer));
	
	// Update Time Layer
	update_layer = text_layer_create(GRect(0, it++ * row_height + 4, text_layer_width, row_height));
	text_layer_set_text_color(update_layer, GColorWhite);
	text_layer_set_background_color(update_layer, GColorClear);
	text_layer_set_font(update_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	text_layer_set_text_alignment(update_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(update_layer));
	
	Tuplet initial_values[] = {
		TupletInteger(INDEX_KEY, (uint8_t) 0),
		TupletCString(AREA_NAME_KEY, "Loading..."),
		TupletCString(WEATHER_DESC_KEY, "Loading..."),
		TupletCString(WIND_KEY, "Wind: --- @ -- ---"),
		TupletCString(AREA_TEMPS_KEY, "Temp(--): -- to --"),
		TupletCString(AREA_SNOWFALL_KEY, "Snow(24h): --   "),
		TupletCString(UPDATE_TIME_KEY, "Updated @ --:--"),
		TupletInteger(UNITS_KEY, (uint8_t) 0),
	};
	
	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
				  sync_tuple_changed_callback, sync_error_callback, NULL);
	
	send_cmd();
}

static void resort_unload(Window *resort) {
	text_layer_destroy(resort_top_text_layer);
	text_layer_destroy(resort_center_text_layer);
	text_layer_destroy(resort_bottom_text_layer);
	action_bar_layer_destroy(resort_action_bar);
	gbitmap_destroy(right_arrow_icon);
	window_stack_pop(true);
}

static void settings_unload(Window *settings) {
	text_layer_destroy(settings_US_text_layer);
	text_layer_destroy(settings_METRIC_text_layer);
	action_bar_layer_destroy(settings_action_bar);
	window_stack_pop(true);
}

static void window_unload(Window *window) {
	app_sync_deinit(&sync);
	text_layer_destroy(wind_layer);
	text_layer_destroy(weather_layer);
	text_layer_destroy(area_name_layer);
	text_layer_destroy(temps_layer);
	text_layer_destroy(snowfall_layer);
	text_layer_destroy(update_layer);
	gbitmap_destroy(settings_icon);
	action_bar_layer_destroy(window_action_bar);
}

static void init(void) {	
	// Create Main Window & Load
	window = window_create();
	window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Done initializing, pushed window: %p", window);
	window_set_background_color(window, GColorBlack);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
	
	// Create Settings Window & Load
	settings = window_create();
	window_set_click_config_provider(settings, (ClickConfigProvider) settings_click_config_provider);
	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Pushing window: %p", settings);
	window_set_background_color(settings, GColorBlack);
	window_set_window_handlers(settings, (WindowHandlers) {
		.load = settings_load,
		.unload = settings_unload
	});
	
	// Create Resort Window & Load
	resort = window_create();
	window_set_click_config_provider(resort, (ClickConfigProvider) resort_click_config_provider);
	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Pushing window: %p", resort);
	window_set_background_color(resort, GColorBlack);
	window_set_window_handlers(resort, (WindowHandlers) {
		.load = resort_load,
		.unload = resort_unload
	});
	
	const int inbound_size = 1024;
	const int outbound_size = 512;
	app_message_open(inbound_size, outbound_size);
	
	window_stack_push(window, true);
}

static void deinit(void) {
	window_destroy(window);
	window_destroy(settings);
}

int main(void) {
	if (persist_exists(0) == true) units = persist_read_int(0);
	init();
	app_event_loop();
	deinit();
}