#include <Arduino.h>
#define LGFX_USE_V1
#include <lvgl.h>
#include <LovyanGFX.hpp>
// #define LV_USE_LOG 0
#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void)
  {
    {                                    // バス制御の設定を行います。
      auto cfg = _bus_instance.config(); // バス設定用の構造体を取得します。

      // SPIバスの設定
      // cfg.spi_host = VSPI_HOST;     // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 0;          // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 50000000; // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read = 16000000;  // 受信時のSPIクロック
      cfg.spi_3wire = false;     // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock = true;       // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = 1;       // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = 18;                      // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 23;                      // SPIのMOSIピン番号を設定
      cfg.pin_miso = 19;                      // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc = 16;                        // SPIのD/Cピン番号を設定  (-1 = disable)
      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }

    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs = 5;    // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst = 17;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy = -1; // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.panel_width = 240;    // 実際に表示可能な幅
      cfg.panel_height = 320;   // 実際に表示可能な高さ
      cfg.offset_x = 0;         // パネルのX方向オフセット量
      cfg.offset_y = 0;         // パネルのY方向オフセット量
      cfg.offset_rotation = 0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel = 8; // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits = 1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable = true;      // データ読出しが可能な場合 trueに設定
      cfg.invert = false;       // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order = false;    // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit = false;   // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared = true;    // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
      //    cfg.memory_width     =   240;  // ドライバICがサポートしている最大の幅
      //    cfg.memory_height    =   320;  // ドライバICがサポートしている最大の高さ

      _panel_instance.config(cfg);
    }

    //*
    {                                      // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config(); // バックライト設定用の構造体を取得します。

      cfg.pin_bl = 21;     // バックライトが接続されているピン番号
      cfg.invert = false;  // バックライトの輝度を反転させる場合 true
      cfg.freq = 44100;    // バックライトのPWM周波数
      cfg.pwm_channel = 7; // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // バックライトをパネルにセットします。
    }
    //*/

    //*
    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;           // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = 319;         // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;           // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = 479;         // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = -1;        // INTが接続されているピン番号
      cfg.bus_shared = true;   // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

      // SPI接続の場合
      cfg.spi_host = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 1000000;       // SPIクロックを設定
      cfg.pin_sclk = 18;        // SCLKが接続されているピン番号
      cfg.pin_mosi = 23;        // MOSIが接続されているピン番号
      cfg.pin_miso = 19;        // MISOが接続されているピン番号
      cfg.pin_cs = 22;          //   CSが接続されているピン番号
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }
    //*/

    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

/*Change to your screen resolution*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  // tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

static void setDriver(void)
{
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin();        /* TFT init */
  tft.setRotation(3); /* Landscape orientation, flipped */
  tft.setBrightness(255);
  // uint16_t calData[] = {3799, 463, 3775, 3855, 242, 403, 226, 3907}; // 480 x 320
  uint16_t calData[] = {3792, 393, 3851, 3883, 261, 403, 224, 3897}; // 320 x 240
  tft.setTouchCalibrate(calData);
  // uint16_t caldata[8];
  // tft.calibrateTouch(caldata, TFT_BLACK, TFT_WHITE, 8);
  // for (int i = 0; i < 8; i++)
  // {
  //   Serial.print(caldata[i]);
  //   Serial.print(",");
  // }

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  Serial.println("Setup done");
}

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

static lv_obj_t *menu_screen;
static lv_obj_t *menu;
static lv_obj_t *screen;
static lv_obj_t *bar1;
static lv_obj_t *bar2;
static lv_obj_t *bar3;
static lv_obj_t *bar4;
static lv_obj_t *label1;
static lv_obj_t *label2;
static lv_obj_t *label3;
static lv_obj_t *label4;
static lv_timer_t *timer;
static lv_obj_t *wfList;
static lv_obj_t *root_page;
static lv_obj_t *listWiFisection;
static lv_obj_t *txtArea;
static lv_obj_t *kb;
static lv_obj_t *container;
static lv_obj_t *mbox1;
static lv_obj_t *msg;

static void ta_event_cb(lv_event_t *e);
static void setting_button(lv_event_t *e);
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
