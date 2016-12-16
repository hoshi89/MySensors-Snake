#include <FastLED.h>
#include "ESP8266WiFi.h"
#include "snake.h"

#define MATRIX_WIDTH  16
#define MATRIX_HEIGHT 9
#define NUM_LEDS      (MATRIX_WIDTH * MATRIX_HEIGHT)

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define LED_PIN  5

#define COLOR_ORDER GRB
#define CHIPSET     WS2811

#define BRIGHTNESS 64
#define MAX_FPS    8

// Enable and select radio type attached
#define MY_RF24_CHANNEL 42
#define MY_RADIO_NRF24

// Set LOW transmit power level as default, if you have an amplified NRF-module and
// power your radio separately with a good regulator you can turn up PA level.
#define MY_RF24_PA_LEVEL RF24_PA_LOW

// Enable serial gateway
#define MY_GATEWAY_SERIAL

// Define a lower baud rate for Arduino's running on 8 MHz (Arduino Pro Mini 3.3V & SenseBender)
#if F_CPU == 8000000L
#define MY_BAUD_RATE 38400
#endif
#include <MySensors.h>

#define UP_PIN      2
#define LEFT_PIN    3
#define RIGHT_PIN   4
#define DOWN_PIN    5

#define NUM_BUTTONS 6
bool button_state[NUM_BUTTONS];

// Params for width and height
const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 9;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;

CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);

snake the_snake(3, vector2d(4, 4), vector2d(MATRIX_WIDTH, MATRIX_HEIGHT));

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}

void receive(const MyMessage &message)
{
  button_state[message.sensor] = message.getBool();
}

void setup()
{
  // Turn off Wifi
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );
}

void loop()
{
  uint32_t start_frame = millis();

  if (button_state[LEFT_PIN]) {
    the_snake.changeDirection(Left);
  }
  if (button_state[UP_PIN]) {
    the_snake.changeDirection(Up);
  }
  if (button_state[DOWN_PIN]) {
    the_snake.changeDirection(Down);
  }
  if (button_state[RIGHT_PIN]) {
    the_snake.changeDirection(Right);
  }
  
  FastLED.clear();

  leds[ XY(the_snake.getFood().x, the_snake.getFood().y)] = CRGB(255, 0, 0);

  snake_part *s = the_snake.getHead();
  leds[ XY(s->pos.x, s->pos.y)] = CRGB(255, 255, 0);
  s = s->next;
  
  while(s != nullptr) {
    leds[ XY(s->pos.x, s->pos.y)] = CRGB(0, 255, 0);
    s = s->next;
  }
  
  the_snake.update();

  FastLED.show();
  delay((1000 / MAX_FPS) - (millis() - start_frame));
}
