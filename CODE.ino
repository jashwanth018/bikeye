/*
  ESP32-CAM Bike Anti-Theft Alarm with Telegram Notification
  
  This project uses an ADXL345 accelerometer to detect movement.
  When triggered, it sounds a buzzer, rotates a servo, takes a picture 
  with the ESP32-CAM, and sends it to a specified Telegram chat. 
  An I2C LCD displays status.
  
  This version uses an averaging algorithm to detect sustained shakes and jolts,
  and a more memory-efficient method for sending photos to Telegram.
*/

// Wi-Fi and Telegram Credentials
const char* WIFI_SSID = "AKSHAY";
const char* WIFI_PASSWORD = "12345678";
const char* BOT_TOKEN = "7991367679:AAEeGAVZBi9emge4S3uL7rOOXyLvGAF1lzg";
const char* CHAT_ID = "7895398263";

// LIBRARIES
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Note: The warning about this library and ESP32 is common and can usually be ignored.
#include "esp_camera.h"
#include <ESP32Servo.h>
#include <math.h> // Added for vector calculations

// Workaround for sensor_t conflict between ESP32-CAM and Adafruit Sensor libraries
#define sensor_t Adafruit_sensor_t
#include <Adafruit_ADXL345_U.h>
#undef sensor_t

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// PIN DEFINITIONS
// Camera model: AI-THINKER
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

// I2C Pins for LCD and ADXL345
#define I2C_SDA           14
#define I2C_SCL           15

// Buzzer Pin
#define BUZZER_PIN        13

// Servo Pin
#define SERVO_PIN         12

// SENSOR AND SYSTEM SETTINGS
#define MOVEMENT_THRESHOLD 2.5  // Threshold for the average change in acceleration. Higher = less sensitive.
#define NUM_SAMPLES        10   // Number of readings to average to detect a shake.
#define DEBOUNCE_TIME_MS   10000 // 10 seconds between alerts

// OBJECTS
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Servo myServo;

// GLOBAL VARIABLES
float baselineMagnitude;
unsigned long lastTriggerTime = 0;
bool systemArmed = false;

// FUNCTION PROTOTYPES
void configInitCamera();
void sendAlert();
void beepBuzzer();
bool sendPhotoToTelegram(camera_fb_t * fb);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
  Serial.begin(115200);
  
  // Initialize I2C bus
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Starting");
  
  // Initialize Buzzer Pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize Servo
  myServo.attach(SERVO_PIN);
  myServo.write(40); 
  
  // Connect to Wi-Fi
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);

  // Initialize ADXL345 Accelerometer
  if(!accel.begin()) {
    Serial.println("Could not find a valid ADXL345 sensor");
    lcd.clear();
    lcd.print("ADXL345 Error!");
    while(1);
  }
  accel.setRange(ADXL345_RANGE_4_G);
  
  // Initialize Camera
  configInitCamera();

  // Set baseline accelerometer readings
  lcd.clear();
  lcd.print("Calibrating...");
  delay(2000);
  sensors_event_t event;
  accel.getEvent(&event);
  baselineMagnitude = sqrt(pow(event.acceleration.x, 2) + pow(event.acceleration.y, 2) + pow(event.acceleration.z, 2));


  systemArmed = true;
  lcd.clear();
  lcd.print("System Armed");
  Serial.println("System Armed and Ready!");
}

void loop() {
  if (systemArmed) {
    float totalMagnitude = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
      sensors_event_t event;
      accel.getEvent(&event);
      totalMagnitude += sqrt(pow(event.acceleration.x, 2) + pow(event.acceleration.y, 2) + pow(event.acceleration.z, 2));
      delay(10); 
    }
    
    float averageMagnitude = totalMagnitude / NUM_SAMPLES;

    if (abs(averageMagnitude - baselineMagnitude) > MOVEMENT_THRESHOLD && (millis() - lastTriggerTime > DEBOUNCE_TIME_MS)) {
      lastTriggerTime = millis();
      Serial.println("SHAKE DETECTED!");
      Serial.print("Average magnitude change: ");
      Serial.println(abs(averageMagnitude - baselineMagnitude));
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Movement!");
      
      myServo.write(120); 
      
      beepBuzzer();
      sendAlert();
      
      lcd.clear();
      lcd.print("System Armed");
    }
  }
}

void beepBuzzer() {
    for (int i = 0; i < 5; i++) {
      tone(BUZZER_PIN, 1000); 
      delay(150);
      noTone(BUZZER_PIN);
      delay(50);
    }
}

void configInitCamera() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; 
    config.jpeg_quality = 10;         
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size = FRAMESIZE_VGA; 
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    lcd.clear();
    lcd.print("Camera Failed");
    delay(2000);
    ESP.restart();
  }
}

void sendAlert() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capturing Image");
  Serial.println("Capturing image...");

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Capture Failed!");
    delay(2000);
    myServo.write(40); 
    return;
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending Alert...");
  
  if (sendPhotoToTelegram(fb)) {
    Serial.println("Message sent successfully");
    lcd.setCursor(0, 1);
    lcd.print("Alert Sent!");
  } else {
    Serial.println("Error sending message");
    lcd.setCursor(0, 1);
    lcd.print("Send Failed!");
  }
  
  esp_camera_fb_return(fb); 
  delay(2000);
  myServo.write(40); 
}

bool sendPhotoToTelegram(camera_fb_t * fb) {
  WiFiClientSecure client;
  
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  
  client.setInsecure(); 

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Telegram connection failed");
    return false;
  }
  Serial.println("Telegram connection successful");

  String boundary = "RandomBoundary";
  String caption = "ALERT: Bike movement detected!";
  
  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + String(CHAT_ID) + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n" + caption + "\r\n";
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  
  String tail = "\r\n--" + boundary + "--\r\n";
  
  uint32_t contentLength = head.length() + fb->len + tail.length();

  client.println("POST /bot" + String(BOT_TOKEN) + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Length: " + String(contentLength));
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println();
  
  client.print(head);
  
  // **FIX: Send the image data in chunks for better reliability**
  size_t fb_len = fb->len;
  uint8_t *fb_buf = fb->buf;
  const size_t chunk_size = 1024;
  for (size_t i = 0; i < fb_len; i = i + chunk_size) {
    if (i + chunk_size < fb_len) {
      client.write(fb_buf + i, chunk_size);
    } else {
      client.write(fb_buf + i, fb_len - i);
    }
  }
  
  client.print(tail);

  String responseBody = "";
  long startTimer = millis();
  while (client.connected() && (millis() - startTimer < 25000)) { 
      if (client.available()) {
          responseBody = client.readString();
          break;
      }
  }
  
  client.stop();
  Serial.println("Telegram Response: " + responseBody);

  if (responseBody.indexOf("\"ok\":true") != -1) {
    return true;
  } else {
    return false;
  }
}

