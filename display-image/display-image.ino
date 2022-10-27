#include <lvgl.h>
#include <TFT_eSPI.h>
// DDN'T include the genereated .c image file

#define N_PX_W 135 // width
#define N_PX_H 240 // height

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ N_PX_W * 10]; // this can be smaller, idk the minimal number for it to work

TFT_eSPI tft =
    TFT_eSPI(N_PX_W, N_PX_H); // Invoke library, pins defined in User_Setup.h

// image_small.c was converted from image_small.jpg using the lvgl online converter
// https://lvgl.io/tools/imageconverter

LV_IMG_DECLARE(image_small); // this seems to load image_small.c

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

    lv_obj_t * screen = lv_scr_act();

    lv_obj_t * image = lv_img_create(screen);
    lv_img_set_src(image, &image_small);
    lv_obj_set_align(image, LV_ALIGN_CENTER);
    
}

void loop() {
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}

