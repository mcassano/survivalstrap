#include <pebble.h>

ActionBarLayer *action_bar;
static Window *s_main_window;
static TextLayer *s_time_layer;

void my_next_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;
}

void my_previous_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) my_next_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) my_previous_click_handler);
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  // Initialize the action bar:
  action_bar = action_bar_layer_create();
  // Associate the action bar with the window:
  action_bar_layer_add_to_window(action_bar, window);
  // Set the click config provider:
  action_bar_layer_set_click_config_provider(action_bar,
                                             click_config_provider);

  // Set the icons:
  // The loading of the icons is omitted for brevity... See gbitmap_create_with_resource()
  //action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_UP, my_icon_previous, true);
  //action_bar_layer_set_icon_animated(action_bar, BUTTON_ID_DOWN, my_icon_next, true);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}