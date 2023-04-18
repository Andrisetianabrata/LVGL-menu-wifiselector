#include <Arduino.h>
#include <vector>
#include "driver.h"
#include <WiFi.h>

TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;
bool passMode = true;
typedef enum
{
  NONE,
  NETWORK_SEARCHING,
  NETWORK_CONNECTED_POPUP,
  NETWORK_CONNECTED,
  NETWORK_CONNECT_FAILED
} Network_Status_t;
Network_Status_t networkStatus = NONE;
enum
{
  LV_MENU_ITEM_BUILDER_VARIANT_1,
  LV_MENU_ITEM_BUILDER_VARIANT_2
};
typedef uint8_t lv_menu_builder_variant_t;

static int foundNetworks = 0;
std::vector<String> foundWifiList;
String pass, ssid;

static lv_timer_t *timer;
static lv_obj_t *wfList;
static lv_obj_t *root_page;
static lv_obj_t *listWiFisection;
static lv_obj_t *txtArea;
static lv_obj_t *kb;
static lv_obj_t *container;
static lv_obj_t *mbox1;
static lv_obj_t* msg;

static void ta_event_cb(lv_event_t *e);
static void switch_handler_wifi(lv_event_t *e);
static void list_event_handler(lv_event_t *e);
static void back_event_handler(lv_event_t *e);
static void switch_handler(lv_event_t *e);
static void pass_input(lv_event_t *e);
static void showingFoundWiFiList();
static void networkScanner();
static void timerForNetwork(lv_timer_t *timer);
static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt, lv_menu_builder_variant_t builder_variant);
static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk);
static void scanWIFITask(void *pvParameters);
static void beginWIFITask(void *pvParameters);

void lv_example_menu_5(void)
{
  lv_obj_t *menu = lv_menu_create(lv_scr_act());

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
  create_switch(section, LV_SYMBOL_AUDIO, "Sound", false);

  lv_obj_t *sub_wifi_menu = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_wifi_menu, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_wifi_menu);
  section = lv_menu_section_create(sub_wifi_menu);
  cont = create_switch(section, LV_SYMBOL_WIFI, "WiFi", false);
  lv_obj_add_event_cb(lv_obj_get_child(cont, 2), switch_handler_wifi, LV_EVENT_ALL, NULL);
  // cont = create_text(section, NULL, "WiFi List", LV_MENU_ITEM_BUILDER_VARIANT_1);
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

  lv_obj_t *sub_menu_mode_page = lv_menu_page_create(menu, NULL);
  lv_obj_set_style_pad_hor(sub_menu_mode_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  lv_menu_separator_create(sub_menu_mode_page);
  section = lv_menu_section_create(sub_menu_mode_page);
  cont = create_switch(section, LV_SYMBOL_AUDIO, "Sidebar enable", true);
  lv_obj_add_event_cb(lv_obj_get_child(cont, 2), switch_handler, LV_EVENT_VALUE_CHANGED, menu);

  /*Create a root page*/
  root_page = lv_menu_page_create(menu, "Settings");
  lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
  section = lv_menu_section_create(root_page);
  cont = create_text(section, LV_SYMBOL_SETTINGS, "Mechanics", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_mechanics_page);
  cont = create_text(section, LV_SYMBOL_AUDIO, "Sound", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_sound_page);
  cont = create_text(section, LV_SYMBOL_SETTINGS, "Display", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_display_page);
  cont = create_text(section, LV_SYMBOL_WIFI, "WiFi Setitng", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_wifi_menu);

  create_text(root_page, NULL, "Others", LV_MENU_ITEM_BUILDER_VARIANT_1);
  section = lv_menu_section_create(root_page);
  cont = create_text(section, NULL, "About", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_about_page);
  cont = create_text(section, LV_SYMBOL_SETTINGS, "Menu mode", LV_MENU_ITEM_BUILDER_VARIANT_1);
  lv_menu_set_load_page_event(menu, cont, sub_menu_mode_page);

  lv_menu_set_sidebar_page(menu, root_page);

  lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);

  // Container
  container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(container, 480, 320);
  lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_center(container);
  lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
  mbox1 = lv_obj_create(container);
  lv_obj_set_size(mbox1, 300, 110);
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
  lv_label_set_text(label, "Show Pass");
  lv_obj_add_event_cb(btn, pass_input, LV_EVENT_ALL, label);
  lv_obj_set_size(btn, 100, 20);
  lv_obj_center(label);
  lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_flow(mbox1, LV_FLEX_FLOW_COLUMN);
}
static void back_event_handler(lv_event_t *e)
{
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);

  if (lv_menu_back_btn_is_root(menu, obj))
  {
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
    lv_obj_center(mbox1);
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

void setup()
{
  Serial.begin(115200);
  setDriver();
  lv_example_menu_5();
}
void loop()
{
  lv_timer_handler();
  delay(5);
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
    for (int i = 0; i < selectedItem.length() - 1; i++)
    {
      if ((selectedItem.substring(i, i + 3) == ") *"))
      {
        // ssidName = selectedItem.substring(0, i);
        // lv_label_set_text_fmt(mboxTitle, "Selected WiFi SSID: %s", ssidName);
        // lv_obj_move_foreground(mboxConnect);
        ssid = selectedItem.substring(0, selectedItem.indexOf("(-") - 1);
        Serial.printf("SSID : %s\n", ssid);
        break;
      }
    }
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
    if (strcmp(lv_label_get_text(label), "Show Pass") == 0)
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