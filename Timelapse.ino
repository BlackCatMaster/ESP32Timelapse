#include <SD.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <ESPmDNS.h>

//#define CAMERA_MODEL_XIAO_ESP32S3         // 使用するボードのコメントを外します
#define CAMERA_MODEL_UNIT_CAMS3           // 使用するボードのコメントを外します
#include "camera_pins.h"

#define WIFI_SSID   "********"            // WiFi SSID
#define WIFI_PASS   "********"            // WiFiパスワード
#define HOST_NAME   "timelapse"           // ホスト名

void startCameraServer();

time_t last_take = 0;                     // 前回の撮影時刻(ms)
int interval = 1;                         // 撮影間隔(s)
int capture = 0;                          // 撮影制御フラグ (0:停止 1:開始)

// 写真を撮ってSDカードに保存する
void takeAndSavePhotos() {
  if (capture) {
    time_t now_ms = millis();             // 起動からの時刻を取得(ms)
    if (now_ms - last_take > interval * 1000) {
      last_take = now_ms;
      time_t now = time(nullptr);         // 現在時刻を取得(s)
      char filename[32];
      sprintf(filename, "/%d.jpg", now);  // エポックタイムでファイル名生成
      Serial.println(filename);
      File file = SD.open(filename, FILE_WRITE);
      camera_fb_t *fb = esp_camera_fb_get(); // 写真撮影
      file.write(fb->buf, fb->len);       // 写真データ書き込み
      file.close();
      esp_camera_fb_return(fb);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // SDカードスロットの初期化
  SPI.begin(SD_PIN_CLK, SD_PIN_MISO, SD_PIN_MOSI, SD_PIN_CS);
  SD.begin(SD_PIN_CS);
  if (SD.cardType() == CARD_NONE) {
    Serial.println("No SD card attached");
    while(1);
  }

  // カメラ初期化
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;
  esp_camera_init(&config);

  // WiFi接続
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_POWER_15dBm);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (++i > 5) ESP.restart();
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  MDNS.begin(HOST_NAME);                  // mDNS初期化

  // NTPで時刻取得
  configTzTime("JST-9", "ntp.nict.jp");
  time_t now = time(nullptr);
  while (now < 10000) {
    delay(500);
    now = time(nullptr);
  }

  startCameraServer();                    // カメラサーバー開始

  Serial.println("Camera Ready! Use 'http://" HOST_NAME ".local' to connect");
}

void loop() {
  takeAndSavePhotos();                    // 写真を撮ってSDカードに保存する 
  delay(10);
}