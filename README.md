# ESP32-Shutter-Line

###### 【Designed By SoTWild】![](https://img.shields.io/badge/Licence-GPL-blue) ![](https://img.shields.io/badge/Platform-Platform%20IO-success) ![](https://img.shields.io/badge/Version-1.0-red) ![](https://img.shields.io/badge/Language-C%2FC%2B%2B-blueviolet)

## 概述：

ESP32-Shutter-Line 是一款基于 `ESP32-WROOM` 制作的快门线，主要适配**佳能**机型。

###### 主要功能：

> 1.自定义设置
>
> 2.最多三种预设
>
> 3.无线遥控&设置
>
> [是否定时几秒]+[拍摄几张]+[间隔时间]+[是否要自动对焦]+[快门速度]

------

### FAQ：

#### 1）整个设备的造价大约是多少？

< 30 ￥

#### 2）复刻的难度如何？

硬件 SMD，软件开源，综合来看很容易复刻。

#### 3）续航时间？

充电芯片采用 TP4056，可以通过配置 `PROG` 引脚控制充电电流，我这里使用了 `500mA` 充电电流，电池为 `500mAh` 聚合物锂电池，理论上可以连续供电超过 `3` 小时，充电时间约为 `1` 小时。（运行电流约为 `150mA`）

#### 4）成品的体积？

长 10cm，宽 4cm，厚 8mm（不加电池）。

#### 5）主控芯片是什么？

`ESP32-D0WD-V3` 双核，模组为 `ESP32-WROOM`。

#### 6）开发环境？

Visual Studio Code + Platform IO

------

## 硬件

### 功能实现原理：

在 **RS-80N3** 型号快门线中，引出了**黄、白、红**三线，根据实验（网上查询也可以）可得知：

> **白红**短接**自动对焦+拍摄**，两端 **3.3V**
>
> **白黄**短接**自动对焦**，两端 **3.3V**
>
> **黄红**短接**无效果**，两端 **0V**

由此可得**白线连接了相机内部的 `GND` **网络，**黄、红大概率连接内部芯片 IO** 引脚，芯片通过检测 IO 对地电平判断是否有对应拍摄指令。

通过三极管可令 ESP32 对引脚进行控制，这里使用 S8050 三极管。

<img src=".\Images\S8050.gif" alt="S8050" style="zoom:25%;" />

<center>S8050</center>

加上锂电池充放电管理、屏幕驱动以及按键，可以简单设计出以下原理图：

### 原理图：

<img src=".\Images\Schematic_ESP32-Shutter-Line.png" alt="Schematic_ESP32-Shutter-Line" style="zoom:100%;" />

### PCB：

<img src=".\Images\IMAGE.png" alt="IMAGE" style="zoom:50%;" />

### BOM表：

| ID   | Name                        | Designator       | Footprint                          |
| ---- | --------------------------- | ---------------- | ---------------------------------- |
| 1    | K4-12×12_TH                 | BACK,SELECT,SET  | KEY-TH_4P-L12.0-W12.0-P5.00-LS12.5 |
| 2    | 10μF/10V                    | C1,C3            | 0805_C_JX                          |
| 3    | 100nF/50V                   | C2,C4            | 0603_C_JX                          |
| 4    | 0.1u                        | C5               | 1206                               |
| 5    | 1uF                         | C6,C7,C8,C9      | C0603                              |
| 6    | S8050LT1-J3Y                | FOCUS,Q2,SHUTTER | SOT-23_L2.9-W1.3-P1.90-LS2.4-BR    |
| 7    | 排针7P-2.54MM直插           | H1               | 排针2.54MM7P                       |
| 8    | 触点                        | BAT,5V,3.3V      | 触点                               |
| 9    | Red/LED                     | LED1,LED3        | 0603_D_JX                          |
| 10   | Green/LED                   | LED2             | 0603_D_JX                          |
| 11   | RU8205C6                    | Q1               | SOT-23-6                           |
| 12   | 0.25R/1%                    | R1               | 1206_R_JX                          |
| 13   | 100                         | R2               | 805                                |
| 14   | 1k                          | R6               | 805                                |
| 15   | 2k/1%                       | R3               | 0603_R_JX                          |
| 16   | 10k/1%                      | R4               | 0603_R_JX                          |
| 17   | 1.2k/1%                     | R5               | 0805-R                             |
| 18   | 1k                          | R7               | R0603                              |
| 19   | TA-3525-A1                  | SW1              | SW-SMD_3P-L9.1-W3.5-P2.50-EH6.8    |
| 20   | DW01+                       | U1               | SOT-23-6-L                         |
| 21   | 3P金手指                    | BAT,U7           | 3P金手指                           |
| 22   | TP4056_JX                   | U3               | SOP8_150MIL_JX                     |
| 23   | ST7789V-12PIN 1.3寸LCD MINI | U4               | ST7789V-12PIN                      |
| 24   | ME6212C33M5G                | U5               | SOT-23-5_L3.0-W1.7-P0.95-LS2.8-BL  |
| 25   | ESP32-WROOM-32              | U6               | WIFIM-SMD_ESP32-WROOM-32-N4        |
| 26   | TYPE-C 6P                   | USB1             | TYPE-C-SMD_TYPE-C-6P               |

------

## 软件

### 单片机端：

```c
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

  WirelessControl();
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
```

```html
const char* WirelessControlTerminal = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>无线控制端</title>
</head>

<body>
    <center>
        <h1>选择一个模式</h1>

        <h2><a href="ManuallySetting.html">手动设置参数拍摄</a></h2>
        <h2><a href="Presets.html">使用预设参数拍摄</a></h2>
        <h2><a href="SetPresets.html">设置预设参数</a></h2>
        <h2><a href="Settings.html">其他设置</a></h2>

        <br>
        <p>ESP32-Shutter-Line</p>
        <p>Designed By SoTWild</p>
    </center>
</body>

</html>
)=====";

const char* ManuallySetting = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>参数设置</title>
</head>

<body>
    <center>
        <h1>请输入相应参数：</h1>
        <p>（时长单位为秒，快门速度小于一秒时正值，输入分母。大于一秒时负值，输入时长）</p>
        <p>（快门速度为0时自动使用机内设置）</p>
        <p>（无需自动对焦输入0，启用自动对焦输入1）</p>
        <form>
            <label for="input1">定时时长：</label>
            <input type="number" id="input1" name="input1"><br>
            <label for="input2">拍摄张数：</label>
            <input type="number" id="input2" name="input2"><br>
            <label for="input3">拍摄间隔：</label>
            <input type="number" id="input3" name="input3"><br>
            <label for="input4">自动对焦：</label>
            <input type="number" id="input4" name="input4"><br>
            <label for="input5">快门速度：</label>
            <input type="number" id="input5" name="input5"><br><br>
            <button type="button" onclick="openURL()">发送数据并开始拍摄</button>
        </form>
    </center>

    <script>
        function openURL() {
            var input1 = document.getElementById("input1").value;
            var input2 = document.getElementById("input2").value;
            var input3 = document.getElementById("input3").value;
            var input4 = document.getElementById("input4").value;
            var input5 = document.getElementById("input5").value;

            // 构建URL，将五个参数作为查询参数
            var url = '/M?input1=' + input1 + '&input2=' + input2 + '&input3=' + input3 + '&input4=' + input4 + '&input5=' + input5;

            // 使用window.location.href在原有窗口中打开URL
            window.location.href = url;
        }
    </script>
</body>

</html>
)=====";

const char* Countdown_1 = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>倒计时</title>
    <style>
        progress {
            width: 200px;
        }

        .hide {
            display: none;
        }
    </style>
</head>

<body>
    <center>
        <h1 id="countdownLabel">预计剩余时间: <span id="countdown"></span></h1>
        <div>
            <progress id="progressBar" value="0" max="100"></progress>
            <span id="progressText"></span>
        </div>
        <div id="endMessage" class="hide">
            <h1>拍摄结束</h1>
            <h1><a href="/">返回</a></h1>
        </div>
    </center>
    <script>
        function startCountdown(duration, displayLabel, display, progressBar, progressText, endMessage) {
            var timer = duration, hours, minutes, seconds;

            var countdownInterval = setInterval(function () {
                hours = parseInt(timer / 3600, 10);
                minutes = parseInt((timer % 3600) / 60, 10);
                seconds = parseInt(timer % 60, 10);

                hours = hours < 10 ? "0" + hours : hours;
                minutes = minutes < 10 ? "0" + minutes : minutes;
                seconds = seconds < 10 ? "0" + seconds : seconds;

                display.textContent = hours + ":" + minutes + ":" + seconds;

                progressBar.value = ((duration - timer) / duration) * 100;

                var percent = ((duration - timer) / duration) * 100;
                progressText.textContent = percent.toFixed(2) + "%";

                if (--timer < 0) {
                    clearInterval(countdownInterval);
                    displayLabel.classList.add("hide");
                    progressBar.style.display = "none";
                    progressText.style.display = "none";
                    endMessage.style.display = "block";
                }
            }, 1000);
        }

        window.onload = function () {
            var countdownTime =
)=====";
const char* Countdown_2 = R"=====(
,
                displayLabel = document.querySelector('#countdownLabel'),
                display = document.querySelector('#countdown'),
                progressBar = document.querySelector('#progressBar'),
                progressText = document.querySelector('#progressText'),
                endMessage = document.querySelector('#endMessage');

            startCountdown(countdownTime, displayLabel, display, progressBar, progressText, endMessage);
        };
    </script>
</body>

</html>
)=====";

const char* Presets_1 = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>使用预设参数拍摄</title>
</head>

<body>
    <center>
        <h1>选择一个预设：</h1>

        <h2>预设A：</h2>
        <p>定时时长：
)=====";
const char* Presets_2 = R"=====(
        <p>拍摄张数：
)=====";
const char* Presets_3 = R"=====(
        <p>拍摄间隔：
)=====";
const char* Presets_4 = R"=====(
        <p>自动对焦：
)=====";
const char* Presets_5 = R"=====(
        <p>快门速度：
)=====";
const char* Presets_6 = R"=====(
                    </p>
        <h3><a href="/PresetsA">使用预设A</a></h3>
        <br>

        <h2>预设B：</h2>
        <p>定时时长：
)=====";
const char* Presets_7 = R"=====(
                    </p>
        <h3><a href="/PresetsB">使用预设B</a></h3>
        <br>

        <h2>预设C：</h2>
        <p>定时时长：
)=====";
const char* Presets_8 = R"=====(
                    </p>
        <h3><a href="/PresetsC">使用预设C</a></h3>
    </center>
</body>

</html>
)=====";

const char* SetPresets_html = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>发送数据至服务器</title>
</head>

<body>
    <center>
        <h1>请输入相应参数进行设置：</h1>
        <p>（时长单位为秒，快门速度小于一秒时正值，输入分母。大于一秒时负值，输入时长）</p>
        <p>（快门速度为0时自动使用机内设置）</p>
        <p>（无需自动对焦输入0，启用自动对焦输入1）</p>
        <form>
            <h3>预设A：</h3>
            <label for="input1-1">定时时长：</label>
            <input type="number" id="input1-1" name="input1-1"><br>
            <label for="input2-1">拍摄张数：</label>
            <input type="number" id="input1-2" name="input1-2"><br>
            <label for="input3-1">拍摄间隔：</label>
            <input type="number" id="input1-3" name="input1-3"><br>
            <label for="input4-1">自动对焦：</label>
            <input type="number" id="input1-4" name="input1-4"><br>
            <label for="input5-1">快门速度：</label>
            <input type="number" id="input1-5" name="input1-5"><br><br>
            <button type="button" onclick="openURL('1')">设置</button>
            <hr>

            <h3>预设B</h3>
            <label for="input2-1">定时时长：</label>
            <input type="number" id="input2-1" name="input2-1"><br>
            <label for="input2-2">拍摄张数：</label>
            <input type="number" id="input2-2" name="input2-2"><br>
            <label for="input2-3">拍摄间隔：</label>
            <input type="number" id="input2-3" name="input2-3"><br>
            <label for="input2-4">自动对焦：</label>
            <input type="number" id="input2-4" name="input2-4"><br>
            <label for="input2-5">快门速度：</label>
            <input type="number" id="input2-5" name="input2-5"><br>
            <button type="button" onclick="openURL('2')">设置</button>
            <hr>

            <h3>预设C</h3>
            <label for="input3-1">定时时长：</label>
            <input type="number" id="input3-1" name="input3-1"><br>
            <label for="input3-2">拍摄张数：</label>
            <input type="number" id="input3-2" name="input3-2"><br>
            <label for="input3-3">拍摄间隔：</label>
            <input type="number" id="input3-3" name="input3-3"><br>
            <label for="input3-4">自动对焦：</label>
            <input type="number" id="input3-4" name="input3-4"><br>
            <label for="input3-5">快门速度：</label>
            <input type="number" id="input3-5" name="input3-5"><br>
            <button type="button" onclick="openURL('3')">设置</button>
            <hr>

            <h2><a href="/ClearPresets" style="color: red;">清空预设</a></h2>
        </form>

        <script>
            function openURL(group) {
                var input1 = document.getElementById("input" + group + "-1").value;
                var input2 = document.getElementById("input" + group + "-2").value;
                var input3 = document.getElementById("input" + group + "-3").value;
                var input4 = document.getElementById("input" + group + "-4").value;
                var input5 = document.getElementById("input" + group + "-5").value;

                // 构建URL，将参数作为查询参数，并添加额外参数group
                var url = '/SetPresets?group=' + group + '&input1=' + input1 + '&input2=' + input2 + '&input3=' + input3 + '&input4=' + input4 + '&input5=' + input5;

                // 使用window.location.href在原有窗口中打开URL 
                window.location.href = url;
            } </script>
    </center>
</body>

</html>
)=====";

const char* Success = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>成功设置</title>
</head>

<body>
    <center>
        <h1>设置成功！</h1>

        <h2><a href="/">返回主页</a></h2>
        <h2><a href="Presets.html">使用预设参数拍摄</a></h2>
    </center>
</body>

</html>
)=====";
```

### 网页端：

> 注：本人缺少 HTML 网页编写经验，便使用了 **ChatGPT 3.5** 协助完成项目。

核心实现原理：通过 ESP32 的 `server.arg()` API 以及网页端的 `查询参数` 进行**网页→ESP32**的通信。

#### 例：

```html
<a href = "/Example?Parameter=1">SET</a>
```

> 客户端效果大概是这样：<a href="/Example?Parameter=1">SET</a>

点击 **SET** 之后会跳转到 `/Example` 下，在 ESP32 端使用 `server.on("/Example", HandleRoot);` 设置对应的回调函数 `HandleRoot()`：

```c
void HandleRoot() {
  int Value = atoi(server.arg("Parameter").c_str());
}
```

此时会获取 `Parameter` 的值，也就是一开始链接中定义的 `1`。如此便实现客户端与 ESP32 的通信。

> 注：这里 `Value` 为 `int` 类型，因此要经过 `String` → `int` 的转换，根据实际情况改变用法。

#### 功能——手动设置参数：

###### 设置数字输入框：

```html
<label for="input">XXX</label>
	<input type="number" id="input" name="input">
```

###### 设置按钮进行查询参数打包和发送：

```html
<button type="button" onclick="openURL()">发送数据</button>
```

###### 查询参数打包并发送函数：

```html
function openURL() {
            var T = document.getElementById("input1").value;
            var N = document.getElementById("input2").value;
            var I = document.getElementById("input3").value;
            var A = document.getElementById("input4").value;
            var S = document.getElementById("input5").value;

            // 构建URL，将五个参数作为查询参数
            var url = '/?T=' + T + '&N=' + N + '&I=' + I + '&A=' + A + '&S=' + S;

            // 使用window.location.href在原有窗口中打开URL
            window.location.href = url;
}
```

#### 功能——设置预设参数：

在查询参数中**添加额外参数** `group` 告诉 ESP32 是哪一组预设即可。

------

## 其他

### 结语：

该项目从**2023年08月02日**开始构思，目前为止基本要完成了，希望该项目能为开源事业添砖加瓦。

在开发过程中，我也吸取到不少经验和教训，这里尽量分享出来，希望能帮助到各位。

###### 总结经验：

> 怎么调试都不成功的话，重新下载库文件再配置一遍会好
>
> LCD的RES最好不要省事练板子RST（EN），单独设置引脚更稳定
>
> 使用SPIFFS请 `include` `FS.h` 以及 `SD.h` ，第一次使用SPIFFS请格式化（-10025）
>
> for函数int i请赋值
>
> 乘除进制单位转换
>
> HTML到cpp的大小写

### Special Thanks：

[ESP8266 – ESP8266WebServer库 – arg – 太极创客 (taichi-maker.com)](http://www.taichi-maker.com/homepage/iot-development/iot-dev-reference/esp8266-c-plus-plus-reference/esp8266webserver/arg/)

[玩转 ESP32 + Arduino(二十八) TFT_eSPI库驱动ST7789(SPI接口) - 简书 (jianshu.com)](https://www.jianshu.com/p/8631a10b5533)

[玩转 ESP32 + Arduino (二十一) SPIFFS文件系统_finedayforu的博客-CSDN博客](https://blog.csdn.net/finedayforu/article/details/108661563)

https://openai.com/chatgpt