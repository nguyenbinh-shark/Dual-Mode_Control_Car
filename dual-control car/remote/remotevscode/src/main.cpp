#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
float KP = 7.0, KI = 0.5, KD = 5.0  ;
int selectedParam = 0,SPEED; // 0: KP, 1: KI, 2: KD
#define D0 16  // GPIO5 d0 lefft
#define D5 14 // GPIO14 d5 righ
#define D3 0 // GPIO12  forwar
#define D7 13 // GPIO13 d7 mode
#define BUTTON_SELECT 15 // Chọn thông số d8
#define BUTTON_UP     12 // Tăng giá trị d6
#define BUTTON_DOWN   2 // Giảm giá trị d4
int isAutoMode =0;
int Pre_SPEED;

uint8_t broadcastAddress[] = {0x8c, 0x4F, 0x00, 0x0F, 0x72, 0x08};  // Địa chỉ MAC của ESP32 8C:4F:00:0F:72:08
typedef struct struct_message {
  char control[32]; // Chuỗi lệnh điều khiển
  float kp;      // Giá trị float 1
  float ki;      // Giá trị float 2
  float kd;      // Giá trị float 3
  float speed; 
} struct_message;

struct_message myData;

void sendData(const char *msg, float kp, float ki, float kd, int speed) {
  strcpy(myData.control, msg); // Cập nhật chuỗi điều khiển
  myData.kp = KP; // Cập nhật kp
  myData.ki = KI; // Cập nhật ki
  myData.kd = KD; // Cập nhật kd
  myData.speed = speed; // Cập nhật speed
  
  // Gửi  toàn bộ struct qua ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

// Các thông số KP, KI, KD


// Chân nút bấm

void drawParameters() {
  display.clearDisplay();

  // Vẽ thông số KP
  if (selectedParam == 0) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Đảo màu
  } else {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }
  display.setCursor(0, 0);
  display.print("KP: ");
  display.print(KP, 1);

  // Vẽ thông số KI
  if (selectedParam == 1) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Đảo màu
  } else {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }
  display.setCursor(0, 16);
  display.print("KI: ");
  display.print(KI, 1);

  // Vẽ thông số KD
  if (selectedParam == 2) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Đảo màu
  } else {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }
  display.setCursor(0, 32);
  display.print("KD: ");
  display.print(KD, 1);

  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setCursor(60, 0);  
  display.display();
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

  pinMode(D0, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Không thể khởi tạo OLED"));
    while (true);
  }

  display.clearDisplay();
  display.display();
  drawParameters();
  display.setCursor(10, 48);
  display.print("Speed: ");
  display.print(SPEED);

}

void loop() {
  SPEED = (analogRead(A0)*100)/1024;
  if(Pre_SPEED != SPEED){
  display.setCursor(10, 48);
  display.print("SPEED: ");
  display.print(SPEED);
  display.display();
  Pre_SPEED = SPEED;

  }
  // Đọc các nút bấm
  if (digitalRead(BUTTON_SELECT) == HIGH) {
    selectedParam = (selectedParam + 1) % 3; // Chuyển giữa 3 thông số
    drawParameters();
    delay(200); // Tránh nhận nhiều lần nhấn
  }

  if (digitalRead(BUTTON_UP) == LOW) {
    if (selectedParam == 0) KP += 0.1;
    else if (selectedParam == 1) KI += 0.1;
    else if (selectedParam == 2) KD += 0.1;
    drawParameters();
    sendData("no",KP,KI,KD,SPEED );
    delay(200);
  }

  if (digitalRead(BUTTON_DOWN) == LOW) {
    if (selectedParam == 0) KP -= 0.1;
    else if (selectedParam == 1) KI -= 0.1;
    else if (selectedParam == 2) KD -= 0.1;
    drawParameters();
    sendData("no",KP,KI,KD,SPEED );
    delay(200);
  }
  if (digitalRead(D7) == LOW) {  // Khi nút nhấn được nhấn
      delay(200); // Chống dội nút
      isAutoMode = isAutoMode + 1;
      if (isAutoMode > 2) {isAutoMode=0;}
      if (isAutoMode == 0) {
          sendData("manual",KP,KI,KD,SPEED); 
          Serial.print("manual");
          Serial.println();
           // Gửi lệnh chuyển sang chế độ tự động
      } 
      if (isAutoMode == 1) {
          sendData("fll",KP,KI,KD,SPEED ); 
          Serial.print("fll");
          Serial.println();
           // Gửi lệnh chuyển sang chế độ tự động
      }  
    }
  if (digitalRead(D0) == LOW) {
    sendData("phai",KP,KI,KD,SPEED );
    Serial.print("phải");
    Serial.println();
    delay(200);
  }
  if (digitalRead(D3) == LOW) {
    sendData("len",KP,KI,KD,SPEED );
        Serial.print("lên");
        Serial.println();
        delay(200);

  }
  if (digitalRead(D5) == LOW) {
    sendData("trai",KP,KI,KD,SPEED);
        Serial.print("trái");
        Serial.println();
        delay(200);

  }
  
}

