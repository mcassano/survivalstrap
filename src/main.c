#include <pebble.h>
#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_SG_MENU_ITEMS 2

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


static TextLayer *sg_text_layer;
static char sg_fire_text[] = "Make a Fire, Dummy";
static char sg_shelter_text[] = "Build a Shelter, Dummy";

//BEACON WINDOW
static void beacon_window_load(Window *window) {
//  s_menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_1);
  window_set_background_color(window, GColorYellow);

  beacon_text_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(beacon_text_layer, GColorYellow);
  text_layer_set_text_color(beacon_text_layer, GColorBlack);
  text_layer_set_text(beacon_text_layer, "SOS");
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(beacon_text_layer));
}

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

  sg_text_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_text_color(beacon_text_layer, GColorWhite);
  text_layer_set_text(sg_text_layer, "Make a Fire, dummy.");
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(sg_text_layer));
}

// SHELTER WINDOW
static void sg_shelter_window_load(Window *window) {
//  s_menu_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_1);

  sg_text_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(beacon_text_layer, GColorRed);
  text_layer_set_text_color(beacon_text_layer, GColorWhite);
  text_layer_set_text(sg_text_layer, "Build a shelter, dummy.");
  
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

  //APP_LOG(APP_LOG_LEVEL_DEBUG, sizeof(sg_menu_items)/sizeof(sg_menu_items[0]));
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  sg_menu_layer = simple_menu_layer_create(bounds, window, sg_menu_section, NUM_MENU_SECTIONS, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(sg_menu_layer));
}

void sg_menu_window_unload(Window *window) {
  simple_menu_layer_destroy(s_simple_menu_layer);
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
  gbitmap_destroy(s_menu_icon_image);
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