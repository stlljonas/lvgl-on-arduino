#include <lvgl.h>
//#include <lv_examples.h>
//#include <lv_demos.h>
#include <demos/lv_demos.h>
#include <TFT_eSPI.h>

// following https://github.com/lvgl/lvgl/blob/master/examples/arduino/LVGL_Arduino/LVGL_Arduino.ino

#define N_PX_W 135 // width
#define N_PX_H 240 // height

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ N_PX_W * 10]; // this can be smaller, idk the minimal number for it to work
static lv_obj_t * tab_view;

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

    lv_disp_flush_ready( disp ); // ??
}

void setup() {
    lv_init(); // seems fine
    tft.begin(); // yup
    tft.setRotation(1); // on change, hor_res and ver_res of disp_drv might need to be switched
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, N_PX_W * 10 );
    
    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = N_PX_H;
    disp_drv.ver_res = N_PX_W;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );
    // lv_demo_widgets();

    // trying to follow the demo_widgets example
    // lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_coord_t tab_h = 20; // h = height??
    tab_view = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);
    lv_obj_t * t1 = lv_tabview_add_tab(tab_view, "Profile");
    lv_obj_t * t2 = lv_tabview_add_tab(tab_view, "Analytics");
    lv_obj_t * t3 = lv_tabview_add_tab(tab_view, "Shop");
    // lv_style_init(&style_text_muted);
    // lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

}
void loop() { // loop looks good
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}
