 
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LEDPIN 2

// Why so serious?

static lv_disp_draw_buf_t draw_buf;    //定义显示器变量
static lv_color_t buf[TFT_WIDTH * 10]; //定义刷新缓存
 
TFT_eSPI tft = TFT_eSPI(240,240);

static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 240;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, LEDPIN, NEO_GRB + NEO_KHZ800);

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
 
    tft.startWrite();                                        //使能写功能
    tft.setAddrWindow(area->x1, area->y1, w, h);             //设置填充区域
    tft.pushColors((uint16_t *)&color_p->full, w * h, true); //写入颜色缓存和缓存大小
    tft.endWrite();                                          //关闭写功能
 
    lv_disp_flush_ready(disp); //调用区域填充颜色函数
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// void ScreenShow( void *pvParameters)
// {
  
//     vTaskDelay(1);   /* code */    
// }

void LED (void *pvParameters)
{
    while (1)
    {
          // Some example procedures showing how to display to the pixels:
        colorWipe(strip.Color(255, 0, 0), 50); // Red
        colorWipe(strip.Color(0, 255, 0), 50); // Green
        colorWipe(strip.Color(0, 0, 255), 50); // Blue
        //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
        // Send a theater pixel chase in...
        theaterChase(strip.Color(127, 127, 127), 50); // White
        theaterChase(strip.Color(127, 0, 0), 50); // Red
        theaterChase(strip.Color(0, 0, 127), 50); // Blue

        rainbow(20);
        rainbowCycle(20);
        theaterChaseRainbow(50);
        vTaskDelay(10);
    }
    
}

void ScanKey(void *pvParameters)
{

}

void ScanEncoder(void *pvParameters)
{

}

void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    lv_init();

    // #if LV_USE_LOG != 0
    // lv_log_register_print_cb( my_print ); /* register print function for debugging */
    // #endif

    tft.begin();          /* TFT init */
    tft.setRotation( 4 ); /* Landscape orientation, flipped */

    /*Set the touchscreen calibration data,
     the actual data for your display can be acquired using
     the Generic -> Touch_calibrate example from the TFT_eSPI library*/
    // uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
    // tft.setTouch( calData );

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    // static lv_indev_drv_t indev_drv;
    // lv_indev_drv_init( &indev_drv );
    // indev_drv.type = LV_INDEV_TYPE_POINTER;
    // indev_drv.read_cb = my_touchpad_read;
    // lv_indev_drv_register( &indev_drv );

    #if 0
    /* Create simple label */
    lv_obj_t *label = lv_label_create( lv_scr_act() );
    lv_label_set_text( label, LVGL_Arduino.c_str() );
    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
    #else
    /* Try an example from the lv_examples Arduino library
       make sure to include it as written above.
    lv_example_btn_1();
   */

    // uncomment one of these demos
    // lv_demo_benchmark();          // OK
    // lv_demo_keypad_encoder();     // works, but I haven't an encoder
    // lv_demo_music();              // NOK
    // lv_demo_printer();
    // lv_demo_stress();             // seems to be OK
    ui_init();
    #endif

      // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
    #if defined (__AVR_ATtiny85__)
        if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
    #endif
    // End of trinket special code

    strip.begin();
    strip.setBrightness(20);
    strip.show(); // Initialize all pixels to 'off'

    // FastLED.setMaxPowerInVoltsAndMilliamps( VOLTS, MAX_MA);
    // FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // chooseNextColorPalette(gTargetPalette);
    // xTaskCreatePinnedToCore(ScreenShow,"ScreenShow",1024,NULL,1024,NULL,0);
    xTaskCreatePinnedToCore(LED,"LED",4096,NULL,1,NULL,0);
    // xTaskCreatePinnedToCore(ScanKey,"ScanKey",2048,NULL,1,NULL,1);
    // xTaskCreatePinnedToCore(ScanEncoder,"ScanEncoder",2048,NULL,1,NULL,1);
}
 
void loop()
{    
    lv_timer_handler(); /* let the GUI do its work */ 
    delay(5);
}