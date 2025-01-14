#include <ESP8266WiFi.h>
#include <espnow.h>
#define D1 5  // GPIO5
#define D2 4  // GPIO4
#define D5 14 // GPIO14
#define D6 12 // GPIO12
#define D7 13 // GPIO13
int isAutoMode =0;
uint8_t broadcastAddress[] = {0x8c, 0x4F, 0x00, 0x0F, 0x72, 0x08};  // Địa chỉ MAC của ESP32 8C:4F:00:0F:72:08
typedef struct struct_message {
  char control[32]; // Chuỗi lệnh điều khiển
  float kp;      // Giá trị float 1
  float ki;      // Giá trị float 2
  float kd;      // Giá trị float 3
} struct_message;

struct_message myData;

void sendData(const char *msg) {
  strcpy(myData.control, msg);
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
}

void loop() {
   
   if (digitalRead(D7) == LOW) {  // Khi nút nhấn được nhấn
      delay(200); // Chống dội nút
      isAutoMode = isAutoMode + 1;
      if (isAutoMode > 2) {isAutoMode=0;}
      if (isAutoMode == 0) {
          sendData("manual"); 
          Serial.print("manual");
          Serial.println();
           // Gửi lệnh chuyển sang chế độ tự động
      } 
      if (isAutoMode == 1) {
          sendData("fll"); 
          Serial.print("fll");
          Serial.println();
           // Gửi lệnh chuyển sang chế độ tự động
      }  
    }
  if (digitalRead(D1) == LOW) {
    sendData("phai");
    Serial.print("phải");
    Serial.println();
    delay(200);
  }
  if (digitalRead(D2) == LOW) {
    sendData("len");
        Serial.print("lên");
        Serial.println();
        delay(200);

  }
  if (digitalRead(D5) == LOW) {
    sendData("trai");
        Serial.print("trái");
        Serial.println();
        delay(200);

  }
  if (digitalRead(D6) == LOW) {
    sendData("speed");
        Serial.print("speed");
        Serial.println();
        delay(200);

  }
  
}