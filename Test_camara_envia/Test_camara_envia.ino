#include "esp_camera.h"
#include <WiFi.h>

// ===== Credenciales WiFi =====
const char* ssid = "CamNet";  // Red de la Raspberry Pi
const char* password = "clave1234";

// ===== Pines AI Thinker ESP32-CAM =====
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

// ===== Configuración global =====
const char* server_ip = "192.168.4.1";  // IP de la Raspberry Pi (como AP)
const int server_port = 8000;           // Puerto del servidor Flask en la Raspberry Pi

// Variables globales
framesize_t g_frame = FRAMESIZE_SVGA;  // Resolución por defecto (800x600)
int g_jpeg_q = 12;                     // Calidad JPEG (10 a 63)
int g_fb_count_low = 1;                // Usar 1 buffer para resoluciones bajas
int g_fb_count_high = 2;               // Usar 2 buffers para resoluciones altas

void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;

  // Ajuste para 20 MHz, YUV422, con PSRAM
  config.xclk_freq_hz = 20000000; // 20 MHz
  config.pixel_format = PIXFORMAT_YUV422;  // YUV422 para la cámara GC2145
  config.frame_size   = g_frame;   // Resolución según g_frame (tipo framesize_t)
  config.jpeg_quality = g_jpeg_q;  // Calidad JPEG
  config.fb_count     = (g_frame >= FRAMESIZE_SVGA) ? g_fb_count_high : g_fb_count_low; // Usar más buffers para resoluciones altas
  config.fb_location  = CAMERA_FB_IN_PSRAM;  // Usar PSRAM para los buffers de imagen

  // Inicialización de la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("❌ Error iniciando cámara");
    while (true);  // Detener la ejecución si no se puede inicializar la cámara
  }

  // Configurar el sensor
  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    s->set_whitebal(s, 1);  // Activar balance de blancos
    s->set_awb_gain(s, 1);   // Activar ganancia de balance de blancos
    s->set_gain_ctrl(s, 1);  // Activar control de ganancia
    s->set_exposure_ctrl(s, 1);  // Activar control de exposición
    s->set_lenc(s, 1);       // Activar corrección de lente
    s->set_dcw(s, 1);        // Activar corrección de distorsión
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  startCamera();

  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi conectado");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Capturamos la imagen
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Error capturando frame");
    return;
  }

  // Enviar la imagen por HTTP POST a la Raspberry Pi
  WiFiClient client;
  if (!client.connect(server_ip, server_port)) {
    Serial.println("❌ No se puede conectar al servidor");
    esp_camera_fb_return(fb);
    delay(200);
    return;
  }

  // Construir la petición POST
  client.println("POST /frame HTTP/1.1");
  client.print("Host: ");
  client.println(server_ip);
  client.println("Content-Type: image/jpeg");
  client.print("Content-Length: ");
  client.println(fb->len);
  client.println();
  client.write(fb->buf, fb->len);

  // Detener la conexión
  client.stop();
  esp_camera_fb_return(fb);

  // Esperamos un poco antes de capturar la siguiente imagen
  delay(100); // Aproximadamente 10 FPS
}
