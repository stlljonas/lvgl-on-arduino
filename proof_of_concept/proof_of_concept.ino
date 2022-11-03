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

static lv_anim_t hor_swipe;

struct sensor_widget{
    lv_obj_t * widget;
    lv_obj_t * value_label;
};

static sensor_widget working_widget_1;
static sensor_widget working_widget_2;
static lv_anim_t slide_out_horizontal_anim;
static lv_anim_t slide_in_horizontal_anim;

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

sensor_widget create_and_slide_in(const char* quantity, char* value, const char* unit, const char* device_name, lv_obj_t * parent, lv_color_t highlight_color = lv_color_white()) {
    // Create the object
    sensor_widget new_widget = create_widget(quantity, value, unit, device_name, parent, highlight_color);
    // Position it outside of screen
    int widget_width = lv_obj_get_width(new_widget.widget);
    int parent_width = lv_obj_get_width(new_widget.widget->parent);
    // lv_obj_set_pos(new_widget.widget, -widget_width, 0);

    // Move to center
    
    lv_anim_init(&slide_in_horizontal_anim);
    lv_anim_set_exec_cb(&slide_in_horizontal_anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
    lv_anim_set_var(&slide_in_horizontal_anim, new_widget.widget);
    lv_anim_set_time(&slide_in_horizontal_anim, 500); // in ms
    lv_anim_set_values(&slide_in_horizontal_anim, -((widget_width + parent_width)/2), 0);
    lv_anim_start(&slide_in_horizontal_anim);
    return new_widget;
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

void slide_out_and_delete(lv_obj_t * obj) {
    // lv_anim_init(&slide_out_horizontal_anim);
    // lv_anim_set_exec_cb(&slide_out_horizontal_anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
    // lv_anim_set_var(&slide_out_horizontal_anim, working_widget.widget);
    // lv_anim_set_time(&slide_out_horizontal_anim, 1000); // in ms
    // lv_anim_set_values(&slide_out_horizontal_anim, 0, 100);
    // lv_anim_start(&hor_swipe);

    int widget_width = lv_obj_get_width(obj);
    int parent_width = lv_obj_get_width(obj->parent);

    lv_anim_init(&slide_out_horizontal_anim);
    lv_anim_set_exec_cb(&slide_out_horizontal_anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
    lv_anim_set_var(&slide_out_horizontal_anim, obj);
    lv_anim_set_time(&slide_out_horizontal_anim, 500); // in ms
    
    lv_anim_set_values(&slide_out_horizontal_anim, 0, ((widget_width + parent_width)/2));
    // void read_cb = [](obj) {lv_obj_del(obj)}
    // lv_anim_set_ready_cb(&slide_out_horizontal_anim, lv_obj_del(obj));
    // lv_anim_exec_xcb_t
    // lv_obj_del_async(obj);
    // lv_obj_del_anim_ready_cb(&slide_out_horizontal_anim);
    lv_anim_set_ready_cb(&slide_out_horizontal_anim, lv_obj_del_anim_ready_cb);
    // lv_anim_set_ready_cb(&slide_out_horizontal_anim, lv_anim_del);
    lv_anim_start(&slide_out_horizontal_anim);
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
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

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
}

void loop() {
    char* quantity = "T:";
    char* unit = "degC";
    char* device_name = "gadgetID";

    lv_color_t highlight_color = lv_color_hex(0x00FF00); // green

    // lv_label_set_text(value_label, buffer);
    if (working_widget_1.widget == NULL) {
        value = 0.0;
        char buffer[6];
        int ret = snprintf(buffer, sizeof buffer, "%01f", value);
        working_widget_1 = create_and_slide_in(quantity, buffer, unit, device_name, screen, highlight_color);
        // lv_anim_init(&slide_out_horizontal_anim);
        // lv_anim_set_exec_cb(&slide_out_horizontal_anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
        // lv_anim_set_var(&slide_out_horizontal_anim, working_widget.widget);
        // lv_anim_set_time(&slide_out_horizontal_anim, 1000); // in ms
        // lv_anim_set_values(&slide_out_horizontal_anim, 0, 100);
        // lv_anim_start(&hor_swipe);

        // lv_anim_init(&hor_swipe);
        // lv_anim_set_exec_cb(&hor_swipe, (lv_anim_exec_xcb_t) lv_obj_set_x);
        // lv_anim_set_var(&hor_swipe, working_widget.widget);
        // lv_anim_set_time(&hor_swipe, 10000); // in ms
        // lv_anim_set_values(&hor_swipe, 0, 100);
        // lv_anim_start(&hor_swipe);
    } else {
        value += 1;
        char buffer[6];

        int ret = snprintf(buffer, sizeof buffer, "%01f", value);    
        lv_label_set_text(working_widget_1.value_label, buffer);
        if (value == 100) {
            slide_out_and_delete(working_widget_1.widget);
            create_and_slide_in(quantity, buffer, unit, device_name, screen, highlight_color);
            // we need a method that replaces the current widget.
            // most notabl, it needs to create a new widget (widget_2), slide out the old widget (widget_1), 
            // slide in the new widget, delete widget_1 and set widget_1 to the new widget (widget_2 should now be 0).
            // this is actually useful as we can use th
        }
    }
    // Build a sensor reading widget
    // lv_obj_clean(screen); // implicit deletion of dynamically allocated widget
    // lv_color_t highlight_color = lv_color_hex(0xFF0000); // red
    lv_timer_handler(); /* let the GUI do its work */
    delay(50);
}
