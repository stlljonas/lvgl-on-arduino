#include <lvgl.h>
#include <TFT_eSPI.h>

// following https://github.com/lvgl/lvgl/blob/master/examples/arduino/LVGL_Arduino/LVGL_Arduino.ino

#define N_PX_W 135 // width
#define N_PX_H 240 // height

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ N_PX_W * 10]; // this can be smaller, idk the minimal number for it to work
static lv_obj_t * tab_view;

TFT_eSPI tft =
    TFT_eSPI(N_PX_W, N_PX_H); // Invoke library, pins defined in User_Setup.h

unsigned long tab_switch_time_ms = 1000;
unsigned long last_tab_switch_timestamp = 0;
int tab_idx = 0;

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ) {
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );
    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();
    lv_disp_flush_ready( disp );
}

void setup() {
    lv_init();
    tft.begin();
    tft.setRotation(1); // on change, hor_res and ver_res of disp_drv might need to be switched
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, N_PX_W * 10 );

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    // Change the following line to your display resolution
    disp_drv.hor_res = N_PX_H;
    disp_drv.ver_res = N_PX_W;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    lv_coord_t tab_height = N_PX_W/6; // h = height !

    // Create tabview
    tab_view = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_height);
    lv_obj_t * t1 = lv_tabview_add_tab(tab_view, "Tab1");
    lv_obj_t * t2 = lv_tabview_add_tab(tab_view, "Tab2");
    lv_obj_t * t3 = lv_tabview_add_tab(tab_view, "Tab3");

    // Add content to tab in from of label
    lv_obj_t * content1 = lv_label_create(t1);
    lv_obj_t * content2 = lv_label_create(t2);
    lv_obj_t * content3 = lv_label_create(t3);
    lv_label_set_text(content1, "Tab1 content");
    lv_label_set_text(content2, "Tab2 content");
    lv_label_set_text(content3, "Tab3 content");
}
void loop() {
    if (millis() - last_tab_switch_timestamp >= tab_switch_time_ms) {
        tab_idx = (tab_idx+1)%3;
        lv_tabview_set_act(tab_view, tab_idx, LV_ANIM_ON);
        last_tab_switch_timestamp = millis();
    }
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}
