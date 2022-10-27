#include <lvgl.h>
#include <TFT_eSPI.h>
// #include "sensi_logo_small.c"

// following https://github.com/lvgl/lvgl/blob/master/examples/arduino/LVGL_Arduino/LVGL_Arduino.ino

#define N_PX_W 135 // width
#define N_PX_H 240 // height

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ N_PX_W * 10]; // this can be smaller, idk the minimal number for it to work

static lv_obj_t * screen = NULL;

TFT_eSPI tft =
    TFT_eSPI(N_PX_W, N_PX_H); // Invoke library, pins defined in User_Setup.h

static lv_style_t * app_style = NULL;

static float value = 0;

struct sensor_widget{
    lv_obj_t * widget;
    lv_obj_t * value_label;
};

static sensor_widget working_widget;

LV_IMG_DECLARE(sensi_logo_small); // required if online converter was used

sensor_widget create_widget(const char* quantity, char* value, const char* unit, const char* device_name, lv_obj_t * parent, lv_color_t highlight_color = lv_color_white()) {
    lv_obj_t * widg = lv_obj_create(parent);
    
    lv_obj_set_size(widg, 220,110);
    lv_obj_set_align(widg, LV_ALIGN_CENTER);
    lv_obj_add_style(widg, app_style, 0);
    lv_obj_set_style_border_color(widg, highlight_color, 0);

    lv_obj_t * value_label = lv_label_create(widg);
    lv_label_set_text(value_label, value);
    lv_obj_set_align(value_label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(value_label, highlight_color, 0);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    // maybe use the states to change the colors? (red, orange, green)

    lv_obj_t * quantity_label = lv_label_create(widg);
    lv_label_set_text(quantity_label, quantity);
    lv_obj_align_to(quantity_label, value_label, LV_ALIGN_OUT_LEFT_MID, -15, 4);


    lv_obj_t * unit_label = lv_label_create(widg);
    lv_label_set_text(unit_label, unit);
    lv_obj_align_to(unit_label, value_label, LV_ALIGN_OUT_RIGHT_MID, 10 , 4);

    lv_obj_t * device_name_label = lv_label_create(widg);
    lv_label_set_text(device_name_label, device_name);
    lv_obj_set_align(device_name_label, LV_ALIGN_BOTTOM_MID);    
    
    return {widg, value_label};
}

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

    screen = lv_scr_act();

    // Create dark style
    static lv_style_t dark_style;

    lv_style_init(&dark_style);
    lv_style_set_bg_color(&dark_style, lv_color_black());
    lv_style_set_text_color(&dark_style, lv_color_white());
    
    // apply dark style
    app_style = &dark_style;

    lv_obj_add_style(screen, app_style, LV_STATE_DEFAULT);

    lv_obj_t * logo = lv_img_create(screen);
    lv_img_set_src(logo, &sensi_logo_small);
    lv_obj_set_align(logo, LV_ALIGN_CENTER);
    lv_timer_handler();
    delay(2000);
    lv_obj_add_flag(logo, LV_OBJ_FLAG_HIDDEN);
    // lv_disp_t
    // Create dark sensi theme

    // lv_theme_t * th = lv_theme_default_init(,  /*Use the DPI, size, etc from this display*/ 
    //                                         LV_COLOR_PALETTE_BLUE, LV_COLOR_PALETTE_CYAN,   /*Primary and secondary palette*/
    //                                         false,    /*Light or dark mode*/ 
    //                                         &lv_font_montserrat_10, &lv_font_montserrat_14, &lv_font_montserrat_18); /*Small, normal, large fonts*/
                                            
    // lv_disp_set_theme(display, th); /*Assign the theme to the display*/
    // lv_theme_apply(screen);

     /*lv_obj_create(screen);
    lv_obj_set_size(widg, 220,110);
    lv_obj_set_align(widg, LV_ALIGN_CENTER);
    lv_obj_add_style(widg, app_style, 0);
    lv_obj_set_style_border_color(widg, highlight_color, 0);

    lv_obj_t * value_label = lv_label_create(widg);
    lv_label_set_text(value_label, buffer);
    lv_obj_set_align(value_label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(value_label, highlight_color, 0);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    // maybe use the states to change the colors? (red, orange, green)

    lv_obj_t * quantity_label = lv_label_create(widg);
    lv_label_set_text(quantity_label, quantity);
    lv_obj_align_to(quantity_label, value_label, LV_ALIGN_OUT_LEFT_MID, -15, 4);


    lv_obj_t * unit_label = lv_label_create(widg);
    lv_label_set_text(unit_label, unit);
    lv_obj_align_to(unit_label, value_label, LV_ALIGN_OUT_RIGHT_MID, 10 , 4);

    lv_obj_t * device_name_label = lv_label_create(widg);
    lv_label_set_text(device_name_label, device_name);
    lv_obj_set_align(device_name_label, LV_ALIGN_BOTTOM_MID);*/
}

void loop() {
    // lv_label_set_text(value_label, buffer);

    char* quantity = "T:";
    char* unit = "degC";
    char* device_name = "gadgetID";

    char buffer[6];
    int ret = snprintf(buffer, sizeof buffer, "%01f", value);    
    // Build a sensor reading widget
    lv_obj_clean(screen); // implicit deletion of dynamically allocated widget
    // lv_color_t highlight_color = lv_color_hex(0xFF0000); // red
    lv_color_t highlight_color = lv_color_hex(0x00FF00); // green
    lv_obj_t * widg = create_widget(quantity, buffer, unit, device_name, screen, highlight_color);
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
    value += 1;
}
