/*
 * Stack-chan Communication Edition
 * WiFi WebServer + Bluetooth + Avatar Display System
 * 
 * @author based on TakaoAkaki's stack-chan-tester
 * Copyright (c) 2024. All right reserved
 */

// ------------------------
// ヘッダファイルのinclude
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <M5Unified.h>
#include <Avatar.h>
#include <SD.h>
#include <Stackchan_system_config.h>
#include "formatString.hpp"

// ------------------------
// グローバル変数定義
using namespace m5avatar;
Avatar avatar;
ColorPalette *cps[2];

// WiFi & WebServer
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
WebServer server(80);

// Bluetooth
BluetoothSerial SerialBT;

// システム設定
StackchanSystemConfig system_config;

// 表示制御
uint32_t display_update_interval = 2000;
uint32_t last_display_update = 0;
String current_message = "待機中...";
String last_received_data = "";
bool bluetooth_connected = false;

// 状態管理
enum CommunicationMode {
  WIFI_MODE,
  BLUETOOTH_MODE,
  BOTH_MODE
};
CommunicationMode current_mode = BOTH_MODE;

// ------------------------
// WiFi WebServer API ハンドラー
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'><title>Stack-chan Communication</title>";
  html += "<style>body { font-family: Arial, sans-serif; margin: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; }";
  html += ".button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; margin: 5px; }";
  html += ".input-group { margin: 10px 0; }";
  html += "label { display: block; margin-bottom: 5px; }";
  html += "input, textarea { width: 100%; padding: 8px; border-radius: 4px; border: 1px solid #ddd; }</style></head><body>";
  html += "<div class='container'><h1>Stack-chan Communication Panel</h1>";
  html += "<div class='input-group'><label for='message'>Message:</label>";
  html += "<textarea id='message' rows='3' placeholder='Enter message to display'></textarea>";
  html += "<button class='button' onclick='sendMessage()'>Send</button></div>";
  html += "<div class='input-group'><label for='expression'>Expression:</label>";
  html += "<select id='expression'><option value='normal'>Normal</option><option value='happy'>Happy</option>";
  html += "<option value='sleepy'>Sleepy</option><option value='doubt'>Doubt</option></select>";
  html += "<button class='button' onclick='changeExpression()'>Change</button></div>";
  html += "<h3>System Status:</h3><div id='status'>Loading...</div></div>";
  html += "<script>function sendMessage() { const message = document.getElementById('message').value;";
  html += "fetch('/api/message', { method: 'POST', headers: {'Content-Type': 'application/json'},";
  html += "body: JSON.stringify({message: message}) }).then(response => response.json())";
  html += ".then(data => { alert('Message sent: ' + data.status); document.getElementById('message').value = ''; updateStatus(); }); }";
  html += "function changeExpression() { const expression = document.getElementById('expression').value;";
  html += "fetch('/api/expression', { method: 'POST', headers: {'Content-Type': 'application/json'},";
  html += "body: JSON.stringify({expression: expression}) }).then(response => response.json())";
  html += ".then(data => { alert('Expression changed: ' + data.status); updateStatus(); }); }";
  html += "function updateStatus() { fetch('/api/status').then(response => response.json()).then(data => {";
  html += "document.getElementById('status').innerHTML = '<p><strong>Current Message:</strong> ' + data.current_message + '</p>' +";
  html += "'<p><strong>Last Received:</strong> ' + data.last_received + '</p>' +";
  html += "'<p><strong>Bluetooth:</strong> ' + (data.bluetooth_connected ? 'Connected' : 'Disconnected') + '</p>' +";
  html += "'<p><strong>WiFi:</strong> ' + (data.wifi_connected ? 'Connected' : 'Disconnected') + '</p>' +";
  html += "'<p><strong>IP Address:</strong> ' + data.ip_address + '</p>'; }); }";
  html += "updateStatus(); setInterval(updateStatus, 5000);</script></body></html>";
  server.send(200, "text/html", html);
}

void handleMessage() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String message = doc["message"];
  current_message = message;
  avatar.setSpeechText(message.c_str());
  last_received_data = "WiFi: " + message;
  
  M5_LOGI("WiFi Message received: %s", message.c_str());
  
  server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Message received\"}");
}

void handleExpression() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String expression = doc["expression"];
  
  // 表情変更
  if (expression == "happy") {
    avatar.setExpression(Expression::Happy);
  } else if (expression == "sleepy") {
    avatar.setExpression(Expression::Sleepy);
  } else if (expression == "doubt") {
    avatar.setExpression(Expression::Doubt);
  } else {
    avatar.setExpression(Expression::Neutral);
  }
  
  M5_LOGI("Expression changed to: %s", expression.c_str());
  
  server.send(200, "application/json", "{\"status\":\"success\",\"expression\":\"" + expression + "\"}");
}

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["current_message"] = current_message;
  doc["last_received"] = last_received_data;
  doc["bluetooth_connected"] = bluetooth_connected;
  doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["ip_address"] = WiFi.localIP().toString();
  doc["mode"] = current_mode;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// ------------------------
// Bluetooth通信処理
void handleBluetoothData() {
  if (SerialBT.available()) {
    String receivedData = SerialBT.readString();
    receivedData.trim();
    
    if (receivedData.length() > 0) {
      M5_LOGI("Bluetooth data received: %s", receivedData.c_str());
      
      // JSON形式のデータかチェック
      if (receivedData.startsWith("{")) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, receivedData);
        
        if (!error) {
          // JSON形式の場合
          if (doc.containsKey("message")) {
            current_message = doc["message"].as<String>();
            avatar.setSpeechText(current_message.c_str());
          }
          if (doc.containsKey("expression")) {
            String expression = doc["expression"];
            if (expression == "happy") {
              avatar.setExpression(Expression::Happy);
            } else if (expression == "sleepy") {
              avatar.setExpression(Expression::Sleepy);
            } else if (expression == "doubt") {
              avatar.setExpression(Expression::Doubt);
            } else {
              avatar.setExpression(Expression::Neutral);
            }
          }
        } else {
          // JSON解析失敗時はそのまま表示
          current_message = receivedData;
          avatar.setSpeechText(current_message.c_str());
        }
      } else {
        // プレーンテキストの場合
        current_message = receivedData;
        avatar.setSpeechText(current_message.c_str());
      }
      
      last_received_data = "BT: " + receivedData;
      
      // 受信確認をBluetoothで返送
      SerialBT.println("ACK: " + receivedData);
    }
  }
}

void bluetoothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    bluetooth_connected = true;
    current_message = "Bluetooth接続しました";
    avatar.setSpeechText(current_message.c_str());
    M5_LOGI("Bluetooth connected");
  } else if (event == ESP_SPP_CLOSE_EVT) {
    bluetooth_connected = false;
    current_message = "Bluetooth切断されました";
    avatar.setSpeechText(current_message.c_str());
    M5_LOGI("Bluetooth disconnected");
  }
}

// ------------------------
// 初期化とメイン処理
void setupWiFi() {
  WiFi.begin(ssid, password);
  current_message = "WiFi接続中...";
  avatar.setSpeechText(current_message.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    M5.Lcd.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    current_message = "WiFi接続成功: " + WiFi.localIP().toString();
    M5_LOGI("WiFi connected! IP: %s", WiFi.localIP().toString().c_str());
  } else {
    current_message = "WiFi接続失敗";
    M5_LOGI("WiFi connection failed");
  }
  avatar.setSpeechText(current_message.c_str());
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/message", HTTP_POST, handleMessage);
  server.on("/api/expression", HTTP_POST, handleExpression);
  server.on("/api/status", HTTP_GET, handleStatus);
  
  server.enableCORS(true);
  server.begin();
  M5_LOGI("Web server started");
}

void setupBluetooth() {
  if (!SerialBT.begin("StackChan-Comm")) {
    M5_LOGE("Bluetooth initialization failed!");
    return;
  }
  
  SerialBT.register_callback(bluetoothCallback);
  current_message = "Bluetooth待機中...";
  avatar.setSpeechText(current_message.c_str());
  M5_LOGI("Bluetooth initialized: StackChan-Comm");
}

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  M5.setTouchButtonHeight(40);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5_LOGI("Stack-chan Communication Edition Started");
  
  // SDカード初期化
  SD.begin(GPIO_NUM_4, SPI, 25000000);
  delay(1000);
  
  // 設定ファイル読み込み
  system_config.loadConfig(SD, "");
  
  // Avatar初期化
  avatar.init();
  cps[0] = new ColorPalette();
  cps[0]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[0]->set(COLOR_BACKGROUND, TFT_WHITE);
  avatar.setColorPalette(*cps[0]);
  
  // バッテリーアイコン設定
  if (M5.getBoard() != m5::board_t::board_M5Stack) {
    avatar.setBatteryIcon(true);
  }
  
  current_message = "システム初期化中...";
  avatar.setSpeechText(current_message.c_str());
  delay(2000);
  
  // WiFi設定
  setupWiFi();
  delay(2000);
  
  // WebServer設定
  setupWebServer();
  
  // Bluetooth設定
  setupBluetooth();
  
  current_message = "準備完了！";
  avatar.setSpeechText(current_message.c_str());
  
  last_display_update = millis();
  
  M5_LOGI("Setup completed");
}

void loop() {
  M5.update();
  
  // Webサーバー処理
  server.handleClient();
  
  // Bluetooth通信処理
  handleBluetoothData();
  
  // ボタン処理
  if (M5.BtnA.wasPressed()) {
    // モード切替
    switch (current_mode) {
      case WIFI_MODE:
        current_mode = BLUETOOTH_MODE;
        current_message = "Bluetoothモード";
        break;
      case BLUETOOTH_MODE:
        current_mode = BOTH_MODE;
        current_message = "両方モード";
        break;
      case BOTH_MODE:
        current_mode = WIFI_MODE;
        current_message = "WiFiモード";
        break;
    }
    avatar.setSpeechText(current_message.c_str());
    M5_LOGI("Mode changed to: %d", current_mode);
  }
  
  if (M5.BtnB.wasPressed()) {
    // ステータス表示
    String status = formatString("IP:%s BT:%s", 
                                WiFi.localIP().toString().c_str(),
                                bluetooth_connected ? "接続" : "切断");
    current_message = status;
    avatar.setSpeechText(current_message.c_str());
  }
  
  if (M5.BtnC.wasPressed()) {
    // 表情をランダムに変更
    Expression expressions[] = {Expression::Neutral, Expression::Happy, 
                               Expression::Sleepy, Expression::Doubt};
    int randomIndex = random(0, 4);
    avatar.setExpression(expressions[randomIndex]);
    current_message = "表情変更！";
    avatar.setSpeechText(current_message.c_str());
  }
  
  // 定期的な表示更新
  if ((millis() - last_display_update) > display_update_interval) {
    // バッテリー状態更新
    if (M5.getBoard() != m5::board_t::board_M5Stack) {
      avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
    }
    
    // 口の動き
    avatar.setMouthOpenRatio(0.5);
    delay(100);
    avatar.setMouthOpenRatio(0.0);
    
    last_display_update = millis();
  }
  
  delay(50); // CPU負荷軽減
}
