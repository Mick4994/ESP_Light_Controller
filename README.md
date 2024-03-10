# 控制出租屋内灯非节假日自动开关

IDE：vscode platformIO Arduino

硬件 ESP32-C3

## 安装部署
软件：
- 工作目录下运行：`git clone https://github.com/Mick4994/ESP_Light_Controller.git`
- 仓库内含`platformio.ini`配置文件，自行搜索 vscode platformIO 配置教程

硬件：
- ESP32-C3 和 SG90舵机
- SG90中间vcc插开发版5v，信号线黄色，接GPIO口，棕色地线

## 重要参数
均在`main.cpp`中修改 
- 默认参数 (影响程序逻辑，非必要不更改)

|参数名|默认值|描述|
|-------|-------|-------|
|is_workday|0|0为非工作日，1为工作日|
|url|https://api.apihubs.cn/holiday/get|查询节假日url，每日限制5次访问<br>详情见: [接口坞](http://www.apihubs.cn/#/apiList)|
|work_state|OFFLINE|程序工作状态，默认未联网<br>更多状态查看OFFLINE注释|
|LOOP_TIME|30|程序循环延时时间：单位s，可改范围12-59|


- 部署时根据自身情况调整的参数

|参数名|描述|
|-------|-------|
|servo_t|舵机转动时间ms（影响转动角度）|
|ssid|你的路由器Wi-Fi网络ssid|
|password|Wi-Fi对应的密码|
|open_schedules|开灯时间表，数字规则：小时 * 100 + 分钟|
|close_schedules|关灯时间表，数字规则同上|
|BRIGHTNESS|亮度|

## 指示灯状态：
- 红色：联网失败，检查ssid和密码是否正确
- 蓝色：非工作日不开启自动开关
- 绿色：正常工作日工作状态
- 黄色：非正常工作，如获取时间失败，获取工作日失败