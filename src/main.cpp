/*
  author: Mick4994
  description: control light button with SG90 servo and ESP32-C3
*/
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 8         // 板载RGB灯珠的引脚，根据实际使用的开发板型号而定
#define LED_COUNT 1         // LED灯条的灯珠数量（板载的是一颗）

// 0：未联网， -1：非正确工作状态（请求失败，获取时间失败），1：正常运行， 2：非工作日
#define OFFLINE 0
#define ERROR -1
#define NORMAL 1
#define NOT_WORKDAY 2

/*
使用 Adafruit_NeoPixel 库创建了一个名为 strip 的对象，控制LED灯珠
LED_COUNT 表示 LED 条上的 LED 数量，LED_PIN 表示连接到 Arduino 的引脚，NEO_GRB + NEO_KHZ800 用于设置 LED 条的颜色排列和通信速率
*/ 
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// 程序工作状态，
int work_state = OFFLINE;

const char* ssid = "Xiaomi_Mick";
const char* password = "13433780355";

// 查询节假日url
String url = "https://api.apihubs.cn/holiday/get";

// 时间表
int open_schedules[] = {748};
int close_schedules[] = {100, 805};
int is_workday = 1;

//延时：ms
int t = 150; 

//使用3号通道 定时器1  总共16个通道 
int channel_PWM = 3;  

// 舵机频率，那么周期也就是1/50，也就是20ms ，PWM⼀共有16个通道，0-7位⾼速通道由80Mhz时钟驱动，后⾯8个为低速通道由1Mhz
int freq_PWM = 50;  //50HZ pwm波

// PWM分辨率，取值为 0-20 之间  ，这⾥填写为10，那么后⾯的ledcWrite 这个⾥⾯填写的pwm值就在 0 - 2的10次⽅ 之间 也就是 0-1024，如果是要求不⾼的东西你可以直接拿1000去算了
int res_PWM = 10;  //分辨率  0-1024  共1025

//使用GPIO 9号引脚
const int  PWM_PIN = 9; 

// 连接网络
void init_network() {
  Serial.begin(115200);

  // 连接 WiFi
  WiFi.begin(ssid, password);

  Serial.print("正在连接 Wi-Fi");
  
  // 检测是否连接成功
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

  Serial.println("连接成功");
  Serial.print("IP 地址：");
  Serial.println(WiFi.localIP());
  work_state = NORMAL;
}

void open_light() {
  // 向下按
  ledcWrite(channel_PWM, 102);
  delay(t);
  // 停止
  ledcWrite(channel_PWM, 77);
}

void close_light() {
  // 向上按
  ledcWrite(channel_PWM, 52);
  delay(t);
  // 停止
  ledcWrite(channel_PWM, 77);
}

void init_servo() {
  //设置通道
  ledcSetup(channel_PWM,freq_PWM,res_PWM);  
  //将引脚绑定到通道上
  ledcAttachPin(PWM_PIN,channel_PWM);  
}

int getMonthDay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    work_state = -1;
    return 0;
  }
  return timeinfo.tm_mday;
}

void requestIsWorkday() {
  // 创建 HTTPClient 对象
  HTTPClient http;

  // 发送GET请求
  http.begin(url);

  int httpCode = http.GET();

  if(httpCode <= 0) {
    work_state = ERROR;
    Serial.println("Request Failed");
    return;
  }

  if(httpCode != HTTP_CODE_OK) {
    work_state = ERROR;
    Serial.println("Request Failed");
    return;
  }

  // 获取响应正文
  String response = http.getString();
  // Serial.println("响应数据");
  // Serial.println(response);

  http.end();

  // 创建 DynamicJsonDocument 对象
  DynamicJsonDocument doc(16384);

  // 解析 JSON 数据
  deserializeJson(doc, response);

  // 从解析后的 JSON 文档中获取值
  int code = doc["code"].as<int>();

  if(code != 0) {
    work_state = ERROR;
    Serial.println("Request Failed");
    return;
  }

  int mday = getMonthDay();

  if(!mday) {
    Serial.println("getMonthDay Failed");
    return;
  }

  // 解析 json 对象到是否是工作日字段
  JsonArray months = doc["data"]["list"];
  int total = doc["data"]["total"];

  int index = total - mday;

  Serial.printf("api's index: %d\n", index);
  JsonObject current_day = months[index];

  int today = current_day["workday"];
  int date = current_day["date"];

  Serial.printf("api's date: %d\n", date);

  if(today == 1) {
    work_state = NORMAL;
    Serial.println("is workday");
    is_workday = 1;
  } else {
    work_state = NOT_WORKDAY;
    Serial.println("Not workday");
    is_workday = 0;
  }
}

const char *ntpServer = "ntp.aliyun.com";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    work_state = ERROR;
    return;
  }
  Serial.println(&timeinfo, "%F %T %A"); // 格式化输出
}

int getHourMinu() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return -1;
  }
  return timeinfo.tm_hour * 100 + timeinfo.tm_min;
}

void Scheduler() {
  int current_time = getHourMinu();

  // 每天0点查今日是否工作日
  if(current_time == 0) {
    requestIsWorkday();
  }

  // 工作日再开启
  if(is_workday) {
    for(int schedule : close_schedules) {
      if(schedule == current_time) {
        close_light();
      }
    }
    for(int schedule : open_schedules) {
      if(schedule == current_time) {
        open_light();
      }
    }
  }
}

void LED_WorkState() {
  if(work_state) {
    switch (work_state) {
      case NORMAL:
        // 设置灯珠为绿色 (正确的工作状态)
        strip.setPixelColor(0, strip.Color(0, 255, 0)); 
        break;
      case NOT_WORKDAY:
        // 设置灯珠为蓝色 (非工作日)
        strip.setPixelColor(0, strip.Color(0, 0, 255)); 
        break;
      default:
        // 设置灯珠为黄色 (未处于正确的工作状态)
        strip.setPixelColor(0, strip.Color(255, 255, 0)); 
        break;
    }
  } else {
    // 设置灯珠为红色 (未联网状态)
    strip.setPixelColor(0, strip.Color(255, 255, 0)); 
  }
  strip.show(); // 显示颜色
}

void setup() {
  init_servo();
  init_network();
  
  // 板载 LED灯 初始化
  strip.begin();
  strip.setBrightness(50); // 设置亮度（0-255范围）

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  requestIsWorkday();
  printLocalTime();
}

void loop() {

  // 联网时进行
  if(work_state) {
    Scheduler();
    LED_WorkState();
  }

  delay(30 * 1000);

  // Wi-Fi 断联自动重连
  if(WiFi.status() != WL_CONNECTED) {
    work_state = OFFLINE;
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
  }
}