#include <Arduino.h>
#include <vector>
#include "driver.h"
#include <WiFi.h>

TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;
bool passMode = true;
static int foundNetworks = 0;
std::vector<String> foundWifiList;
String pass, ssid;

void lv_example_menu_5(void)
{
  menu_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(menu_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_width(menu_screen, 320);
  lv_obj_set_height(menu_screen, 240);
  lv_obj_set_style_border_width(menu_screen, 0, LV_PART_MAIN);

  menu = lv_menu_create(menu_screen);

  lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
  if (lv_color_brightness(bg_color) > 127)
  {
    lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 10), 0);
  }
  else
  {
    lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);
  }
  lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
  lv_obj_add_event_cb(menu, back_event_handler, LV_EVENT_CLICKED, menu);
  lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_center(menu);

  lv_obj_t *cont;
  lv_obj_t *section;

  /*Create sub pages*/
  lv_obj_t *sub_mechanics_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_mechanics_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_mechanics_page);
  section = lv_menu_section_create(sub_mechanics_page);
  create_slider(section, LV_SYMBOL_SETTINGS, "Velocity", 0, 150, 120);
  create_slider(section, LV_SYMBOL_SETTINGS, "Acceleration", 0, 150, 50);
  create_slider(section, LV_SYMBOL_SETTINGS, "Weight limit", 0, 150, 80);

  lv_obj_t *sub_sound_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_sound_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_sound_page);
  section = lv_menu_section_create(sub_sound_page);
  create_switch(section, LV_SYMBOL_LIST, "Minggu", false);
  create_switch(section, LV_SYMBOL_LIST, "Senin", false);
  create_switch(section, LV_SYMBOL_LIST, "Selasa", false);
  create_switch(section, LV_SYMBOL_LIST, "Rabu", false);
  create_switch(section, LV_SYMBOL_LIST, "Kamis", false);
  create_switch(section, LV_SYMBOL_LIST, "Jum'at", false);
  create_switch(section, LV_SYMBOL_LIST, "Sabtu", false);

  // WiFi List 
  lv_obj_t *sub_wifi_menu = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_wifi_menu, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_wifi_menu);
  section = lv_menu_section_create(sub_wifi_menu);
  cont = create_switch(section, LV_SYMBOL_WIFI, "WiFi", false);
  lv_obj_add_event_cb(lv_obj_get_child(cont, 2), switch_handler_wifi, LV_EVENT_ALL, NULL);
  listWiFisection = lv_obj_create(section);
  lv_obj_set_size(listWiFisection, lv_pct(100), 200);
  lv_obj_set_style_border_width(listWiFisection, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(listWiFisection, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(listWiFisection, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST | LV_OBJ_FLAG_SCROLLABLE);
  wfList = lv_list_create(listWiFisection);
  lv_obj_set_size(wfList, lv_pct(100), lv_pct(100));

  lv_obj_t *sub_display_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_display_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_display_page);
  section = lv_menu_section_create(sub_display_page);
  create_slider(section, LV_SYMBOL_SETTINGS, "Brightness", 0, 150, 100);

  lv_obj_t *sub_software_info_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_software_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  section = lv_menu_section_create(sub_software_info_page);
  create_text(section, NULL, "Version 1.0", LV_MENU_ITEM_BUILDER_VARIANT_1);

  lv_obj_t *sub_legal_info_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_legal_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  section = lv_menu_section_create(sub_legal_info_page);
  for (uint32_t i = 0; i < 15; i++)
  {
    create_text(section, NULL,
                "This is a long long long long long long long long long text, if it is long enough it may scroll.",
                LV_MENU_ITEM_BUILDER_VARIANT_1);
  }

  lv_obj_t *sub_about_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_about_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_about_page);
  section = lv_menu_section_create(sub_about_page);
  cont = create_text(section, NULL, "Software information", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_software_info_page);
  cont = create_text(section, NULL, "Legal information", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_legal_info_page);

  /*Create a root page*/
  root_page = lv_menu_page_create(menu, "Settings");
  lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  section = lv_menu_section_create(root_page);
  cont = create_text(section, LV_SYMBOL_SETTINGS, "Calibration", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_mechanics_page);
  cont = create_text(section, LV_SYMBOL_AUDIO, "Schedule", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_sound_page);
  // lv_menu_set_load_page_event(menu, cont, sub_display_page);
  cont = create_text(section, LV_SYMBOL_WIFI, "WiFi Setitng", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_wifi_menu);

  create_text(root_page, NULL, "Others", LV_MENU_ITEM_BUILDER_VARIANT_1);
  section = lv_menu_section_create(root_page);
  cont = create_text(section, NULL, "About", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_about_page);

  lv_menu_set_sidebar_page(menu, NULL);
  lv_menu_set_page(menu, root_page);

  // Container
  container = lv_obj_create(menu_screen);
  lv_obj_set_size(container, lv_pct(100), lv_pct(100));
  lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_center(container);
  lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
  mbox1 = lv_obj_create(container);
  lv_obj_set_size(mbox1, 200, 110);
  lv_obj_clear_flag(mbox1, LV_OBJ_FLAG_SCROLLABLE);
  txtArea = lv_textarea_create(mbox1);
  lv_obj_set_size(txtArea, lv_pct(100), 40);
  lv_textarea_set_one_line(txtArea, true);
  lv_textarea_set_password_mode(txtArea, passMode);
  lv_obj_center(mbox1);
  kb = lv_keyboard_create(container);
  lv_obj_add_event_cb(txtArea, ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *btn_cont = lv_obj_create(mbox1);
  lv_obj_clear_flag(btn_cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(btn_cont, lv_pct(100), 30);
  lv_obj_set_style_bg_opa(btn_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(btn_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(btn_cont, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_t *btn = lv_btn_create(btn_cont);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Apply");
  lv_obj_add_event_cb(btn, pass_input, LV_EVENT_ALL, label);
  lv_obj_set_size(btn, 50, 20);
  lv_obj_center(label);
  btn = lv_btn_create(btn_cont);
  label = lv_label_create(btn);
  lv_label_set_text(label, "Close");
  lv_obj_add_event_cb(btn, pass_input, LV_EVENT_ALL, label);
  lv_obj_set_size(btn, 50, 20);
  lv_obj_center(label);
  btn = lv_btn_create(btn_cont);
  label = lv_label_create(btn);
  lv_label_set_text(label, "Show");
  lv_obj_add_event_cb(btn, pass_input, LV_EVENT_ALL, label);
  lv_obj_set_size(btn, 50, 20);
  lv_obj_center(label);
  lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_flow(mbox1, LV_FLEX_FLOW_COLUMN);
}
void home(void)
{
  screen = lv_obj_create(NULL);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_width(screen, 320);
  lv_obj_set_height(screen, 240);
  lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);

  lv_obj_t *clock_label = lv_label_create(screen);
  lv_obj_set_size(clock_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_pos(clock_label, 22, 9 - 5);
  lv_label_set_text(clock_label, "20:21");
  lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

  LV_IMG_DECLARE(icon_wifi);
  lv_obj_t *img_wifi = lv_img_create(screen);
  lv_img_set_src(img_wifi, &icon_wifi);
  lv_obj_set_size(img_wifi, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_pos(img_wifi, 278, 6 - 5);
  lv_img_set_antialias(img_wifi, true);

  lv_obj_t *red = lv_obj_create(screen);
  lv_obj_set_size(red, 78, 78);
  lv_obj_set_pos(red, 10, 40 - 13);
  lv_obj_set_style_radius(red, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(red, lv_color_hex(0xDB0069), LV_PART_MAIN);
  lv_obj_set_style_border_width(red, 0, LV_PART_MAIN);

  LV_IMG_DECLARE(icon_info);
  lv_obj_t *img_info = lv_img_create(red);
  lv_img_set_src(img_info, &icon_info);
  lv_obj_set_size(img_info, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(img_info);
  lv_obj_clear_flag(img_info, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *black = lv_obj_create(screen);
  lv_obj_set_size(black, 78, 78);
  lv_obj_set_pos(black, 10, 123 - 13);
  lv_obj_set_style_radius(black, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(black, lv_color_hex(0x2ECE77), LV_PART_MAIN);
  lv_obj_set_style_border_width(black, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(black, setting_button, LV_EVENT_ALL, NULL);

  LV_IMG_DECLARE(icon_Tools_);
  lv_obj_t *img_tools = lv_img_create(black);
  lv_img_set_src(img_tools, &icon_Tools_);
  lv_obj_set_size(img_tools, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(img_tools);
  lv_obj_clear_flag(img_tools, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *bigBlue = lv_obj_create(screen);
  lv_obj_set_size(bigBlue, 218, 161);
  lv_obj_set_pos(bigBlue, 93, 40 - 13);
  lv_obj_set_style_radius(bigBlue, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(bigBlue, lv_color_hex(0x870093), LV_PART_MAIN);
  lv_obj_set_style_border_width(bigBlue, 0, LV_PART_MAIN);
  lv_obj_clear_flag(bigBlue, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(bigBlue, 0, LV_PART_MAIN);

  lv_obj_t *fan_switch = lv_obj_create(screen);
  lv_obj_set_size(fan_switch, 33, 33);
  lv_obj_set_pos(fan_switch, 93, 197);
  lv_obj_set_style_radius(fan_switch, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(fan_switch, lv_color_hex(0xDB0069), LV_PART_MAIN);
  lv_obj_set_style_border_width(fan_switch, 0, LV_PART_MAIN);
  lv_obj_clear_flag(fan_switch, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(fan_switch, 0, LV_PART_MAIN);
  LV_IMG_DECLARE(icon_fan);
  lv_obj_t *img_fan = lv_img_create(fan_switch);
  lv_img_set_src(img_fan, &icon_fan);
  lv_obj_set_size(img_fan, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(img_fan);
  lv_obj_clear_flag(img_fan, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *water_switch = lv_obj_create(screen);
  lv_obj_set_size(water_switch, 33, 33);
  lv_obj_set_pos(water_switch, 130, 197);
  lv_obj_set_style_radius(water_switch, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(water_switch, lv_color_hex(0xDB0069), LV_PART_MAIN);
  lv_obj_set_style_border_width(water_switch, 0, LV_PART_MAIN);
  lv_obj_clear_flag(water_switch, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(water_switch, 0, LV_PART_MAIN);
  LV_IMG_DECLARE(icon_water);
  lv_obj_t *img_water = lv_img_create(water_switch);
  lv_img_set_src(img_water, &icon_water);
  lv_obj_set_size(img_water, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(img_water);
  lv_obj_clear_flag(img_water, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *icon_mix = lv_obj_create(screen);
  lv_obj_set_size(icon_mix, 33, 33);
  lv_obj_set_pos(icon_mix, 130, 197);
  lv_obj_set_style_radius(icon_mix, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(icon_mix, lv_color_hex(0xDB0069), LV_PART_MAIN);
  lv_obj_set_style_border_width(icon_mix, 0, LV_PART_MAIN);
  lv_obj_clear_flag(icon_mix, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(icon_mix, 0, LV_PART_MAIN);
  LV_IMG_DECLARE(icon_random);
  lv_obj_t *img_mix = lv_img_create(icon_mix);
  lv_img_set_src(img_mix, &icon_random);
  lv_obj_set_size(img_mix, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_center(img_mix);
  lv_obj_clear_flag(img_mix, LV_OBJ_FLAG_CLICKABLE);


  static lv_style_t style_bg;
  static lv_style_t style_indic;

  lv_style_init(&style_bg);
  lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_border_width(&style_bg, 2);
  lv_style_set_pad_all(&style_bg, 3); /*To make the indicator smaller*/
  lv_style_set_radius(&style_bg, 50);

  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_radius(&style_indic, 50);

  bar1 = lv_bar_create(bigBlue);
  lv_obj_add_style(bar1, &style_indic, LV_PART_INDICATOR);
  lv_obj_add_style(bar1, &style_bg, 0);
  lv_obj_set_size(bar1, 15, 100);
  lv_bar_set_range(bar1, -20, 40);
  lv_obj_set_pos(bar1, 16, 45);
  bar2 = lv_bar_create(bigBlue);
  lv_obj_add_style(bar2, &style_indic, LV_PART_INDICATOR);
  lv_obj_add_style(bar2, &style_bg, 0);
  lv_obj_set_size(bar2, 15, 100);
  lv_bar_set_range(bar2, -20, 40);
  lv_obj_set_pos(bar2, 38, 45);
  bar3 = lv_bar_create(bigBlue);
  lv_obj_add_style(bar3, &style_indic, LV_PART_INDICATOR);
  lv_obj_add_style(bar3, &style_bg, 0);
  lv_obj_set_size(bar3, 15, 100);
  lv_bar_set_range(bar3, -20, 40);
  lv_obj_set_pos(bar3, 60, 45);
  bar4 = lv_bar_create(bigBlue);
  lv_obj_add_style(bar4, &style_indic, LV_PART_INDICATOR);
  lv_obj_add_style(bar4, &style_bg, 0);
  lv_obj_set_size(bar4, 15, 100);
  lv_bar_set_range(bar4, -20, 40);
  lv_obj_set_pos(bar4, 82, 45);

  label1 = lv_label_create(bigBlue);
  lv_obj_set_style_text_font(label1, &lv_font_montserrat_10, 0);
  lv_obj_set_pos(label1, 110, 133);

  label2 = lv_label_create(bigBlue);
  lv_obj_set_style_text_font(label2, &lv_font_montserrat_10, 0);
  lv_obj_set_pos(label2, 145, 118);

  label3 = lv_label_create(bigBlue);
  lv_obj_set_style_text_font(label3, &lv_font_montserrat_10, 0);
  lv_obj_set_pos(label3, 130, 102);

  label4 = lv_label_create(bigBlue);
  lv_obj_set_style_text_font(label4, &lv_font_montserrat_10, 0);
  lv_obj_set_pos(label4, 140, 86);
}

void setup()
{
  Serial.begin(115200);
  setDriver();
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, &lv_font_montserrat_12);
  lv_disp_set_theme(dispp, theme);
  home();
  lv_example_menu_5();
  lv_disp_load_scr(screen);
}
void loop()
{
  lv_timer_handler();
  static uint32_t prev = millis();
  static char text[20];
  if (millis() - prev > 1000)
  {
    float suhu = random(100);
    float ph = random(14);
    float suhu_air = random(100);
    int tds = random(1000);
    lv_bar_set_value(bar1, suhu, LV_ANIM_ON);
    lv_bar_set_value(bar2, ph, LV_ANIM_ON);
    lv_bar_set_value(bar3, suhu_air, LV_ANIM_ON);
    lv_bar_set_value(bar4, tds, LV_ANIM_ON);

    sprintf(text, "SUHU AIR : %.2f\u00b0c", suhu_air);
    lv_label_set_text(label1, text);
    sprintf(text, "pH : %.2fpH", ph);
    lv_label_set_text(label2, text);
    sprintf(text, "SUHU : %.2f\u00b0c", suhu);
    lv_label_set_text(label3, text);
    sprintf(text, "TDS : %dppm", tds);
    lv_label_set_text(label4, text);
    prev = millis();
  }
  delay(5);
}

static void back_event_handler(lv_event_t *e)
{
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);

  if (lv_menu_back_btn_is_root(menu, obj))
  {
    // lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
    // lv_obj_center(mbox1);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, false);
  }
}
static void switch_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    if (lv_obj_has_state(obj, LV_STATE_CHECKED))
    {
      lv_menu_set_page(menu, NULL);
      lv_menu_set_sidebar_page(menu, root_page);
      lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
    }
    else
    {
      lv_menu_set_sidebar_page(menu, NULL);
      lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
      lv_menu_set_page(menu, root_page);
    }
  }
}
static void switch_handler_wifi(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    if (lv_obj_has_state(obj, LV_STATE_CHECKED))
    {
      lv_obj_clear_flag(listWiFisection, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST);
      if (ntScanTaskHandler == NULL)
      {
        networkStatus = NETWORK_SEARCHING;
        networkScanner();
        timer = lv_timer_create(timerForNetwork, 1000, wfList);
        lv_list_add_text(wfList, "WiFi: Looking for Networks...");
      }
    }
    else
    {
      lv_obj_add_flag(listWiFisection, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_ADV_HITTEST);
      if (ntScanTaskHandler != NULL)
      {
        networkStatus = NONE;
        foundNetworks = 0;
        vTaskDelete(ntScanTaskHandler);
        ntScanTaskHandler = NULL;
        lv_timer_del(timer);
        lv_obj_clean(wfList);
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.disconnect(true);
      }
    }
  }
}
static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt, lv_menu_builder_variant_t builder_variant)
{
  lv_obj_t *obj = lv_menu_cont_create(parent);

  lv_obj_t *img = NULL;
  lv_obj_t *label = NULL;

  if (icon)
  {
    img = lv_img_create(obj);
    lv_img_set_src(img, icon);
  }

  if (txt)
  {
    label = lv_label_create(obj);
    lv_label_set_text(label, txt);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 1);
  }

  if (builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt)
  {
    lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_swap(img, label);
  }

  return obj;
}
static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max, int32_t val)
{
  lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);

  lv_obj_t *slider = lv_slider_create(obj);
  lv_obj_set_flex_grow(slider, 1);
  lv_slider_set_range(slider, min, max);
  lv_slider_set_value(slider, val, LV_ANIM_OFF);

  if (icon == NULL)
  {
    lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
  }

  return obj;
}
static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk)
{
  lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

  lv_obj_t *sw = lv_switch_create(obj);
  lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

  return obj;
}

static void timerForNetwork(lv_timer_t *timer)
{
  LV_UNUSED(timer);

  switch (networkStatus)
  {

  case NETWORK_SEARCHING:
    showingFoundWiFiList();
    break;

  case NETWORK_CONNECTED_POPUP:
    // popupMsgBox("WiFi Connected!", "Now you'll get the current time soon.");
    msg = lv_msgbox_create(NULL, "Connected to the WiFi", "Yay now you connect to the network", NULL, true);
    lv_obj_center(msg);
    networkStatus = NETWORK_CONNECTED;
    // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    break;

  case NETWORK_CONNECTED:

    showingFoundWiFiList();
    // updateLocalTime();
    break;

  case NETWORK_CONNECT_FAILED:
    networkStatus = NETWORK_SEARCHING;
    // popupMsgBox("Oops!", "Please check your wifi password and try again.");
    msg = lv_msgbox_create(NULL, "Connect Failed", "Oops can't connect to the network please check Password", NULL, true);
    lv_obj_center(msg);
    break;

  default:
    break;
  }
}
static void networkScanner()
{
  xTaskCreate(scanWIFITask,
              "ScanWIFITask",
              4096,
              NULL,
              1,
              &ntScanTaskHandler);
}
static void networkConnector()
{
  xTaskCreate(beginWIFITask,
              "beginWIFITask",
              2048,
              NULL,
              1,
              &ntConnectTaskHandler);
}
static void showingFoundWiFiList()
{
  if (foundWifiList.size() == 0 || foundNetworks == foundWifiList.size())
    return;

  lv_obj_clean(wfList);
  lv_list_add_text(wfList, foundWifiList.size() > 0 ? "WiFi: Found Networks" : "WiFi: Not Found!");

  for (std::vector<String>::iterator item = foundWifiList.begin(); item != foundWifiList.end(); ++item)
  {
    lv_obj_t *btn = lv_list_add_btn(wfList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, list_event_handler, LV_EVENT_CLICKED, NULL);
    Serial.println((*item).c_str());
    delay(1);
  }

  foundNetworks = foundWifiList.size();
}
static void scanWIFITask(void *pvParameters)
{
  while (1)
  {
    foundWifiList.clear();
    int n = WiFi.scanNetworks();
    vTaskDelay(10);
    for (int i = 0; i < n; ++i)
    {
      String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      foundWifiList.push_back(item);
      vTaskDelay(10);
    }
    vTaskDelay(5000);
  }
}
static void beginWIFITask(void *pvParameters)
{

  unsigned long startingTime = millis();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(100);
  Serial.printf("Connect to SSID : %s, PASS : %s", ssid, pass);
  WiFi.begin(ssid.c_str(), pass.c_str());
  while (WiFi.status() != WL_CONNECTED && (millis() - startingTime) < (10 * 1000))
  {
    vTaskDelay(250);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    networkStatus = NETWORK_CONNECTED_POPUP;
    // saveWIFICredentialEEPROM(1, ssidName + " " + ssidPW);
  }
  else
  {
    networkStatus = NETWORK_CONNECT_FAILED;
    // saveWIFICredentialEEPROM(0, "");
  }

  vTaskDelete(NULL);
}
static void list_event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED)
  {
    String selectedItem = String(lv_list_get_btn_text(wfList, obj));
    // for (int i = 0; i < selectedItem.length() - 1; i++)
    // {
    //   if ((selectedItem.substring(i, i + 3) == ") *"))
    //   {
    //     // ssidName = selectedItem.substring(0, i);
    //     // lv_label_set_text_fmt(mboxTitle, "Selected WiFi SSID: %s", ssidName);
    //     // lv_obj_move_foreground(mboxConnect);
    //     ssid = selectedItem.substring(0, selectedItem.indexOf("(-") - 1);
    //     Serial.printf("SSID : %s\n", ssid);
    //     break;
    //   }
    // }
    if (selectedItem.indexOf(") *") != -1)
    {
      lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
    }
    else if (selectedItem.indexOf(")  ") != -1)
    {
      ssid = selectedItem.substring(0, selectedItem.indexOf("(-") - 1);
      pass = "";
      networkConnector();
    }
  }
}
static void ta_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kbData = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED)
  {
    lv_keyboard_set_textarea(kbData, ta);
    lv_obj_clear_flag(kbData, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(mbox1, LV_ALIGN_TOP_MID, 0, 10);
  }

  if (code == LV_EVENT_DEFOCUSED)
  {
    lv_keyboard_set_textarea(kbData, NULL);
    lv_obj_add_flag(kbData, LV_OBJ_FLAG_HIDDEN);
    lv_obj_center(mbox1);
  }
}
static void pass_input(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);

  if (code == LV_EVENT_CLICKED)
  {
    Serial.println(lv_label_get_text(label));
    // lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
    if (strcmp(lv_label_get_text(label), "Show") == 0)
    {
      passMode = !passMode;
      lv_textarea_set_password_mode(txtArea, passMode);
    }
    if (strcmp(lv_label_get_text(label), "Close") == 0)
    {
      passMode = true;
      lv_textarea_set_password_mode(txtArea, passMode);
      lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
      lv_textarea_set_text(txtArea, "");
    }
    if (strcmp(lv_label_get_text(label), "Apply") == 0)
    {
      passMode = true;
      lv_textarea_set_password_mode(txtArea, passMode);
      lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
      pass = String(lv_textarea_get_text(txtArea));
      lv_textarea_set_text(txtArea, "");
      networkConnector();
    }
  }
}
static void setting_button(lv_event_t *e)
{
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED)
  {
    // _ui_screen_change(ui_Seting, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    lv_scr_load_anim(menu_screen, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0, false);
  }
}