#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 摄像头配置
#define CAMERA_MODEL_AI_THINKER
#define LED_PIN 4  // ESP32-CAM 上板载 LED 通常连接到 GPIO 4 引脚

bool send_flag;

// Wi-Fi 配置
const char* ssid = "ESP_Moto";  // 替换为你的 Wi-Fi SSID
const char* password = "00000000";  // 替换为你的 Wi-Fi 密码
// 设置固定 IP 地址
IPAddress local_IP(192, 168, 1, 1);  // 固定 IP 地址
IPAddress gateway(192, 168, 1, 1);   // 网关（一般为同一个地址）
IPAddress subnet(255, 255, 255, 0);  // 子网掩码
// 创建 Web 服务器
WebServer server(80);

// 摄像头引脚设置
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

#define LED_GPIO_NUM       4  // ESP32 上板载 LED 引脚

// 摄像头初始化函数
esp_err_t initializeCamera() {
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

  config.frame_size = FRAMESIZE_QVGA;  // 设置为 320x240 分辨率
  config.pixel_format = PIXFORMAT_JPEG;  // 使用 JPEG 格式
  config.jpeg_quality = 10;  // JPEG 质量较低，数值越小质量越差，帧率越高
  config.xclk_freq_hz = 20000000;  // 设置 20 MHz 的时钟频率
  config.fb_count = 1;  // 设置缓冲区数量

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("摄像头初始化失败，错误代码：%s\n", esp_err_to_name(err));
    return err;
  }

  return ESP_OK;
}

uint8_t data[9];  // 定义数据缓冲区
// 定义包头和包尾
#define PACKET_HEADER1 0xA5  // 第一个包头
#define PACKET_HEADER2 0x11  // 第二个包头
#define PACKET_FOOTER 0x5A   // 包尾

// 发送数据包的函数
void sendData(uint8_t data[], size_t length) {

    // 发送第一个包头
    Serial.write(PACKET_HEADER1);

    // 发送第二个包头
    Serial.write(PACKET_HEADER2);

    // 发送数据并计算校验和
    for (int i = 0; i < length; i++) {
        Serial.write(data[i]);  // 发送字节
    }

    // 发送包尾
    Serial.write(PACKET_FOOTER);
}

// Wi-Fi 初始化函数
void initializeWiFi() {

 int maxConnections = 1;  // 设置最大连接数，默认为 4，可以设置为 5 或更高

  // 配置为 AP 模式，并设置最大连接数
  WiFi.softAPConfig(local_IP, gateway, subnet);  // 设置固定 IP 地址、网关和子网掩码
  WiFi.softAP(ssid, password, 1, false, maxConnections);  // 启动 AP 模式，最后一个参数设置最大连接数
  String MyIP = WiFi.softAPIP().toString();
  Serial.print("[IP:");

  Serial.println("]");


//   WiFi.begin("KeleYYDS", "00000000");
//   Serial.print("等待连接 Wi-Fi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\n连接成功");
//   Serial.print("IP 地址：");
//   String MyIP = WiFi.localIP().toString();
//    Serial.print(MyIP);

}

// ESP32 端代码示例（处理模式切换）
String currentMode = "履带模式"; // 默认模式

void handleSwitchMode() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "履带模式") {
      currentMode = "履带模式";
      data[0] = 0;
      sendData(data, 8);
      Serial.println("履带模式切换成功");
    } else if (mode == "四足模式") {
      currentMode = "四足模式";

      data[0] = 1;
      sendData(data, 8);
      Serial.println("四足模式切换成功");
    }
  }
  server.send(200, "text/plain", currentMode);
}

// 网页初始化函数
void initializeWebServer() {
  // 加载 HTML 页面
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Failed to load HTML");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/favicon.ico", HTTP_GET, []() {
    // 可以返回一个有效的响应，或者直接忽略
    server.send(200, "image/x-icon", "");  // 返回空的 favicon 响应
  });

  // 处理 404 错误：返回主页
server.onNotFound([]() {
    Serial.println("收到未知请求: " + server.uri());
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to load HTML");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
});



  
  // 视频流接口
  server.on("/video", HTTP_GET, []() {
    
    WiFiClient client = server.client();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println();

    while (client.connected()) {
      camera_fb_t *fb = esp_camera_fb_get();

      if (!fb) {
        Serial.println("获取帧失败");
        client.stop();
        return;
      }

        // 检查客户端是否连接
      if (!client.connected()) {
        Serial.println("用户退出网页");
        esp_camera_fb_return(fb);  // 释放帧缓冲区
        break;  // 客户端已断开连接，退出循环
      }

      client.println("--frame");
      client.println("Content-Type: image/jpeg");
      client.println("Content-Length: " + String(fb->len));
      client.println();
      client.write(fb->buf, fb->len);
      client.println();

      esp_camera_fb_return(fb);
    }

  client.stop();  // 退出视频流后，关闭连接

  });

  // 控制小车运动
  server.on("/move", HTTP_GET, []() {
    String direction = server.arg("dir");

  
    // 解析参数中的滑块编号和滑块值
    int separatorIndex = direction.indexOf(':');
    if (separatorIndex != -1) {
      String sliderName = direction.substring(0, separatorIndex);  // 例如 slider1
      int sliderValue = direction.substring(separatorIndex + 1).toInt();  // 例如 100
        // 根据滑块值进行相应操作（例如控制电机等）
      if (sliderName == "slider1") {
        // 控制 slider1 相关功能
        // 例如：控制电机速度或其他参数
        Serial.print("控制滑块1，值为：");
        Serial.println(sliderValue);
        data[3] = sliderValue;
      } 
      else if (sliderName == "slider2") {
        // 控制 slider2 相关功能
        Serial.print("控制滑块2，值为：");
        Serial.println(sliderValue);
        data[4] = sliderValue;
      }
    }

    if (direction == "forward") {
      data[1] = 1;
      if(data[0] == 1){ //如果是四足模式，每次遥控都打断动作
        data[2] = 0;   
      }
    } else if (direction == "backward") {
      data[1] = 2;
      if(data[0] == 1){
        data[2] = 0;   
      }
    } else if (direction == "left") {
      data[1] = 3;
      if(data[0] == 1){
        data[2] = 0;   
      }
    } else if (direction == "right") {
      data[1] = 4;
      if(data[0] == 1){
        data[2] = 0;   
      }
    } else if (direction == "pause") {
      data[1] = 0;
      if(data[0] == 1){
        data[2] = 0;   
      }
    } else if (direction == "action1") {
      data[1] = 5;
      data[2] = 1;
    } else if (direction == "action2") {
      data[1] = 5;
      data[2] = 2;
    } else if (direction == "action3") {
      data[1] = 5;
      data[2] = 3;
    } else if (direction == "action4") {
      data[1] = 5;
      data[2] = 4;
    } else if (direction == "action5") {
      data[1] = 5;
      data[2] = 5;
    } else if (direction == "action6") {
      data[1] = 5;
      data[2] = 6;
    } else if (direction == "led_on") {
      // 打开 LED
      digitalWrite(LED_GPIO_NUM, HIGH);
      Serial.println("LED 打开");
    } else if (direction == "led_off") {
      // 关闭 LED
      digitalWrite(LED_GPIO_NUM, LOW);
      Serial.println("LED 关闭");
    } else if (direction == "mode_SZU") {
      data[0] = 1;
      data[1] = 0;
      data[2] = 0;
    }  else if (direction == "mode_LUDAI") {
      data[0] = 0;
      if(data[2] == 0){
        data[2] = 2;
      }
    } 

    

    send_flag = 1;

    server.send(200, "text/plain", "Movement: " + direction);
  });

    

  // 切换模式接口
  server.on("/switchMode", HTTP_GET, handleSwitchMode);

  // 启动服务器
  server.begin();
}

// 视频流任务
void videoStreamTask(void *pvParameters) {
  while (true) {
    server.handleClient();  // 处理视频流请求
    vTaskDelay(10 / portTICK_PERIOD_MS);  // 给其他任务留出时间
  }
}

// 按钮控制任务
void buttonControlTask(void *pvParameters) {
  while (true) {
    // 处理按钮控制请求
    server.handleClient();  // 处理按钮控制指令
    vTaskDelay(2 / portTICK_PERIOD_MS);  // 给其他任务留出时间
  }
}

//定时清除标志位任务
void clear_task(void *pvParameters) {
  while (true) {
    if(data[2] != 0){
      vTaskDelay(1500 / portTICK_PERIOD_MS);  // 给其他任务留出时间
      data[2] = 0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // 给其他任务留出时间
  }
}

void setup() {
  // 设置串口波特率为 9600
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // 设置 LED 引脚为输出模式
  pinMode(33, OUTPUT); // 设置板载LED 为输出模式
  digitalWrite(33, LOW); // 默认打开板载LED
  digitalWrite(LED_PIN, LOW); // 默认关闭 LED
  data[3] = 180;
  data[4] = 90;

  // 初始化 SPIFFS 文件系统
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS 初始化失败");
    return;
  }

  // 初始化 Wi-Fi
  initializeWiFi();

  // 初始化摄像头
  esp_err_t err = initializeCamera();
  if (err != ESP_OK) {
    return;
  }

  // 初始化 Web 服务器
  initializeWebServer();
  
  // 创建视频流任务
  xTaskCreatePinnedToCore(
    videoStreamTask,  // 任务函数
    "videoStreamTask",  // 任务名称
    1024*8,  // 堆栈大小
    NULL,  // 参数
    1,  // 优先级
    NULL,  // 任务句柄
    1  // 核心 1 运行任务
  );

  //创建按钮控制任务
  xTaskCreatePinnedToCore(
    buttonControlTask,  // 任务函数
    "buttonControlTask",  // 任务名称
    1024*8,  // 堆栈大小
    NULL,  // 参数
    1,  // 优先级
    NULL,  // 任务句柄
    0  // 核心 0 运行任务
  );
  // //定时清除标志位任务
  // xTaskCreatePinnedToCore(
  //   clear_task,  // 任务函数
  //   "clear_flag_task",  // 任务名称
  //   1024*8,  // 堆栈大小
  //   NULL,  // 参数
  //   1,  // 优先级
  //   NULL,  // 任务句柄
  //   0  // 核心 0 运行任务
  // );
}

void loop() {
  sendData(data, 8);

  vTaskDelay(100 / portTICK_PERIOD_MS);  // 给其他任务留出时间
}
