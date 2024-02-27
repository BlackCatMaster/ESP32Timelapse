#include <Arduino.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include "index_html.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;
extern int interval;
extern int capture;

// 撮影間隔毎に写真を送信（Timelapse開始で停止）
static esp_err_t stream_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  char *part_buf[128];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  time_t last_time = 0;
  while (!capture) {
    time_t now = millis();
    if (now - last_time > interval * 1000) {
      last_time = now;
      camera_fb_t *fb = esp_camera_fb_get(); 
      size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, fb->len);
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      if (res == ESP_OK)
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      if (res == ESP_OK)
        res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
      if (res != ESP_OK) break;
    }
  }
  return res;
}

// パラメータ取得処理
static esp_err_t parse_get(httpd_req_t *req, char **obuf) {
  char *buf = NULL;
  size_t buf_len = 0;

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char *)malloc(buf_len);
    if (!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      *obuf = buf;
      return ESP_OK;
    }
    free(buf);
  }
  httpd_resp_send_404(req);
  return ESP_FAIL;
}

// ボタンなどの操作が行われたときの処理
static esp_err_t cmd_handler(httpd_req_t *req) {
  char *buf = NULL;
  char variable[32];
  char value[32];

  if (parse_get(req, &buf) != ESP_OK) {
    return ESP_FAIL;
  }
  if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK ||
    httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
    free(buf);
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }
  free(buf);

  int val = atoi(value);
  sensor_t *s = esp_camera_sensor_get();
  int res = 0;

  if (!strcmp(variable, "interval")) interval = val;
  else if (!strcmp(variable, "vflip")) {
    res = s->set_hmirror(s, val);
    res = s->set_vflip(s, val);
  } else if (!strcmp(variable, "capture")) capture = val;
  else return httpd_resp_send_500(req);

  return httpd_resp_send(req, NULL, 0);
}

// index_htmlの送信
static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Accept-Charset", "UTF-8");
    return httpd_resp_send(req, (const char *)index_html, sizeof(index_html));
}

// カメラサーバー
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;

  // ドキュメントルートディレクトリの設定
  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,   // 呼び出される関数
    .user_ctx = NULL
  };

  // 制御用ディレクトリの設定
  httpd_uri_t cmd_uri = {
    .uri = "/control",
    .method = HTTP_GET,
    .handler = cmd_handler,     // 呼び出される関数
    .user_ctx = NULL
  };

  // 映像配信用ディレクトリの設定
  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,  // 呼び出される関数
    .user_ctx = NULL
  };

  // デフォルトポート(80)に登録するURI
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }

  // 映像配信ポート(81)に登録するURI
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}