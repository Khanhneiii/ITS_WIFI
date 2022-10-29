#include <FastLED.h>

#define LED_PIN     13
#define NUM_LEDS    16
#define BRIGHTNESS  64
#define LED_TYPE    WS2812
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup_fastled() {
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void Config_led(String &hexled,int brightness){
    char *hex = const_cast<char*>(hexled.c_str());
    uint32_t color = strtoul(hex, nullptr, HEX);
    for( int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = color;  
      FastLED.setBrightness( brightness );
    }
         
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}
