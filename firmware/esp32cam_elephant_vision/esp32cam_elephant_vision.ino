/**
 * =========================================================================================
 * VETRAX: ESP32-CAM Elephant & Wildlife Optical Reconnaissance Subsystem
 * =========================================================================================
 * Target Hardware : AI-Thinker ESP32-CAM (OV2640 2MP Camera Module)
 * Functionality   : Live MJPEG Video Streaming, Night Flash Illumination, Snapshot Capture
 * Application     : Visual tracking of elephant movement corridors & human-wildlife safety
 * =========================================================================================
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"

// --- AI-THINKER ESP32-CAM PINOUT MAPPING ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Onboard high-power Flash LED (for night wildlife tracking)
#define FLASH_LED_PIN      4

// --- NETWORK CREDENTIALS ---
// Connects to the main VETRAX Scout AP or basecamp network
const char* ssid = "VETRAX_Scout_AP";
const char* password = "elephant123";

// HTTP Server Handles
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;
bool flashState = false;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// --- MJPEG VIDEO STREAM HANDLER ---
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println(" ⚠️ Camera frame capture failed");
      res = ESP_FAIL;
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (res != ESP_OK) {
      break;
    }
  }
  return res;
}

// --- STILL CAPTURE HANDLER ---
static esp_err_t capture_handler(httpd_req_t *req) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

// --- NIGHT FLASH LED TOGGLE HANDLER ---
static esp_err_t flash_handler(httpd_req_t *req) {
  flashState = !flashState;
  digitalWrite(FLASH_LED_PIN, flashState ? HIGH : LOW);
  char resp[32];
  snprintf(resp, sizeof(resp), "Flash LED: %s", flashState ? "ON" : "OFF");
  httpd_resp_send(req, resp, strlen(resp));
  return ESP_OK;
}

// --- WEB PORTAL HANDLER ---
static esp_err_t index_handler(httpd_req_t *req) {
  const char html[] = 
    "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>VETRAX Elephant Cam</title><style>"
    "body{background:#111827;color:#f3f4f6;text-align:center;font-family:sans-serif;margin:0;padding:12px;}"
    "h2{color:#60a5fa;margin-bottom:12px;}"
    ".stream-box{max-width:100%;border-radius:12px;border:2px solid #374151;overflow:hidden;box-shadow:0 4px 14px rgba(0,0,0,0.5);}"
    "img{width:100%;max-width:640px;height:auto;display:block;margin:0 auto;}"
    ".btn{padding:12px 24px;margin:10px;font-size:16px;border-radius:8px;border:none;cursor:pointer;font-weight:700;}"
    ".btn-flash{background:#f59e0b;color:#111827;}"
    ".btn-snap{background:#10b981;color:#fff;}"
    "</style></head><body>"
    "<h2>🐘 VETRAX Night & Day Wildlife Stream</h2>"
    "<div class='stream-box'><img src='/stream'></div><br>"
    "<button class='btn btn-flash' onclick=\"fetch('/flash')\">🔦 Toggle Night Flash</button>"
    "<button class='btn btn-snap' onclick=\"window.open('/capture','_blank')\">📸 Snapshot</button>"
    "</body></html>";
  httpd_resp_send(req, html, strlen(html));
  return ESP_OK;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
  httpd_uri_t capture_uri = { .uri = "/capture", .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL };
  httpd_uri_t flash_uri = { .uri = "/flash", .method = HTTP_GET, .handler = flash_handler, .user_ctx = NULL };
  httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &capture_uri);
    httpd_register_uri_handler(camera_httpd, &flash_uri);
    httpd_register_uri_handler(camera_httpd, &stream_uri);
    Serial.println(" ✅ Camera Web Server started on port 80");
  }
}

void setup() {
  // Prevent brownout resets during camera & Wi-Fi transmission
  #ifdef RTC_CNTL_BROWN_OUT_REG
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  #endif

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- VETRAX ESP32-CAM Wildlife Optical Unit ---");

  // Camera Hardware Configuration
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
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame size & quality configuration
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA; // 640x480 resolution
    config.jpeg_quality = 12;          // 0-63, lower means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA; // 320x240 for non-PSRAM fallback
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  // Camera Initialization
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf(" ❌ Camera init failed with error 0x%x\n", err);
    return;
  }
  Serial.println(" ✅ OV2640 Camera Sensor Initialized successfully");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print(" 📡 Connecting to VETRAX Scout Network");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n ✅ Wi-Fi Connected!");
  Serial.print(" 🌐 Live Stream Available at: http://");
  Serial.println(WiFi.localIP());

  // Start HTTP Streaming Server
  startCameraServer();
}

void loop() {
  delay(10000);
}
