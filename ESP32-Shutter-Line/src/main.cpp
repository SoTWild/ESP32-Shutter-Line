/*
  2023/08/01 开始编辑源代码
  ========================
  2023/08/01 编写菜单界面 & Presets逻辑
  2023/08/02 编写Start
  2023/08/03 HTML
  2023/08/04 WirelessControl 核心实现
  2023/08/05 WirelessControl 完善 & Presets & SetPresets 完善
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include "HTML.h"

TFT_eSPI tft = TFT_eSPI();
WebServer server(80);

const char *ssid = "ESP32-Shutter-Line";
const char *psword = "123456789";

int SELECT_PIN = 12;
int SET_PIN = 14;
int BACK_PIN = 27;
int FOCUS_PIN = 25;
int SHUTTER_PIN = 26;
int BACKLIGHT_PIN = 22;
int REFRESH = 0;
int CameraFocusSpeed = 1500; //ms
int countdownTime = 0;  //s
int Sel = 1;
int Set = 0;
int T = 0;
int N = 1;
int I = 0;
int A = 0;
int S = 1;

char buf[128];
unsigned long line = 1;

const char* mergeStrings(const char* A, int number, const char* B) {
    // 计算所需的空间大小
    int size = strlen(A) + 12 + strlen(B) + 1;  // 12是数字转换为字符串的最大长度，额外1用于存储字符串结束符

    // 分配内存来存储结果字符串
    char* result = (char*)malloc(size * sizeof(char));

    // 将字符串 A 复制到结果字符串中
    strcpy(result, A);

    // 将数字转换为字符串并追加到结果字符串中
    char numberStr[12];  // 分配足够的空间来存储数字的字符串表示
    sprintf(numberStr, "%d", number);
    strcat(result, numberStr);

    // 将字符串 B 追加到结果字符串中
    strcat(result, B);

    return result;  // 返回合并后的字符串
}

String readFileLine(const char* path, int num) {
  Serial.printf("Reading file: %s line: %d\n", path, num);
  File file = SPIFFS.open(path);
  if (!file) {
    return ("Failed to open file for reading");
  }
  char* p = buf;
  while (file.available()) {
    char c = file.read();
    if (c == '\n') {
      num--;
      if (num == 0) {
        *(p++) = '\0';
        String s(buf);
        s.trim();
        return s;
      }
    }
    else if (num == 1) {
      *(p++) = c;
    }
  }
  file.close();
  return String("error");
}

void DrawInterface() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(40, 5, 4);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.print("Presets");
  tft.setCursor(40, 55);//85
  tft.print("Set");
  tft.setCursor(40, 105);//165
  tft.print("Wireless");
  tft.setCursor(40, 155);//165
  tft.print("Start");
}

void DrawPresets() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(4, 5, 4);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.print("   A   B   C");
}

void DrawPresetsInfo(int Type) {
  if (Type == 1) {
    File file = SPIFFS.open("/Presets_A.txt", FILE_READ);
    if(!file) {
      Serial.println("open file failed");
    }
    tft.setCursor(0 , 60);
    tft.setTextSize(1);
    tft.fillRect(0, 51, 240, 190, TFT_BLACK);
    tft.println("Timing setting : " + readFileLine("/Presets_A.txt",1));
    tft.println("Number of taken : " + readFileLine("/Presets_A.txt",2));
    tft.println("Shooting interval : " + readFileLine("/Presets_A.txt",3));
    tft.println("Auto focus : " + readFileLine("/Presets_A.txt",4));
    tft.println("Shutter speed : " + readFileLine("/Presets_A.txt",5));
    tft.setTextSize(2);
    file.close();
  }
  else if (Type == 2) {
    File file = SPIFFS.open("/Presets_B.txt", FILE_READ);
    if(!file) {
      Serial.println("open file failed");
    }
    tft.setCursor(0 , 60);
    tft.setTextSize(1);
    tft.fillRect(0, 51, 240, 190, TFT_BLACK);
    tft.println("Timing setting : " + readFileLine("/Presets_B.txt",1));
    tft.println("Number of taken : " + readFileLine("/Presets_B.txt",2));
    tft.println("Shooting interval : " + readFileLine("/Presets_B.txt",3));
    tft.println("Auto focus : " + readFileLine("/Presets_B.txt",4));
    tft.println("Shutter speed : " + readFileLine("/Presets_B.txt",5));
    tft.setTextSize(2);
    file.close();
  }
  else if (Type == 3) {
    File file = SPIFFS.open("/Presets_C.txt", FILE_READ);
    if(!file) {
      Serial.println("open file failed");
    }
    tft.setCursor(0 , 60);
    tft.setTextSize(1);
    tft.fillRect(0, 51, 240, 190, TFT_BLACK);
    tft.println("Timing setting : " + readFileLine("/Presets_C.txt",1));
    tft.println("Number of taken : " + readFileLine("/Presets_C.txt",2));
    tft.println("Shooting interval : " + readFileLine("/Presets_C.txt",3));
    tft.println("Auto focus : " + readFileLine("/Presets_C.txt",4));
    tft.println("Shutter speed : " + readFileLine("/Presets_C.txt",5));
    tft.setTextSize(2);
    file.close();
  }
}

void DrawSel() {
  tft.setTextSize(1);
  tft.setCursor(0 ,0);
  tft.fillScreen(TFT_BLACK);
  tft.println("1.Timing setting :");
  tft.println("2.Number of taken :");
  tft.println("3.Shooting interval :");
  tft.println("4.Auto focus :");
  tft.println("5.Shutter speed :");
  tft.println("6.Save as :");
}

void DrawWireless() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(40,90);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  IPAddress myIP = WiFi.softAPIP();
  tft.print("AP IP address: ");
  tft.setTextColor(TFT_RED);
  tft.setCursor(50,120);
  tft.print(myIP);
  tft.setTextColor(TFT_WHITE);
}

void DrawStart() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 55);
  tft.print("Shooting...");
}

void Select() {
  delayMicroseconds(20000);   //按键消抖
  Sel++;
  REFRESH = 1;
}

void SET() {
  delayMicroseconds(20000);
  Set++;
}

void Shoot(int TimeSet ,int Number ,int Interval ,int Auto ,int Speed) {
  DrawStart();
  countdownTime = TimeSet + Number * Interval;
  Serial.println(countdownTime);
  const char* Result = mergeStrings(Countdown_1, countdownTime, Countdown_2);
  server.send(200, "text/html", Result);  //倒计时页面
      TimeSet = TimeSet * 1000;
      Interval = Interval * 1000;
      if(Speed != 0) {
        if(Speed < 0) {
          Speed = -Speed * 1000;
        }
        else {
          Speed = 1000 / Speed;
        }
      }
      Serial.println(TimeSet);
      Serial.println(Interval);
      Serial.println(Speed);

      delay(TimeSet);//Time Setting
      for(int i = 0; i < Number; ++i) {//Start Shooting
        Serial.println(i + 1);
        if(Auto == 1) {//Auto Focus
          if(Speed == 0) {
            Serial.println("Start Shoot-Auto Focus & Automatic Exposure");
            digitalWrite(FOCUS_PIN, HIGH);
            //delay(CameraFocusSpeed);
            digitalWrite(SHUTTER_PIN, HIGH);
            delay(500);  //Automatic Exposure Time
            digitalWrite(FOCUS_PIN, LOW);
            digitalWrite(SHUTTER_PIN, LOW);
          }
          else {
            Serial.println("Start Shoot-Auto Focus & BULB");
            digitalWrite(FOCUS_PIN, HIGH);
            //delay(CameraFocusSpeed);
            digitalWrite(SHUTTER_PIN, HIGH);
            delay(Speed); //Shutter Speed
            digitalWrite(FOCUS_PIN, LOW);
            digitalWrite(SHUTTER_PIN, LOW);
          }
        }
        if(Auto == 0) {
          if(Speed == 0) {
            Serial.println("Start Shoot-Manual Focus & Automatic Exposure");
            digitalWrite(SHUTTER_PIN, HIGH);
            delay(500);
            digitalWrite(SHUTTER_PIN, LOW);
          }
          else {
            Serial.println("Start Shoot-Manual Focus & BULB");
            digitalWrite(SHUTTER_PIN, HIGH);
            delay(Speed);//Shutter Speed
            digitalWrite(SHUTTER_PIN, LOW);
          }
        }
        delay(Interval);//Shooting Interval
      }
      Set = 0;
      Sel = 1;
      REFRESH = 1;
      DrawInterface();
}

void favicon() {

}

void ShootHandleRoot() {
  int input1Value = atoi(server.arg("input1").c_str());
  int input2Value = atoi(server.arg("input2").c_str());
  int input3Value = atoi(server.arg("input3").c_str());
  int input4Value = atoi(server.arg("input4").c_str());
  int input5Value = atoi(server.arg("input5").c_str());

  Serial.println(input1Value);
  Serial.println(input2Value);
  Serial.println(input3Value);
  Serial.println(input4Value);
  Serial.println(input5Value);

  Shoot(input1Value ,input2Value ,input3Value ,input4Value ,input5Value);
}

void NotFound() {
  //server.send(200, "text/html", HandleNotFound);
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void HomePage() {
  server.send(200, "text/html", WirelessControlTerminal);
}

void ManualSet() {
  server.send(200, "text/html", ManuallySetting);
}

void Presets() {
  std::string Result = std::string(Presets_1) + 
  readFileLine("/Presets_A.txt",1).c_str() + "<p>" + Presets_2 +
  readFileLine("/Presets_A.txt",2).c_str() + "<p>" + Presets_3 +
  readFileLine("/Presets_A.txt",3).c_str() + "<p>" + Presets_4 +
  readFileLine("/Presets_A.txt",4).c_str() + "<p>" + Presets_5 +
  readFileLine("/Presets_A.txt",5).c_str() + Presets_6 +
  readFileLine("/Presets_B.txt",1).c_str() + "<p>" + Presets_2 +
  readFileLine("/Presets_B.txt",2).c_str() + "<p>" + Presets_3 +
  readFileLine("/Presets_B.txt",3).c_str() + "<p>" + Presets_4 +
  readFileLine("/Presets_B.txt",4).c_str() + "<p>" + Presets_5 +
  readFileLine("/Presets_B.txt",5).c_str() + Presets_7 +
  readFileLine("/Presets_C.txt",1).c_str() + "<p>" + Presets_2 +
  readFileLine("/Presets_C.txt",2).c_str() + "<p>" + Presets_3 +
  readFileLine("/Presets_C.txt",3).c_str() + "<p>" + Presets_4 +
  readFileLine("/Presets_C.txt",4).c_str() + "<p>" + Presets_5 +
  readFileLine("/Presets_C.txt",5).c_str() + Presets_8;

  server.send(200, "text/html", Result.c_str());
}

void PresetsA() {
  T = atoi(readFileLine("/Presets_A.txt",1).c_str());
            N = atoi(readFileLine("/Presets_A.txt",2).c_str());
            I = atoi(readFileLine("/Presets_A.txt",3).c_str());
            A = atoi(readFileLine("/Presets_A.txt",4).c_str());
            S = atoi(readFileLine("/Presets_A.txt",5).c_str());
  Shoot(T ,N ,I ,A ,S);
}

void PresetsB() {
  T = atoi(readFileLine("/Presets_B.txt",1).c_str());
            N = atoi(readFileLine("/Presets_B.txt",2).c_str());
            I = atoi(readFileLine("/Presets_B.txt",3).c_str());
            A = atoi(readFileLine("/Presets_B.txt",4).c_str());
            S = atoi(readFileLine("/Presets_B.txt",5).c_str());
            Shoot(T ,N ,I ,A ,S);
}

void PresetsC() {
  T = atoi(readFileLine("/Presets_C.txt",1).c_str());
            N = atoi(readFileLine("/Presets_C.txt",2).c_str());
            I = atoi(readFileLine("/Presets_C.txt",3).c_str());
            A = atoi(readFileLine("/Presets_C.txt",4).c_str());
            S = atoi(readFileLine("/Presets_C.txt",5).c_str());
            Shoot(T ,N ,I ,A ,S);
}

void SetPresets() {
  server.send(200, "text/html", SetPresets_html);
}

void SetPresetsHandleRoot() {
  int Group = atoi(server.arg("group").c_str());
  int input1Value = atoi(server.arg("input1").c_str());
  int input2Value = atoi(server.arg("input2").c_str());
  int input3Value = atoi(server.arg("input3").c_str());
  int input4Value = atoi(server.arg("input4").c_str());
  int input5Value = atoi(server.arg("input5").c_str());

  Serial.println(Group);
  Serial.println(input1Value);
  Serial.println(input2Value);
  Serial.println(input3Value);
  Serial.println(input4Value);
  Serial.println(input5Value);

  if (Group == 1) {
    File file = SPIFFS.open("/Presets_A.txt", FILE_WRITE);
    if(!file) {
      Serial.println("open file failed");
    }
    file.println(input1Value);//T
    file.println(input2Value);//N
    file.println(input3Value);//I
    file.println(input4Value);//A
    file.println(input5Value);//S
    file.close();
  }
  else if (Group == 2) {
    File file = SPIFFS.open("/Presets_B.txt", FILE_WRITE);
    if(!file) {
      Serial.println("open file failed");
    }
    file.println(input1Value);//T
    file.println(input2Value);//N
    file.println(input3Value);//I
    file.println(input4Value);//A
    file.println(input5Value);//S
    file.close();
  }
  else if (Group == 3) {
    File file = SPIFFS.open("/Presets_C.txt", FILE_WRITE);
    if(!file) {
      Serial.println("open file failed");
    }
    file.println(input1Value);//T
    file.println(input2Value);//N
    file.println(input3Value);//I
    file.println(input4Value);//A
    file.println(input5Value);//S
    file.close();
  }
  server.send(200, "text/html", Success);
}

void ClearPresets() {
  //SPIFFS.format();
  SPIFFS.remove("/Presets_A.txt");
  SPIFFS.remove("/Presets_B.txt");
  SPIFFS.remove("/Presets_C.txt");
  server.send(200, "text/html", Success);
}

void WirelessControl() {
  DrawWireless();

  server.on("/", HomePage);
  server.on("/M", ShootHandleRoot);
  server.on("/Presets.html", Presets);
  server.on("/PresetsA", PresetsA);
  server.on("/PresetsB", PresetsB);
  server.on("/PresetsC", PresetsC);
  server.on("/SetPresets.html", SetPresets);
  server.on("/ClearPresets", ClearPresets);
  server.on("/SetPresets", SetPresetsHandleRoot);
  server.on("/ManuallySetting.html", ManualSet);
  server.on("/favicon.ico", favicon);
  server.onNotFound(NotFound);
  server.begin();

  while(1) {
    server.handleClient();
    if(digitalRead(BACK_PIN) == 0) {
          DrawInterface();
          REFRESH = 1;
          Set = 0;
          break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  Serial.println("Initiate WiFi AP");
  WiFi.softAP(ssid, psword);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Serial.println("Initiate SPIFFS");
  //SPIFFS.format();
  while (!SPIFFS.begin()) {
    Serial.print(".");
  }
  Serial.println("SPIFFS OK!");
  if(!SPIFFS.exists("/Presets_A.txt")) {
    File file = SPIFFS.open("/Presets_A.txt", FILE_WRITE);
    if (!file) {
    Serial.println("open file failed");
    }
    file.close();
  }
  else if(!SPIFFS.exists("/Presets_B.txt")) {
    File file = SPIFFS.open("/Presets_B.txt", FILE_WRITE);
    if (!file) {
    Serial.println("open file failed");
    }
    file.close();
  }
  else if(!SPIFFS.exists("/Presets_C.txt")) {
    File file = SPIFFS.open("/Presets_C.txt", FILE_WRITE);
    if (!file) {
    Serial.println("open file failed");
    }
    file.close();
  }
  /*File file = SPIFFS.open("/Presets_A.txt", FILE_WRITE);//延时设置
  if(!file) {
    Serial.println("open file failed");
  }
  file.println("3");//T
  file.println("1800");//N
  file.println("3");//I
  file.println("0");//A
  file.println("0");//S
  file.close();
  file = SPIFFS.open("/Presets_B.txt", FILE_WRITE);//自拍设置
  if(!file) {
    Serial.println("open file failed");
  }
  file.println("10");//T
  file.println("3");//N
  file.println("1");//I
  file.println("1");//A
  file.println("0");//S
  file.close();
  file = SPIFFS.open("/Presets_C.txt", FILE_WRITE);//长曝光设置
  if(!file) {
    Serial.println("open file failed");
  }
  file.println("3");//T
  file.println("1");//N
  file.println("0");//I
  file.println("0");//A
  file.println("-45");//S
  file.close();*/

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  DrawInterface();
  tft.setCursor(5, 5, 4);
  tft.print(">");

  pinMode(SELECT_PIN, INPUT_PULLUP);
  pinMode(SET_PIN, INPUT_PULLUP);
  pinMode(BACK_PIN, INPUT_PULLUP);
  pinMode(FOCUS_PIN, OUTPUT);
  pinMode(SHUTTER_PIN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(SELECT_PIN), Select, FALLING);
  attachInterrupt(digitalPinToInterrupt(SET_PIN), SET, FALLING);
  //attachInterrupt(digitalPinToInterrupt(BACK_PIN), BACK, FALLING);
  //digitalWrite(SHUTTER_PIN, HIGH);
  //delay(100);
  //digitalWrite(SHUTTER_PIN, LOW);
  digitalWrite(BACKLIGHT_PIN, HIGH);

  REFRESH = 1;
}

void loop() {
  //Serial.println(Sel);
  if((0 <= Sel)&&(Sel <= 4)) {
    if((Sel == 1)&&(REFRESH == 1)) {
      tft.fillRect(0, 0, 40, 200, TFT_BLACK);
      tft.setCursor(5, 5, 4);
      tft.print(">");
      REFRESH = 0;
    }
    else if((Sel == 2)&&(REFRESH == 1)) {
      tft.fillRect(0, 0, 40, 200, TFT_BLACK);
      tft.setCursor(5, 55, 4);
      tft.print(">");
      REFRESH = 0;
    }
    else if((Sel == 3)&&(REFRESH == 1)) {
      tft.fillRect(0, 0, 40, 200, TFT_BLACK);
      tft.setCursor(5, 105, 4);
      tft.print(">");
      REFRESH = 0;
    }
    else if((Sel == 4)&&(REFRESH == 1)) {
      tft.fillRect(0, 0, 40, 200, TFT_BLACK);
      tft.setCursor(5, 155, 4);
      tft.print(">");
      REFRESH = 0;
    }
  }
  else {
    Sel = 1;
  }
  if(Set == 1) {
    if(Sel == 1) {
      DrawPresets();
      Set = 0;
      REFRESH = 1;
      while(1) {//Presets
        if((0 <= Sel)&&(Sel <= 3)) {
          if((Sel == 1)&&(REFRESH == 1)) {
            tft.fillRect(0, 0, 240, 50, TFT_BLACK);
            tft.setCursor(5, 5, 4);
            tft.print(" > A   B   C");
            REFRESH = 0;
            DrawPresetsInfo(1);
          }
          else if((Sel == 2)&&(REFRESH == 1)) {
            tft.fillRect(0, 0, 240, 50, TFT_BLACK);
            tft.setCursor(5, 5, 4);
            tft.print("   A > B   C");
            REFRESH = 0;
            DrawPresetsInfo(2);
          }
          else if((Sel == 3)&&(REFRESH == 1)) {
            tft.fillRect(0, 0, 240, 50, TFT_BLACK);
            tft.setCursor(5, 5, 4);
            tft.print("   A   B > C");
            REFRESH = 0;
            DrawPresetsInfo(3);
          }
          if((Sel == 1)&&(Set == 1)) {
            Set = 0;
            T = atoi(readFileLine("/Presets_A.txt",1).c_str());
            N = atoi(readFileLine("/Presets_A.txt",2).c_str());
            I = atoi(readFileLine("/Presets_A.txt",3).c_str());
            A = atoi(readFileLine("/Presets_A.txt",4).c_str());
            S = atoi(readFileLine("/Presets_A.txt",5).c_str());
          }
          else if((Sel == 2)&&(Set == 1)) {
            Set = 0;
            T = atoi(readFileLine("/Presets_B.txt",1).c_str());
            N = atoi(readFileLine("/Presets_B.txt",2).c_str());
            I = atoi(readFileLine("/Presets_B.txt",3).c_str());
            A = atoi(readFileLine("/Presets_B.txt",4).c_str());
            S = atoi(readFileLine("/Presets_B.txt",5).c_str());
          }
          else if((Sel == 3)&&(Set == 1)) {
            Set = 0;
            T = atoi(readFileLine("/Presets_C.txt",1).c_str());
            N = atoi(readFileLine("/Presets_C.txt",2).c_str());
            I = atoi(readFileLine("/Presets_C.txt",3).c_str());
            A = atoi(readFileLine("/Presets_C.txt",4).c_str());
            S = atoi(readFileLine("/Presets_C.txt",5).c_str());
          }
        }
        else {
          Sel = 1;
        }
        if(digitalRead(BACK_PIN) == 0) {
          Serial.println(T);
          Serial.println(N);
          Serial.println(I);
          Serial.println(A);
          Serial.println(S);
          DrawInterface();
          REFRESH = 1;
          Set = 0;
          break;
        }
        else {
          delay(200);
        }
      }
    }
    else if(Sel == 2) {
      DrawSel();
      Set = 0;
      while(1) {//Set (NOT DONE YET)
        if((0 <= Sel)&&(Sel <= 8)) {
          if((Sel == 1)&&(REFRESH == 1)) {
            //tft.fillRect(200, 0, 40, 150, TFT_YELLOW);
            DrawSel();
            tft.println(">1 2 3 4 5 6");
            REFRESH = 0;
          }
          else if((Sel == 2)&&(REFRESH == 1)) {
            //tft.fillRect(200, 0, 40, 150, TFT_YELLOW);
            DrawSel();
            tft.println(" 1>2 3 4 5 6");
            REFRESH = 0;
          }
          //else
        }
        if(digitalRead(BACK_PIN) == 0) {
          DrawInterface();
          REFRESH = 1;
          Set = 0;
          break;
        }
        else {
          delay(200);
        }
      }
    }
    else if(Sel == 3) {
      Set = 0;
      WirelessControl();  //Wireless
    }
    else if(Sel == 4) {//Start
      Shoot(T ,N ,I ,A ,S);
    }
  }
  else {
    Set = 0;
  }
}