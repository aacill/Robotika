#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// GANTI dengan data WiFi dan Telegram anda
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

const char* botToken = "BOT_TOKEN_TELEGRAM"; // Bot token dari BotFather
const char* chat_id = "CHAT_ID_ANDA"; // Chat ID pribadi/kelompok

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

#define SOUND_SENSOR_PIN 34
#define SOUND_THRESHOLD 1000 

unsigned long lastTriggerTime = 0;
const unsigned long triggerDelay = 10000; 

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = 5;
  config.pin_d1       = 18;
  config.pin_d2       = 19;
  config.pin_d3       = 21;
  config.pin_d4       = 36;
  config.pin_d5       = 39;
  config.pin_d6       = 34;
  config.pin_d7       = 35;
  config.pin_xclk     = 0;
  config.pin_pclk     = 22;
  config.pin_vsync    = 25;
  config.pin_href     = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn     = 32;
  config.pin_reset    = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera gagal diinisialisasi, error 0x%x", err);
    return;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SOUND_SENSOR_PIN, INPUT);
  setupCamera();

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Tersambung");

  client.setInsecure(); // Untuk koneksi HTTPS Telegram
}

void sendPhotoTelegram() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Gagal mengambil gambar");
    return;
  }

  Serial.println("Mengirim gambar ke Telegram...");
  bot.sendPhotoByBinary(chat_id, "image/jpeg", fb->len, fb->buf, false, "Terdeteksi suara keras!");
  esp_camera_fb_return(fb);
}

void loop() {
  int suara = analogRead(SOUND_SENSOR_PIN);
  Serial.println(suara);

  if (suara > SOUND_THRESHOLD && (millis() - lastTriggerTime > triggerDelay)) {
    lastTriggerTime = millis();
    sendPhotoTelegram();
  }

  delay(200);
}