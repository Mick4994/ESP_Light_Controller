/*
  author: Mick4994
  description: control light button with SG90 servo and ESP32-C3
*/
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Xiaomi_Mick";
const char* password = "13433780355";

// 查询节假日url
String url = "https://api.apihubs.cn/holiday/get";

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

void request() {
  // 创建 HTTPClient 对象
  HTTPClient http;

  // 发送GET请求
  http.begin(url);

  // 获取响应正文
  String response = http.getString();
  Serial.println("响应数据");
  // Serial.println(response);

  http.end();

  // 创建 DynamicJsonDocument 对象
  DynamicJsonDocument doc(1024);

  // 解析 JSON 数据
  deserializeJson(doc, response);

  // 从解析后的 JSON 文档中获取值
  int code = doc["code"].as<int>();

  if(code == 0) {
    
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
      return;
    }
    Serial.println(&timeinfo, "%F %T %A"); // 格式化输出
}

void setup() {
  init_servo();
  init_network();
  // request();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop() {

}