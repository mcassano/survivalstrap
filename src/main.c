#include <pebble.h>

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_SG_MENU_ITEMS 2

#define TIMEOUT_MS 1000
#define MAX_READ_SIZE 100

static ScrollLayer *s_scroll_layer;
static TextLayer *s_scroll_text_layer;
static char s_scroll_text[] = "Starting a fire \n\n Step 1: Gather Tinder \n \n -Grass \n -Pine Needles \n - Weed Tops and Seed Down \n -Inner bark \n -Coconut Fibers \n -Cotton Balls \n -Q Tips \n \n \n Step 2: Create Sparks \n \n -Place the tinder on a stable surface, like a log, rock, or the ground \n -Hold the Steel in one hand near the tinder \n - Pull the Ferro Rod towards you while holding the steel still so you don’t disturb the tinder. \n -When the sparks land they should create a cherry red glow \n \n \n Step 3: Add Air \n \n -GENTLY blow on the glowing ember which should create additional smoke, keep adding air until a flame is created.  \n -CAREFULLY add kindling to your fire until you have a stable campfire.  ";

static ScrollLayer *s_shelter_scroll_layer;
static TextLayer *s_shelter_text_layer;
static char s_shelter_scroll_text[] = "Build a Shelter \n\nFallen Tree: Using a fallen tree to get started can save a lot of time, if there’s a fallen tree you can lay down under this is a great start of a shelter. \n \nLean To: If no fallen trees are available or are not large enough finding a rock or pair of small/medium trees will work \n \n Fallen Tree: \n - Step 1: Add additional medium size branches to the body the tree to add structure. \n -Step 2: Add leaves and other small branches to help add wind and rain resistance. \n -Step 3: Add small and medium size branches so that the wind does not remove your leaves and other insulation. \n\n Lean To:  \n -Step 1: Find two trees about 5-8 feet apart. \n - Step 2: Locate a medium size long 2-5 inches in diameter that is long enough to reach both trees this will be the “backbone” of our shelter \n -Step 3: You will lash this log to the trees around 4’ off the ground. \n -Step 4: Lash the log to the trees by using the cord of the watch strap, unravel the cord then create a loop in the last 6-8 inches of the cord, lay the loop over the “backbone”and bring the ends of the rope around the log and through the loop to create a clove hitch. \n -Step 5: Wrap the ends of the cords together and wrap them around the front of the tree and pull the log up to the 4’ height. \n -Step 6: Wrap the lose end around the back of the backbone log and begin to wrap it around the tree spiraling upwards \n -Step 7: Once you have 2 - 3 loops wrap the end to the bottom of the log and make 2-3 more loops around the tree under the backbone. \n -Step 8: To Finish loop the cord around the log on either side of the tree.  End with a simple overhand knot.\n -Repeat steps 5-8 on the other side. \n Step 9: Add long thin branches by laying them at a 45 degree angle keep them closely spaced together to help keep out wind and rain. \n -Step 10: Pile up leaves, dirt, snow or other materials onto the branches to help create insulation. ";

static SmartstrapAttribute *s_raw_attribute;
static SmartstrapAttribute *s_attr_attribute;

static int isBeaconing = 0;
static int continueBeaconing = 0;

static AppSync s_sync;
static uint8_t s_sync_buffer[64];

static Window *s_main_window;
static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem s_first_menu_items[NUM_FIRST_MENU_ITEMS];

// Beacon Window variables
static Window *beacon_window;
static TextLayer *beacon_text_layer;
static TextLayer *beacon_text_layer_status;
static TextLayer *beacon_text_layer_lat;
static TextLayer *beacon_text_layer_lon;

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

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case 7:
      latitudeTextLayer = text_layer_create(GRect(0, 80, 144, 50));
      text_layer_set_text(latitudeTextLayer, new_tuple->value->cstring);
      break;
    case 12:
      longitudeTextLayer = text_layer_create(GRect(0, 100, 144, 50));
      text_layer_set_text(longitudeTextLayer, new_tuple->value->cstring);
      break;
  }
}

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
  uint32_t num = rand() % 10000;
  memcpy(buffer, &num, 4);

  // send it off
  result = smartstrap_attribute_end_write(s_attr_attribute, sizeof(num), true);
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
    app_timer_register(5000, prv_send_request, NULL);    
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

static void request_gps(void *context) {
  DictionaryIterator *iter;
  int errorValue = app_message_outbox_begin(&iter);

  if (!iter || errorValue != 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: %d", errorValue);
 
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR SENDING");
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
  app_timer_register(10000, request_gps, NULL);    
}

static void enableBeacon(void) {
  app_timer_register(1000, prv_send_request, NULL);
  continueBeaconing = 1;
}

static void disableBeacon(void) {
  continueBeaconing = 0;
}

void my_next_click_handler(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "CENTER BUTTON");
  Window *window = (Window *)context;
  text_layer_destroy(beacon_text_layer_status);

  beacon_text_layer_status = text_layer_create(GRect(0, 50, 144, 50));
  text_layer_set_background_color(beacon_text_layer_status, GColorClear);
  if (isBeaconing) {
  text_layer_set_text_color(beacon_text_layer_status, GColorRed);
    text_layer_set_text(beacon_text_layer_status, "OFF");
    isBeaconing = 0;
    disableBeacon();
  } else {
  text_layer_set_text_color(beacon_text_layer_status, GColorGreen);
    text_layer_set_text(beacon_text_layer_status, "ON");
    isBeaconing = 1;
    enableBeacon();
  }
  // Improve the layout to be more like a watchface
  text_layer_set_font(beacon_text_layer_status, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(beacon_text_layer_status, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer_status));
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) my_next_click_handler);
}

static void beacon_window_load(Window *window) {
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

  beacon_text_layer = text_layer_create(GRect(0, 10, 144, 50));
  text_layer_set_background_color(beacon_text_layer, GColorClear);
  text_layer_set_text_color(beacon_text_layer, GColorBlack);
  text_layer_set_text(beacon_text_layer, "BEACON IS");

  // Improve the layout to be more like a watchface
  text_layer_set_font(beacon_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(beacon_text_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer));
  
  beacon_text_layer_status = text_layer_create(GRect(0,50,144,50));
  text_layer_set_background_color(beacon_text_layer_status, GColorClear);
  if (isBeaconing) {
    text_layer_set_text_color(beacon_text_layer_status, GColorGreen);
    text_layer_set_text(beacon_text_layer_status, "ON");
  } else {
    text_layer_set_text_color(beacon_text_layer_status, GColorRed);
    text_layer_set_text(beacon_text_layer_status, "OFF");
  }
  text_layer_set_font(beacon_text_layer_status, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(beacon_text_layer_status, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer_status));

  beacon_text_layer_lat = text_layer_create(GRect(0,105,144,50));
  text_layer_set_background_color(beacon_text_layer_lat, GColorClear);
  text_layer_set_text_color(beacon_text_layer_lat, GColorBlack);
  text_layer_set_text(beacon_text_layer_lat, text_layer_get_text(latitudeTextLayer));
  text_layer_set_font(beacon_text_layer_lat, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(beacon_text_layer_lat, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer_lat));

  beacon_text_layer_lon = text_layer_create(GRect(0,120,144,50));
  text_layer_set_background_color(beacon_text_layer_lon, GColorClear);
  text_layer_set_text_color(beacon_text_layer_lon, GColorBlack);
  text_layer_set_text(beacon_text_layer_lon, text_layer_get_text(longitudeTextLayer));
  text_layer_set_font(beacon_text_layer_lon, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(beacon_text_layer_lon, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer_lon));

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
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_text_bounds = GRect(0, 66, bounds.size.w, 2000);

  s_scroll_layer = scroll_layer_create(bounds);

  // Create GBitmap, then set to created BitmapLayer
  static BitmapLayer *s_background_layer;
  static GBitmap *s_background_bitmap;
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FIRE);
  s_background_layer = bitmap_layer_create(GRect(15, 0, 111, 66));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);

  // Initialize the text layer
  s_scroll_text_layer = text_layer_create(max_text_bounds);
  text_layer_set_text(s_scroll_text_layer, s_scroll_text);

  // Change the font to a nice readable one
  // This is system font; you can inspect pebble_fonts.h for all system fonts
  // or you can take a look at feature_custom_font to add your own font
  text_layer_set_font(s_scroll_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(s_scroll_text_layer);
  text_layer_set_size(s_scroll_text_layer, max_size);
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, max_size.h + 70));

  // Add the layers for display
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_scroll_text_layer));  
  scroll_layer_add_child(s_scroll_layer, bitmap_layer_get_layer(s_background_layer));


  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
}

// SHELTER WINDOW
static void sg_shelter_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_text_bounds = GRect(0, 50, bounds.size.w, 2000);
  
  // Initialize the scroll layer
  s_shelter_scroll_layer = scroll_layer_create(bounds);

  // Create GBitmap, then set to created BitmapLayer
  static BitmapLayer *s_background_layer;
  static GBitmap *s_background_bitmap;
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FALLENTREE);
  s_background_layer = bitmap_layer_create(GRect(25, 0, 96, 50));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_shelter_scroll_layer, window);

  // Initialize the text layer
  s_shelter_text_layer = text_layer_create(max_text_bounds);
  text_layer_set_text(s_shelter_text_layer, s_shelter_scroll_text);

  // Change the font to a nice readable one
  // This is system font; you can inspect pebble_fonts.h for all system fonts
  // or you can take a look at feature_custom_font to add your own font
  text_layer_set_font(s_shelter_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(s_shelter_text_layer);
  text_layer_set_size(s_shelter_text_layer, max_size);
  scroll_layer_set_content_size(s_shelter_scroll_layer, GSize(bounds.size.w, max_size.h + 54));

  // Add the layers for display
  scroll_layer_add_child(s_shelter_scroll_layer, text_layer_get_layer(s_shelter_text_layer));  
  scroll_layer_add_child(s_shelter_scroll_layer, bitmap_layer_get_layer(s_background_layer));

  layer_add_child(window_layer, scroll_layer_get_layer(s_shelter_scroll_layer));
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
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  SmartstrapHandlers handlers = (SmartstrapHandlers) {
    .availability_did_change = prv_availablility_status_changed,
    .did_write = prv_did_write,
    .did_read = prv_did_read,
    .notified = prv_notified
  };
  smartstrap_subscribe(handlers);
  smartstrap_set_timeout(50);
  s_raw_attribute = smartstrap_attribute_create(0, 0, 2000);
  s_attr_attribute = smartstrap_attribute_create(0x1001, 0x1001, 20);
   app_message_open(64, 64);


  Tuplet initial_values[] = {
    TupletInteger(7, (uint8_t) 0),
    TupletInteger(12, (uint8_t) 0)
  }; 
   app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  request_gps((void*)1);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}