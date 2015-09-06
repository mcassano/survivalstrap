#include <pebble.h>
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_SG_MENU_ITEMS 2

#define TIMEOUT_MS 1000
#define MAX_READ_SIZE 100

static TextLayer *s_attr_text_layer;
static TextLayer *s_raw_text_layer;
static char s_text_buffer1[20];
static char s_text_buffer2[20];
static SmartstrapAttribute *s_raw_attribute;
static SmartstrapAttribute *s_attr_attribute;

static int isBeaconing = 0;
static int continueBeaconing = 0;

static Window *s_main_window;
static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem s_first_menu_items[NUM_FIRST_MENU_ITEMS];
static GBitmap *s_menu_icon_image;

// Beacon Window variables
static Window *beacon_window;
static TextLayer *beacon_text_layer;

// Survival Guide Variables
static Window *survival_guide_menu_window;
static SimpleMenuLayer *sg_menu_layer;
static SimpleMenuSection sg_menu_section[1];
static SimpleMenuItem sg_menu_items[NUM_SG_MENU_ITEMS];

static Window *sg_fire_window;
static Window *sg_shelter_window;

static TextLayer *latitudeTextLayer;
static TextLayer *longitudeTextLayer;

static TextLayer *sg_text_layer;
static char sg_fire_text[] = "Make a Fire, Dummy";
static char sg_shelter_text[] = "Build a Shelter, Dummy";

static void prv_update_text(void) {
  if (smartstrap_service_is_available(SMARTSTRAP_RAW_DATA_SERVICE_ID)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Connected!");
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Not connected!");
  }
}

static void prv_did_read(SmartstrapAttribute *attr, SmartstrapResult result,
                              const uint8_t *data, size_t length) {
  if (attr == s_attr_attribute) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "did_read(s_attr_attribute, %d, %d)", result, length);
    if (result == SmartstrapResultOk && length == 4) {
      uint32_t num;
      memcpy(&num, data, 4);
    }
  } else if (attr == s_raw_attribute) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "did_read(s_raw_attribute, %d, %d)", result, length);
    if (result == SmartstrapResultOk && length == 4) {
      uint32_t time;
      memcpy(&time, data, 4);
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "did_read(<%p>, %d)", attr, result);
  }
}

static void prv_did_write(SmartstrapAttribute *attr, SmartstrapResult result) {
  if (attr == s_attr_attribute) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "did_write(s_attr_attribute, %d)", result);
  } else if (attr == s_raw_attribute) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "did_write(s_raw_attribute, %d)", result);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "did_write(<%p>, %d)", attr, result);
  }
}

static void prv_write_read_test_attr(void) {
  SmartstrapResult result;
  if (!smartstrap_service_is_available(smartstrap_attribute_get_service_id(s_attr_attribute))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "s_attr_attribute is not available");
    return;
  }

  // get the write buffer
  uint8_t *buffer = NULL;
  size_t length = 0;
  result = smartstrap_attribute_begin_write(s_attr_attribute, &buffer, &length);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Write of s_attr_attribute failed with result %d", result);
    return;
  }

  // write the data into the buffer
  snprintf((char*)buffer, length, "Hello, smartstrap!");

  // send it off
  result = smartstrap_attribute_end_write(s_attr_attribute, sizeof(buffer), true);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Write of s_attr_attribute failed with result %d", result);
  }
}

static void prv_read_raw(void) {
  if (!smartstrap_service_is_available(smartstrap_attribute_get_service_id(s_raw_attribute))) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "s_raw_attribute is not available");
    return;
  }
  SmartstrapResult result = smartstrap_attribute_read(s_raw_attribute);
  if (result != SmartstrapResultOk) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Read of s_raw_attribute failed with result: %d", result);
  }
}

static void prv_send_request(void *context) {
  prv_write_read_test_attr();
  prv_read_raw();
  if (continueBeaconing) {
    app_timer_register(1000, prv_send_request, NULL);    
  } else {
    
  }
}

static void prv_availablility_status_changed(SmartstrapServiceId service_id, bool is_available) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Availability for 0x%x is %d", service_id, is_available);
  prv_update_text();
}

static void prv_notified(SmartstrapAttribute *attr) {
  if (attr == s_attr_attribute) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "notified(s_attr_attribute)");
  } else if (attr == s_raw_attribute) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "notified(s_raw_attribute)");
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "notified(<%p>)", attr);
  }
}

static void enableBeacon(void) {
  app_timer_register(1000, prv_send_request, NULL);
  continueBeaconing = 1;
}

static void disableBeacon(void) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "LAT: %s", text_layer_get_text(latitudeTextLayer));
  continueBeaconing = 0;
}

void my_next_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "CENTER BUTTON");
  Window *window = (Window *)context;
  text_layer_destroy(beacon_text_layer);

  beacon_text_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(beacon_text_layer, GColorClear);
  text_layer_set_text_color(beacon_text_layer, GColorBlack);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "CENTER BUTTON 2");
  if (isBeaconing) {
    text_layer_set_text(beacon_text_layer, "BEACON IS OFF");
    isBeaconing = 0;
    disableBeacon();
  } else {
    text_layer_set_text(beacon_text_layer, "BEACON IS ON");
    isBeaconing = 1;
    enableBeacon();
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "CENTER BUTTON3");
  // Improve the layout to be more like a watchface
  text_layer_set_font(beacon_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(beacon_text_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer));
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) my_next_click_handler);
}

static void beacon_window_load(Window *window) {
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

  beacon_text_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(beacon_text_layer, GColorClear);
  text_layer_set_text_color(beacon_text_layer, GColorBlack);
  text_layer_set_text(beacon_text_layer, "BEACON IS OFF");

  // Improve the layout to be more like a watchface
  text_layer_set_font(beacon_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(beacon_text_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer));
  
  Tuplet initial_values[] = {
    TupletInteger(7, (uint8_t) 0),
    TupletInteger(12, (uint8_t) 0)
  };
  /*
   app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  request_weather();
  */
}


//BEACON WINDOW


void beacon_window_unload(Window *window) {
  text_layer_destroy(beacon_text_layer);
}

static void beacon_select_callback(int index, void *ctx) {
  //s_first_menu_items[index].subtitle = "You've hit select here!";
  //layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
  
  beacon_window = window_create();
  window_set_window_handlers(beacon_window, (WindowHandlers) {
    .load = beacon_window_load,
    .unload = beacon_window_unload,
  });
  window_stack_push(beacon_window, true);
}

// BUILD FIRE WINDOW
static void sg_fire_window_load(Window *window) {
//  s_menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_1);
  window_set_background_color(window, GColorRed);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  sg_text_layer = text_layer_create(bounds);
  
  text_layer_set_background_color(sg_text_layer, GColorRed);
  text_layer_set_text_color(sg_text_layer, GColorWhite);
  text_layer_set_text(sg_text_layer, sg_fire_text);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(sg_text_layer));
}

// SHELTER WINDOW
static void sg_shelter_window_load(Window *window) {
//  s_menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_1);

  window_set_background_color(window, GColorRed);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  sg_text_layer = text_layer_create(bounds);
  
  text_layer_set_background_color(sg_text_layer, GColorBrass);
  text_layer_set_text_color(sg_text_layer, GColorWhite);
  text_layer_set_text(sg_text_layer, sg_shelter_text);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(sg_text_layer));
}

void sg_content_window_unload(Window *window) {
  text_layer_destroy(sg_text_layer);
}

static void sg_select_callback(int index, void *ctx) {
  if (index == 0) {
    sg_fire_window = window_create();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "selected fire");
    window_set_window_handlers(sg_fire_window, (WindowHandlers) {
      .load = sg_fire_window_load,
      .unload = sg_content_window_unload
    });
    window_stack_push(sg_fire_window, true);

  }
  else {
    sg_shelter_window = window_create();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "selected shelter");
    window_set_window_handlers(sg_shelter_window, (WindowHandlers) {
      .load = sg_shelter_window_load,
      .unload = sg_content_window_unload
    });
    window_stack_push(sg_shelter_window, true);
  }
  
}


// SURVIVAL GUIDE WINDOW
void sg_menu_window_load(Window *window) {
  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
  // an int as such to easily change the order of menu items later
  int num_sg_items = 0;

  sg_menu_items[num_sg_items++] = (SimpleMenuItem) {
    .title = "Make Fire",
    .callback = sg_select_callback,
  };
  sg_menu_items[num_sg_items++] = (SimpleMenuItem) {
    .title = "Build Shelter",
    .callback = sg_select_callback,
  };
  
  sg_menu_section[0] = (SimpleMenuSection) {
    .num_items = NUM_SG_MENU_ITEMS,
    .items = sg_menu_items,
  };
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  sg_menu_layer = simple_menu_layer_create(bounds, window, sg_menu_section, NUM_MENU_SECTIONS, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(sg_menu_layer));
}

void sg_menu_window_unload(Window *window) {
  simple_menu_layer_destroy(sg_menu_layer);
}

static void survival_guide_select_callback() {
  survival_guide_menu_window = window_create();
  window_set_window_handlers(survival_guide_menu_window, (WindowHandlers) {
    .load = sg_menu_window_load,
    .unload = sg_menu_window_unload,
  });
  window_stack_push(survival_guide_menu_window, true);
}


/* LOAD MAIN WINDOW */
static void main_window_load(Window *window) {
//  s_menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_1);

  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
  // an int as such to easily change the order of menu items later
  int num_a_items = 0;

  s_first_menu_items[num_a_items++] = (SimpleMenuItem) {
    .title = "SOS Beacon",
    .callback = beacon_select_callback,
  };
  s_first_menu_items[num_a_items++] = (SimpleMenuItem) {
    .title = "Survival Guide",
    .callback = survival_guide_select_callback,
  };
  s_menu_sections[0] = (SimpleMenuSection) {
    .num_items = NUM_FIRST_MENU_ITEMS,
    .items = s_first_menu_items,
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_simple_menu_layer = simple_menu_layer_create(bounds, window, s_menu_sections, NUM_MENU_SECTIONS, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
}

void main_window_unload(Window *window) {
  simple_menu_layer_destroy(s_simple_menu_layer);
  //gbitmap_destroy(s_menu_icon_image);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
//  create_windows();
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}