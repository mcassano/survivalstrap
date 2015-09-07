#include <ArduinoPebbleSerial.h>

static const uint16_t SUPPORTED_SERVICES[] = {0x0000, 0x1001};
static const uint8_t NUM_SERVICES = 2;
static uint8_t pebble_buffer[GET_PAYLOAD_BUFFER_SIZE(200)];

double lat, lon;

void setup() {
// General init
//2400 baud for the 434 model
Serial.begin(2400);
pinMode(LED_BUILTIN, OUTPUT);
pinMode(13, OUTPUT);
digitalWrite(LED_BUILTIN, LOW);

Serial.println("Starting");
// Teensy 2.0 uses the one-wire software serial mode (pin 2);
const uint8_t PEBBLE_PIN = 2;
ArduinoPebbleSerial::begin_software(PEBBLE_PIN, pebble_buffer, sizeof(pebble_buffer), Baud57600,
SUPPORTED_SERVICES, NUM_SERVICES);
}

void loop() {
// Let the ArduinoPebbleSerial code do its processing
size_t length;
uint16_t service_id;
uint16_t attribute_id;
RequestType type;
if (ArduinoPebbleSerial::feed(&service_id, &attribute_id, &length, &type)) {
if ((service_id == 0) && (attribute_id == 0)) {
// we have a raw data frame to process
static bool led_status = false;
led_status = !led_status;
digitalWrite(LED_BUILTIN, led_status);
if (type == RequestTypeRead) {
// send a response to the Pebble - reuse the same buffer for the response
uint32_t current_time = millis();
memcpy(pebble_buffer, &current_time, 4);
ArduinoPebbleSerial::write(true, pebble_buffer, 4);
Serial.println(pebble_buffer);
digitalWrite(3, HIGH);
delay(100);
digitalWrite(3, LOW);
} else if (type == RequestTypeWrite) {
//Serial.print("Got raw data write: ");
//Serial.println((uint8_t)pebble_buffer[0], DEC);
} else {
// invalid request type - just ignore the request
}
} else if ((service_id == 0x1001) && (attribute_id == 0x1001)) {
static uint32_t s_test_attr_data = 99999;
if (type == RequestTypeWriteRead) {
// read the previous value and write the new one
uint32_t old_value = s_test_attr_data;
memcpy(&s_test_attr_data, pebble_buffer, sizeof(s_test_attr_data));
ArduinoPebbleSerial::write(true, (const uint8_t *)&old_value, sizeof(old_value));
//Serial.println("Got WriteRead for 0x1001,0x1001");
} else {
// invalid request type - just ignore the request
}
} else {
// unsupported attribute - fail the request
ArduinoPebbleSerial::write(false, NULL, 0);
}
}

static bool is_connected = false;
if (ArduinoPebbleSerial::is_connected()) {
if (!is_connected) {
Serial.println("Connected to the smartstrap!");
is_connected = true;
}
static uint32_t last_notify = 0;
if (last_notify == 0) {
last_notify = millis();
}
// notify the pebble every 2.5 seconds
if (millis() - last_notify  > 2500) {
/*
Serial.println("Sending notification for 0x1001,0x1001");
ArduinoPebbleSerial::notify(0x1001, 0x1001);
*/
last_notify = millis();
}
} else {
if (is_connected) {
Serial.println("Disconnected from the smartstrap!");
is_connected = false;
}
}
}
