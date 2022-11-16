#include "Button2.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <ArduinoBleSensiScan.h>
#include "orientation.h"
// #include "sensi_logo_small.c"

// following https://github.com/lvgl/lvgl/blob/master/examples/arduino/LVGL_Arduino/LVGL_Arduino.ino

#define N_PX_W 135 // width
#define N_PX_H 240 // height

// Buttons to control displayed values
#define BUTTON_1 0
#define BUTTON_2 35

Button2 btn_right(BUTTON_1);
Button2 btn_left(BUTTON_2);

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
    sensor_widget(): widget(nullptr), value_label(nullptr) {};
};

static sensor_widget current_widget;
static sensor_widget temp_widget;
static lv_anim_t slide_out_anim;
static lv_anim_t slide_in_anim;

// static char* quantity = "T:";
// static char* unit = "degC";
// static char* device_name = "gadgetID";

SensiScan sensiScan = SensiScan();
static int updateIntervalMs = 500;
static int64_t lastUpdateTimeMs = 0;

std::map<Gadget, std::vector<Sample>> knownGadgets;
std::string selectedGadgetId = "";
UnitType selectedUnit = UnitType::CO2;

const std::string unitTypeSymbols[] = {"UNDEFINED", "degC",  "%",     "",
                                       "",          "ppm",   "μg/m3", "ppb",
                                       "μg/m3",     "μg/m3", "μg/m3"};


LV_IMG_DECLARE(sensi_logo_small); // required if online converter was used

void create_widget(sensor_widget& sensor_widget, const char* quantity, char* value, const char* unit, const char* device_name, lv_obj_t* parent, lv_color_t highlight_color = lv_color_white()) {
    // Serial.println("creating widget");
    lv_obj_t* widg = lv_obj_create(parent);
    sensor_widget.widget = widg;
    lv_obj_set_size(widg, 220,110);
    lv_obj_set_align(widg, LV_ALIGN_CENTER);
    lv_obj_add_style(widg, app_style, 0);
    lv_obj_set_style_border_color(widg, highlight_color, 0);
    lv_obj_t* value_label = lv_label_create(widg);
    sensor_widget.value_label = value_label;
    lv_label_set_text(sensor_widget.value_label, value);
    lv_obj_set_align(sensor_widget.value_label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(sensor_widget.value_label, highlight_color, 0);
    lv_obj_set_style_text_font(sensor_widget.value_label, &lv_font_montserrat_30, LV_STATE_DEFAULT);
    // maybe use the states to change the colors? (red, orange, green)

    lv_obj_t * quantity_label = lv_label_create(widg);
    lv_label_set_text(quantity_label, quantity);
    lv_obj_align_to(quantity_label, sensor_widget.value_label, LV_ALIGN_OUT_LEFT_MID, -15, 4);

    lv_obj_t * unit_label = lv_label_create(widg);
    lv_label_set_text(unit_label, unit);
    lv_obj_align_to(unit_label, sensor_widget.value_label, LV_ALIGN_OUT_RIGHT_MID, 10 , 4);

    lv_obj_t * device_name_label = lv_label_create(widg);
    lv_label_set_text(device_name_label, device_name);
    lv_obj_set_align(device_name_label, LV_ALIGN_BOTTOM_MID);    
}

// This assumes that the widget is centered w.r.t. to the parent
void init_slide_anim(const sensor_widget& sensor_widget, lv_anim_t * anim, Direction direction, Slide slide) {

    int out_of_sight_distance_horizontal = (lv_obj_get_width(sensor_widget.widget) 
        + lv_obj_get_width(sensor_widget.widget->parent))/2;
    int out_of_sight_distance_vertical = (lv_obj_get_height(sensor_widget.widget) 
        + lv_obj_get_height(sensor_widget.widget->parent))/2;
    // Serial.println((int)anim);
    lv_anim_init(anim);
    lv_anim_set_var(anim, sensor_widget.widget);
    lv_anim_set_time(anim, 500); // in ms

    int out_of_sight_position;

    switch (direction)
    {
    case up:
        lv_anim_set_exec_cb(anim, (lv_anim_exec_xcb_t) lv_obj_set_y);
        out_of_sight_position = - (lv_obj_get_height(sensor_widget.widget) 
            + lv_obj_get_height(sensor_widget.widget->parent))/2;
        break;
    case left:
        lv_anim_set_exec_cb(anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
        out_of_sight_position = - (lv_obj_get_width(sensor_widget.widget) 
            + lv_obj_get_width(sensor_widget.widget->parent))/2;
        break;
    case down:
        lv_anim_set_exec_cb(anim, (lv_anim_exec_xcb_t) lv_obj_set_y);
        out_of_sight_position = (lv_obj_get_height(sensor_widget.widget) 
            + lv_obj_get_height(sensor_widget.widget->parent))/2;
        break;
    case right:
        lv_anim_set_exec_cb(anim, (lv_anim_exec_xcb_t) lv_obj_set_x);
        out_of_sight_position = (lv_obj_get_width(sensor_widget.widget) 
            + lv_obj_get_width(sensor_widget.widget->parent))/2;
        break;

    default:
        break; // I don't like this
    }

    switch (slide) {
        case in:
            lv_anim_set_values(anim, out_of_sight_position, 0);
            break;
        case out:
            lv_anim_set_values(anim, 0, out_of_sight_position);
            break;
        default:
            break; // Don't like this either
    }

   // need to call lv_anim_start(anim) afterwards 
}

void lv_obj_del_anim_ready_cb_custom(lv_anim_t * a)
{
    lv_obj_del_anim_ready_cb(a); // deletes widget and value_label objects
    // reset pointers
    temp_widget.widget = nullptr;
    temp_widget.value_label = nullptr;
}

void switch_widget_horizontal(sensor_widget new_widget) {
    temp_widget = current_widget;
    current_widget = new_widget;
    init_slide_anim(temp_widget, &slide_out_anim, right, out);
    init_slide_anim(current_widget, &slide_in_anim, left, in);
    lv_anim_set_ready_cb(&slide_out_anim, lv_obj_del_anim_ready_cb_custom);
    lv_anim_start(&slide_out_anim);
    lv_anim_start(&slide_in_anim);
}

void switch_widget_vertical(sensor_widget new_widget) {
    temp_widget = current_widget;
    current_widget = new_widget;
    init_slide_anim(temp_widget, &slide_out_anim, down, out);
    init_slide_anim(current_widget, &slide_in_anim, up, in);
    lv_anim_set_ready_cb(&slide_out_anim, lv_obj_del_anim_ready_cb_custom);
    lv_anim_start(&slide_out_anim);
    lv_anim_start(&slide_in_anim);
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

void setupButtons() {
  btn_left.setPressedHandler([](Button2 &b) {
    Serial.printf("Left button pressed\n");
    // value = 0;
    // char value_buffer[6];
    // snprintf(value_buffer, sizeof value_buffer, "%01f", value);    
    // create_widget(temp_widget, quantity, value_buffer, unit, device_name, screen); //, highlight_color);
    // switch_widget_vertical(temp_widget);
    rotateSelectedUnit();
    selectAndDisplaySample();
  });

  btn_right.setPressedHandler([](Button2 &b) {
    Serial.printf("Right button pressed\n");
    // value = 0;
    // char value_buffer[6];
    // snprintf(value_buffer, sizeof value_buffer, "%01f", value);    
    // create_widget(temp_widget, quantity, value_buffer, unit, device_name, screen); //, highlight_color);
    // switch_widget_vertical(temp_widget);
    rotateSelectedGadget();
    selectAndDisplaySample();
  });
}

std::vector<Sample>::const_iterator
findSampleByUnit(const std::vector<Sample> &samples, UnitType unit) {
  auto sampleIt = samples.begin();
  while (sampleIt != samples.end()) {
    if (sampleIt->type == unit) {
      return sampleIt;
    }
    ++sampleIt;
  }
  return sampleIt;
}

std::map<Gadget, std::vector<Sample>>::const_iterator
findGadgetById(const std::map<Gadget, std::vector<Sample>> &knownGadgets,
               std::string gadgetId) {
  auto gadgetIt = knownGadgets.begin();
  while (gadgetIt != knownGadgets.end()) {
    if (gadgetIt->first.deviceId == gadgetId) {
      return gadgetIt;
    }
    ++gadgetIt;
  }
  return gadgetIt;
}

void UpdateScanResults() {
  // fetch gadgets found by sensiscan library
  std::map<Gadget, std::vector<Sample>> newGadgets;
  sensiScan.getScanResults(newGadgets);
//   if (knownGadgets.size() == 0) {
//     selectedGadgetId = newGadgets.begin()->first.deviceId;
//     selectedUnit = newGadgets.begin()->second[0].type;
//   }
  for (const auto &gadget : newGadgets) {
    if (findGadgetById(knownGadgets, gadget.first.deviceId) ==
        knownGadgets.end()) {
      // add new Gadget
      knownGadgets.insert(gadget);
    } else {
      // update existing Gadet
      knownGadgets[gadget.first] = gadget.second;
    }
  }
  Serial.print("Number of known Gadgets: ");
  Serial.println(knownGadgets.size());
}

void selectAndDisplaySample() {
  if (knownGadgets.empty()) {
    // nothing to display
    // displaySplashScreen();
    Serial.println("no gadgets");
    return;
  }

  // find selected gadget in known gadgets
  auto currentGadgetIt = findGadgetById(knownGadgets, selectedGadgetId);
  // latest selected gadget not available
  if (currentGadgetIt == knownGadgets.end()) {
    currentGadgetIt = knownGadgets.begin();
  }

  // find selected sample in gadget's samples
  std::vector<Sample> currentSamples = currentGadgetIt->second;
  auto currentSampleIt = findSampleByUnit(currentSamples, selectedUnit);
  // latest selected sample not available
  if (currentSampleIt == currentSamples.end()) {
    currentSampleIt = currentSamples.begin();
  }

  Serial.printf("Current sample of type %s from %s => %f\n",
                unitTypeString[currentSampleIt->type].c_str(),
                currentGadgetIt->first.deviceId.c_str(),
                currentSampleIt->value);
  Serial.print("selectedUnit: ");
  Serial.println(selectedUnit);
  Serial.print("gadgetId: ");
  Serial.println(selectedGadgetId.c_str());
  Serial.print("currentUnit: ");
  Serial.println(currentSampleIt->type);
  Serial.print("currentGadgetId: ");
  Serial.println(currentGadgetIt->first.deviceId.c_str());
    // if the selected gadgetId and type are the same, just update the value
    // otherwise create a new widget with the new values and swtich the widget
  char value[6];
  snprintf(value, sizeof value, "%01f", currentSampleIt->value);
  const char* type = unitTypeString[currentSampleIt->type].c_str();

  if (selectedGadgetId == currentGadgetIt->first.deviceId.c_str() && 
    selectedUnit == currentSampleIt->type) {
        Serial.println("a");
        lv_label_set_text(current_widget.value_label, value);
    }
  else if (selectedGadgetId == currentGadgetIt->first.deviceId.c_str()) {
    selectedUnit = currentSampleIt->type;
    Serial.println("b");
    create_widget(temp_widget, type, value, unitTypeSymbols[selectedUnit].c_str(), selectedGadgetId.c_str(), screen);
    switch_widget_horizontal(temp_widget);
  }
  else {
    selectedGadgetId = currentGadgetIt->first.deviceId;
    selectedUnit = currentSampleIt->type;
    Serial.println("c");
    create_widget(temp_widget, type, value, unitTypeSymbols[selectedUnit].c_str(), selectedGadgetId.c_str(), screen);
    switch_widget_vertical(temp_widget);
  }
}
//   display(currentSampleIt->value, currentSampleIt->type,
//           currentGadgetIt->first.deviceId.c_str());

void rotateSelectedUnit() {
    if (knownGadgets.size() == 0) {
    return;
  } 
  std::vector<Sample> samples =
      findGadgetById(knownGadgets, selectedGadgetId)->second;
  if (samples.size() < 2) {
    return;
  }
  auto sampleIt = findSampleByUnit(samples, selectedUnit);
  if (sampleIt == samples.end()) {
    // selected sample not found, choose first
    selectedUnit = samples.begin()->type;
  } else {
    // select next sample
    ++sampleIt;
    if (sampleIt == samples.end()) {
      // cyclic rotation
      sampleIt = samples.begin();
    }
    selectedUnit = sampleIt->type;
    char value[6];
    snprintf(value, sizeof value, "%01f", sampleIt->value);
    const char* type = unitTypeString[selectedUnit].c_str();
    create_widget(temp_widget, type, value, unitTypeSymbols[selectedUnit].c_str(), selectedGadgetId.c_str(), screen);
    switch_widget_horizontal(temp_widget);
  }
}

void rotateSelectedGadget() {
  if (knownGadgets.size() < 2) {
    return;
  }
  auto gadgetIt = findGadgetById(knownGadgets, selectedGadgetId);
  if (gadgetIt == knownGadgets.end()) {
    // selected gadget not found, choose first
    selectedGadgetId = knownGadgets.begin()->first.deviceId;
  } else {
    // select next gadget
    ++gadgetIt;
    if (gadgetIt == knownGadgets.end()) {
      // cyclic rotation
      gadgetIt = knownGadgets.begin();
    }
    selectedGadgetId = gadgetIt->first.deviceId;
    selectedUnit = gadgetIt->second[0].type;
    char value[6];
    snprintf(value, sizeof value, "%01f", gadgetIt->second[0].value);
    const char* type = unitTypeString[selectedUnit].c_str();
    create_widget(temp_widget, type, value, unitTypeSymbols[selectedUnit].c_str(), selectedGadgetId.c_str(), screen);
    switch_widget_vertical(temp_widget);
  }
}

void buttonLoop() {
  btn_left.loop();
  btn_right.loop();
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    setupButtons();
    lv_init();
    sensiScan.begin();
    tft.begin();
    tft.setRotation(1); // on change, hor_res and ver_res of disp_drv (below) might need to be switched
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, N_PX_W * 10 );
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );

    // Change the following two lines to your display resolution
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
    delay(1000);
    lv_obj_add_flag(logo, LV_OBJ_FLAG_HIDDEN);
    lv_timer_handler();
    Serial.println((int)screen);

    // Show initial widget
    // value = 0.0;
    // char value_buffer[6];
    // int ret = snprintf(value_buffer, sizeof value_buffer, "%01f", value);
    // char* none = "--";
    // create_widget(current_widget, none, "no data", none, none, screen);
    while(knownGadgets.size() < 1) {
        UpdateScanResults();
        delay(updateIntervalMs / 4);
    }
    selectedGadgetId = knownGadgets.begin()->first.deviceId;
    selectedUnit = knownGadgets.begin()->second[0].type;
    char value[6];
    snprintf(value, sizeof value, "%01f", knownGadgets.begin()->second[0].value);
    const char* type = unitTypeString[selectedUnit].c_str();
    create_widget(current_widget, type, value, unitTypeSymbols[selectedUnit].c_str(), selectedGadgetId.c_str(), screen);
    lv_timer_handler();

    // selectAndDisplaySample();
    // lv_timer_handler();
}

void loop() {
    buttonLoop();

    lv_color_t highlight_color = lv_color_hex(0x00FF00); // green
    if (millis() - lastUpdateTimeMs >= updateIntervalMs) {
        Serial.println("update");
        UpdateScanResults();
        // selectAndDisplaySample();
        lastUpdateTimeMs = millis();
    }
    selectAndDisplaySample();

    // value += 1;
    // char value_buffer[6];
    // snprintf(value_buffer, sizeof value_buffer, "%01f", value);    
    // lv_label_set_text(current_widget.value_label, value_buffer);
    // if (value >= 10 && temp_widget.widget == nullptr) {
    //     value = 0;
    //     create_widget(temp_widget, quantity, value_buffer, unit, device_name, screen, highlight_color);
    //     switch_widget_vertical(temp_widget);
    // }

    // Build a sensor reading widget
    // lv_obj_clean(screen); // implicit deletion of dynamically allocated widget
    // lv_color_t highlight_color = lv_color_hex(0xFF0000); // red
    lv_timer_handler(); /* let the GUI do its work */
    delay(10);
}
