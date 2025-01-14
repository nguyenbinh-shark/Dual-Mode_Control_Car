
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Không dùng reset
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <WiFi.h>
#include <esp_now.h>
#define a1 39
#define a2 36
#define a3 33
#define a4 32
#define a5 35
#define a6 34
#define motorPin1 4 // Chân điều khiển động cơ trái
#define motorPin2 5 // Chân điều khiển động cơ phải
#define sampling_time 28 // thời gian lấy mẫu
#define inv_sampling_time 35 // 1/samplig time
#define ss1 16  // Chân Trig của cảm biến siêu âm
#define ss2 17  // Chân Echo của cảm biến siêu âm
#define buzzerPin 12 // Chân điều khiển còi
static unsigned long lastToggleTime = 0;
unsigned long toggleInterval = 20000; // Khoảng thời gian nhấp nháy (ms)
// Định nghĩa cấu trúc tin nhắn
float second,first,re;//khai báo biến để xác đinh thời gian 1 vongf looploop
typedef struct struct_message {
  char control[32];  // Chuỗi lệnh điều khiển
  float kp;          // Tham số PID - Proportional
  float ki;          // Tham số PID - Integral
  float kd;          // Tham số PID - Derivative
  float speed;      // Tham số tốc độ
} struct_message;
struct_message myData;  // Biến để chứa tin nhắn nhận được
// phần khai báo các biến
int starttime =0;         // đếm thời gian 
int duration =500 ;       // mốc thời gian
int set=1,s1,s2,sp=0,pwm,SS1,SS2, controlstop;          // biến trạng thái auto/manual
float P,I,D , PID,ENA,ENB;
int check,err,pre_err,err1,i_err;
int s[6],f[6],st[6]={1400,890,1000,1000,1030,950};
unsigned long now = 0;
unsigned long settime = 30000;
// Hàm callback khi nhận dữ liệu qua ESP-NOW
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  // In ra lệnh điều khiển nhận đượcc
  Serial.print("Nhận lệnh: ");
  Serial.println(myData.control);
  if (strcmp(myData.control, "fll") == 0) { // chế độ tự động dò line
    set = 1;
    }
  if (strcmp(myData.control, "manual") == 0) { // chế độ điều khiển
    set = 0;
    }
  pwm =myData.speed;

}
void setup() { 
  // Thiết lập chế độ cho các chân điều khiển
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(a1,INPUT);
  pinMode(a2,INPUT);
  pinMode(a3,INPUT);
  pinMode(a4,INPUT);
  pinMode(a5,INPUT);
  pinMode(a6,INPUT);
  pinMode(ss1, INPUT);
  pinMode(ss2, INPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // Chế độ Station
  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);// Đăng ký hàm callback nhận dữ liệu
  // Bắt đầu I2C và khởi tạo màn hình OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Dừng lại nếu không khởi tạo được
  }
}
void control(){
  // Tình thông số PID
   P=myData.kp*err;
   D=myData.kd*(err-pre_err)*inv_sampling_time;
   i_err=myData.ki*(sampling_time*(err+pre_err)/2000);
   I=I+i_err;
   PID=(P+I+D);
   ENA=pwm+PID; // động cơ trái
   ENB=pwm-PID; // động cơ phải
   if (ENA > 255) ENA =255; //kiểm tra PWM tràn số
   if (ENA < 0  ) ENA =0;
   if (ENB > 255) ENB =255;
   if (ENB < 0  ) ENB =0;
   analogWrite(motorPin1,ENA); // gán xung pwm
   analogWrite(motorPin2,ENB);
   pre_err=err; 
   check=err;
}
void loop() { 
  first=millis();// tính thời gian vòng lặp
  SS1=digitalRead(ss1);
  SS2=digitalRead(ss2);
  if (SS1 == 0 || SS2==0) {
    digitalWrite(buzzerPin,HIGH);
    analogWrite(motorPin1, 0);
    analogWrite(motorPin2, 0); // Đảo trạng thái còi
    delay(5000);
  }else {
    digitalWrite(buzzerPin, LOW); // Tắt còi khi không có vật
  }

if( set == 0){

  if (strcmp(myData.control, "len") == 0) {
    Serial.println("Chạy tới");
    analogWrite(motorPin1, pwm);
    analogWrite(motorPin2, pwm +10);
  } 
  if (strcmp(myData.control, "trai") == 0) {
    Serial.println("trái");
    analogWrite(motorPin1, pwm-pwm*0.66);
    analogWrite(motorPin2, pwm);
  } 
  if (strcmp(myData.control, "phai") == 0) {
    Serial.println("phải");
    analogWrite(motorPin1, pwm);
    analogWrite(motorPin2, pwm-pwm*0.66);
  } 
  if (strcmp(myData.control, "xuong") == 0) {
    Serial.println("Dừng");
    analogWrite(motorPin1, 0);
    analogWrite(motorPin2, 0);
  } 
  if (millis()- starttime >= duration){
      strcmp(myData.control, "xuong");
      strcpy(myData.control, "xuong");      
      starttime = millis();  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("  TRAN NGUYEN BINH  "));
  display.setTextSize(2);
  display.print(F("   Manual  "));
  display.println();
  display.print("Speed: ");
  display.print(sp);
  display.display();
  }
}
if(set == 1 ){
  control();
    if (millis() - now >= settime) {
        I = 0; // Reset biến
        // Serial.println("Biến I đã được reset về 0.");
        now = millis();  // Cập nhật thời gian reset lần cuối
      }
// cảm biến 1
        delayMicroseconds(10); 
        s[0]=analogRead(a1);
        if (s[0] > st[0]) {
          f[0]=1;
          err =-7;
          if (check != err) check=err;
        } else {f[0]=0;}
// cảm biến 2
        delayMicroseconds(10); 
        s[1]=analogRead(a2);
        if (s[1] > st[1]) {
          f[1]=1;
          err =-3;
          if (check != err) check=err;
        }else{f[1]=0;}
//cảm biến 3
        delayMicroseconds(10);
        s[2]=analogRead(a3);
        if (s[2] > st[2]) {
          f[2]=1;
          err =0;
          if (check != err) check=err;
        } else{f[2]=0;}
//cảm biến 4
        delayMicroseconds(10); 
        s[3]=analogRead(a4);
        if (s[3] > st[3]) {
          f[3]=1;
          err = 0;
          if (check != err) check=err;
        }else{f[3]=0;}
//cảm biến 5
        delayMicroseconds(10); 
        s[4]=analogRead(a5);
        if (s[4] > st[4]) {
          f[4]=1;
          err =3;
          if (check != err) check=err;
        }else{f[4]=0;}
//cảm biến 6
        delayMicroseconds(10); 
        s[5]=analogRead(a6);
        if (s[5] > st[5]) {
          f[5]=1;
          err =7;
          if (check != err) check=err;
        }else{f[5]=0;}
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("TRAN NGUYEN BINH"));
  display.print(F("follow line"));
  display.println();
    for(int i=0;i<6;i++){
    display.print(f[i]);
    display.print(" ");  
    }
    display.println();
    display.print("err:");
    display.print(err);
    display.println();
    display.print(" P:");
    display.print(P);
    display.print(" I:");
    display.print(I);
    display.print(" D:");
    display.print(D);
    display.println(); 
    display.print(" PID:");
    display.print(set);
    display.display();
  }
  second =millis();
re = second - first;// tính thời gian vòng lặp
    Serial.println(re);
    Serial.println(SS1);
    Serial.println(SS2);
    delay(10);// tính thời gian vòng lặp
}