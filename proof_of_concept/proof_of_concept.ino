#include <lvgl.h>
#include <TFT_eSPI.h>

// following https://github.com/lvgl/lvgl/blob/master/examples/arduino/LVGL_Arduino/LVGL_Arduino.ino

#define N_PX_W 135 // width
#define N_PX_H 240 // height

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ N_PX_W * 10]; // this can be smaller, the minimal number for it to work is unknown

TFT_eSPI tft =
    TFT_eSPI(N_PX_W, N_PX_H); // Invoke library, pins defined in User_Setup.h

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ) {
    // uint32_t w = ( area->x2 - area->x1 + 1 ); // this is incorrect
    // uint32_t h = ( area->y2 - area->y1 + 1 ); // this also
    uint32_t w = ( area->x2 - area->x1 + 1 ); // these seem right after inspecting
    uint32_t h = ( area->y2 - area->y1 + 1 ); // setAddrWindow()
    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true ); // whats this color_p->full?
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

void setup() {
    lv_init();
    tft.begin();
    tft.setRotation(1); // on change, hor_res and ver_res of disp_drv might need to be switched
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, N_PX_W * 10 );

    /* Initialize the display */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /* Change the following lines to your display resolution */
    disp_drv.hor_res = N_PX_H;
    disp_drv.ver_res = N_PX_W;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );
    
    // display some text
    lv_obj_t * text = lv_label_create(lv_scr_act());
    lv_label_set_text(text, "Hello World");
}

void loop() { // loop looks good
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}
