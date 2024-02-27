#if defined(CAMERA_MODEL_XIAO_ESP32S3)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39
#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

#define SD_PIN_CS         21
#define SD_PIN_MOSI       9
#define SD_PIN_CLK        7
#define SD_PIN_MISO       8

#elif defined(CAMERA_MODEL_UNIT_CAMS3)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     11
#define SIOD_GPIO_NUM     17
#define SIOC_GPIO_NUM     41
#define Y9_GPIO_NUM       13
#define Y8_GPIO_NUM       4
#define Y7_GPIO_NUM       10
#define Y6_GPIO_NUM       5
#define Y5_GPIO_NUM       7
#define Y4_GPIO_NUM       16
#define Y3_GPIO_NUM       15
#define Y2_GPIO_NUM       6
#define VSYNC_GPIO_NUM    42
#define HREF_GPIO_NUM     18
#define PCLK_GPIO_NUM     12

#define LED_BUILTIN       14
#define SD_PIN_CS         9
#define SD_PIN_MOSI       38
#define SD_PIN_CLK        39
#define SD_PIN_MISO       40

#else
#error "Camera model not selected"
#endif