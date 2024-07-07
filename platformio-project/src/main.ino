#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

const char* ssid = "ssid";
const char* password = "password";
const char* Gemini_Token = "your api key";
const char* Gemini_Max_Tokens = "100";
String res = "";

#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

int wait_time = 0;
String resp_text = "";

bool change_action = 0;

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

/// DEFINE GLOBAL OBJECTS
lv_obj_t * scr;
lv_obj_t * output_scr;
lv_obj_t * output_tb;
///

int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
}

void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void toggle_keyboard(lv_event_t * e){
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * tb = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);

  if(code == LV_EVENT_FOCUSED) {
    lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(kb,tb);
  }

  else if(code == LV_EVENT_DEFOCUSED){
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(kb, NULL);
  }
}

static void answer_request(lv_event_t * e){
  lv_obj_t * kb = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t * tb = (lv_obj_t *)lv_event_get_user_data(e);

  lv_obj_add_flag(scr, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(output_scr, LV_OBJ_FLAG_HIDDEN);

  String qstn_text = lv_textarea_get_text(tb);

  send_get(qstn_text);
  delay(1000);

  lv_textarea_set_text(output_tb, resp_text.c_str());
}

static void go_back_trigger(lv_event_t * e){
  lv_obj_add_flag(output_scr, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_HIDDEN);
}

void main_gui(void){
  scr = lv_obj_create(lv_scr_act());
  lv_obj_set_size(scr, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_align(scr, LV_ALIGN_CENTER, 0, 0);

  output_scr = lv_obj_create(lv_scr_act());
  lv_obj_set_size(output_scr, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_align(output_scr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(output_scr, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t * kb = lv_keyboard_create(scr);
  lv_obj_t * tb = lv_textarea_create(scr);
  lv_obj_set_size(tb, 220, 60);
  lv_obj_align(tb, LV_ALIGN_TOP_MID, 0, 0);
  lv_textarea_set_placeholder_text(tb, "Message Gemini");
  lv_obj_add_event_cb(tb, toggle_keyboard, LV_EVENT_ALL, kb);

  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_size(kb, 310, 120);
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 7);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(kb, answer_request, LV_EVENT_READY, tb);

  LV_IMAGE_DECLARE(logo);
  lv_obj_t * logo_img = lv_image_create(scr);
  lv_img_set_src(logo_img, &logo);
  lv_obj_align(logo_img, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_set_size(logo_img, 150, 60);

  output_tb = lv_textarea_create(output_scr);
  lv_obj_set_size(output_tb, SCREEN_WIDTH, 200);
  lv_obj_align(output_tb, LV_ALIGN_TOP_MID, 0, -10);

  lv_obj_t * btn_back = lv_button_create(output_scr);
  lv_obj_set_size(btn_back, SCREEN_WIDTH, 40);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0 , 0);
  lv_obj_add_event_cb(btn_back, go_back_trigger, LV_EVENT_CLICKED, NULL);

  lv_obj_t * btn_back_text = lv_label_create(btn_back);
  lv_obj_align(btn_back_text, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(btn_back_text, "Go Back");
}

void setup() {
  /// GUI PART
  lv_init();

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(3);

  lv_display_t * disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
    
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  /// GEMINI PART
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1);
  }

  main_gui();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void send_get(String qinput){
  resp_text = "";
 
  res = qinput;
  int len = res.length();
  res = "\"" + res + "\"";

  HTTPClient https;

  if (https.begin("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + (String)Gemini_Token)) {  // HTTPS

    https.addHeader("Content-Type", "application/json");
    String payload = String("{\"contents\": [{\"parts\":[{\"text\":" + res + "}]}],\"generationConfig\": {\"maxOutputTokens\": " + (String)Gemini_Max_Tokens + "}}");

    int httpCode = https.POST(payload);

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();

      DynamicJsonDocument doc(1024);


      deserializeJson(doc, payload);
      String Answer = doc["candidates"][0]["content"]["parts"][0]["text"];

      Answer.trim();
      String filteredAnswer = "";
      for (size_t i = 0; i < Answer.length(); i++) {
        char c = Answer[i];
        if (isalnum(c) || isspace(c)) {
          filteredAnswer += c;
        } else {
          filteredAnswer += ' ';
        }
      }
      Answer = filteredAnswer;

      resp_text = Answer;
    } else {
      // error status
    }
    https.end();
  } else {
    //error status
  }
  res = "";
}

void loop(){
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
}