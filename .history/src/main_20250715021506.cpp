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
#include "communication_config.h"
#include "wifi_manager.h"

// 日本語フォント用
#include <M5GFX.h>

// UTF-8エンコーディング指定
#pragma GCC diagnostic ignored "-Wwrite-strings"

// ------------------------
// グローバル変数定義
using namespace m5avatar;
Avatar avatar;
ColorPalette *cps[2];

// WiFi & WebServer
CommunicationConfig comm_config;
WiFiManager* wifi_manager;
WebServer* server = nullptr;

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

// ランダム日本語メッセージ
int lyrics_idx = 0;

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
  html += "<meta charset='UTF-8'><title>スタックチャン 通信パネル</title>";
  html += "<style>body { font-family: 'Hiragino Sans', 'ヒラギノ角ゴ ProN W3', sans-serif; margin: 20px; background: #f5f5f5; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += ".header { text-align: center; margin-bottom: 30px; }";
  html += ".button { background-color: #4CAF50; color: white; padding: 12px 24px; border: none; border-radius: 6px; cursor: pointer; margin: 5px; font-size: 16px; }";
  html += ".button:hover { background-color: #45a049; }";
  html += ".button.secondary { background-color: #2196F3; }";
  html += ".button.danger { background-color: #f44336; }";
  html += ".input-group { margin: 15px 0; }";
  html += "label { display: block; margin-bottom: 8px; font-weight: bold; }";
  html += "input, textarea, select { width: 100%; padding: 12px; border-radius: 6px; border: 1px solid #ddd; font-size: 16px; }";
  html += ".status-panel { background: #e8f5e8; padding: 15px; border-radius: 6px; margin: 20px 0; }";
  html += ".wifi-panel { background: #e3f2fd; padding: 15px; border-radius: 6px; margin: 20px 0; }";
  html += ".network-list { max-height: 200px; overflow-y: auto; border: 1px solid #ddd; border-radius: 6px; }";
  html += ".network-item { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; }";
  html += ".network-item:hover { background: #f0f0f0; }";
  html += ".network-item.connected { background: #c8e6c9; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<div class='header'><h1>スタックチャン 通信パネル</h1></div>";
  
  // メッセージ送信パネル
  html += "<div class='input-group'><label for='message'>メッセージ送信:</label>";
  html += "<textarea id='message' rows='3' placeholder='表示したいメッセージを入力してください'></textarea>";
  html += "<button class='button' onclick='sendMessage()'>送信</button></div>";
  
  // 表情変更パネル
  html += "<div class='input-group'><label for='expression'>表情変更:</label>";
  html += "<select id='expression'><option value='normal'>普通</option><option value='happy'>嬉しい</option>";
  html += "<option value='sleepy'>眠い</option><option value='doubt'>疑問</option></select>";
  html += "<button class='button' onclick='changeExpression()'>表情変更</button></div>";
  
  // WiFi管理パネル
  html += "<div class='wifi-panel'><h3>WiFi管理</h3>";
  html += "<button class='button secondary' onclick='scanNetworks()'>ネットワークスキャン</button>";
  html += "<button class='button secondary' onclick='toggleWiFiMode()'>WiFiモード切替</button>";
  html += "<div id='networkList' class='network-list'></div>";
  html += "<div class='input-group'><label for='newSSID'>新しいネットワーク追加:</label>";
  html += "<input type='text' id='newSSID' placeholder='SSID'>";
  html += "<input type='password' id='newPassword' placeholder='パスワード'>";
  html += "<button class='button' onclick='addNetwork()'>追加</button></div></div>";
  
  // ステータスパネル
  html += "<div class='status-panel'><h3>システム状態</h3><div id='status'>読み込み中...</div></div>";
  
  html += "</div>";
  
  // JavaScript
  html += "<script>";
  html += "function sendMessage() { const message = document.getElementById('message').value;";
  html += "fetch('/api/message', { method: 'POST', headers: {'Content-Type': 'application/json'},";
  html += "body: JSON.stringify({message: message}) }).then(response => response.json())";
  html += ".then(data => { alert('メッセージを送信しました: ' + data.status); document.getElementById('message').value = ''; updateStatus(); }); }";
  
  html += "function changeExpression() { const expression = document.getElementById('expression').value;";
  html += "fetch('/api/expression', { method: 'POST', headers: {'Content-Type': 'application/json'},";
  html += "body: JSON.stringify({expression: expression}) }).then(response => response.json())";
  html += ".then(data => { alert('表情を変更しました: ' + data.status); updateStatus(); }); }";
  
  html += "function scanNetworks() { fetch('/api/wifi/scan').then(response => response.json()).then(data => { updateNetworkList(data.networks); }); }";
  
  html += "function toggleWiFiMode() { fetch('/api/wifi/toggle', {method: 'POST'}).then(response => response.json()).then(data => { alert('WiFiモードを切り替えました: ' + data.mode); updateStatus(); }); }";
  
  html += "function addNetwork() { const ssid = document.getElementById('newSSID').value; const password = document.getElementById('newPassword').value;";
  html += "if(!ssid) { alert('SSIDを入力してください'); return; }";
  html += "fetch('/api/wifi/add', { method: 'POST', headers: {'Content-Type': 'application/json'},";
  html += "body: JSON.stringify({ssid: ssid, password: password}) }).then(response => response.json())";
  html += ".then(data => { alert('ネットワークを追加しました'); document.getElementById('newSSID').value = ''; document.getElementById('newPassword').value = ''; scanNetworks(); }); }";
  
  html += "function connectToNetwork(ssid) { fetch('/api/wifi/connect', { method: 'POST', headers: {'Content-Type': 'application/json'},";
  html += "body: JSON.stringify({ssid: ssid}) }).then(response => response.json()).then(data => { alert('接続を試行中: ' + ssid); updateStatus(); }); }";
  
  html += "function updateNetworkList(networks) { const listEl = document.getElementById('networkList'); listEl.innerHTML = '';";
  html += "networks.forEach(net => { const div = document.createElement('div'); div.className = 'network-item' + (net.connected ? ' connected' : '');";
  html += "div.innerHTML = net.ssid + (net.connected ? ' (接続中)' : ''); div.onclick = () => connectToNetwork(net.ssid); listEl.appendChild(div); }); }";
  
  html += "function updateStatus() { fetch('/api/status').then(response => response.json()).then(data => {";
  html += "document.getElementById('status').innerHTML = '<p><strong>現在のメッセージ:</strong> ' + data.current_message + '</p>' +";
  html += "'<p><strong>最後の受信:</strong> ' + data.last_received + '</p>' +";
  html += "'<p><strong>Bluetooth:</strong> ' + (data.bluetooth_connected ? '接続済み' : '未接続') + '</p>' +";
  html += "'<p><strong>WiFi状態:</strong> ' + data.wifi_status + '</p>' +";
  html += "'<p><strong>IPアドレス:</strong> ' + data.ip_address + '</p>' +";
  html += "'<p><strong>接続クライアント数:</strong> ' + data.connected_clients + '</p>'; }); }";
  
  html += "updateStatus(); scanNetworks(); setInterval(updateStatus, 5000);</script></body></html>";
  server->send(200, "text/html", html);
}

void handleMessage() {
  if (server->method() != HTTP_POST) {
    server->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server->arg("plain"));
  
  if (error) {
    server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String message = doc["message"];
  current_message = message;
  avatar.setSpeechText(message.c_str());
  last_received_data = "WiFi: " + message;
  
  // シンプルな表示（英語フォント使用）
  M5.Display.fillRect(0, M5.Display.height() - 40, M5.Display.width(), 40, TFT_BLACK);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(5, M5.Display.height() - 38);
  M5.Display.print("WiFi RX:");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(5, M5.Display.height() - 18);
  M5.Display.print(message);
  
  M5_LOGI("WiFi Message received: %s", message.c_str());
  
  server->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Message received\"}");
}

void handleExpression() {
  if (server->method() != HTTP_POST) {
    server->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server->arg("plain"));
  
  if (error) {
    server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
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
  
  server->send(200, "application/json", "{\"status\":\"success\",\"expression\":\"" + expression + "\"}");
}

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["current_message"] = current_message;
  doc["last_received"] = last_received_data;
  doc["bluetooth_connected"] = bluetooth_connected;
  doc["wifi_connected"] = wifi_manager->isConnected();
  doc["wifi_status"] = wifi_manager->getStatus();
  doc["ip_address"] = wifi_manager->getIPAddress();
  doc["connected_clients"] = wifi_manager->getConnectedClients();
  doc["is_ap_mode"] = wifi_manager->isAPMode();
  doc["mode"] = current_mode;
  
  String response;
  serializeJson(doc, response);
  server->send(200, "application/json", response);
}

// WiFi管理API
void handleWiFiScan() {
  wifi_manager->refreshNetworkScan();
  auto available = wifi_manager->getAvailableNetworks();
  auto configured = comm_config.getValidNetworks();
  
  DynamicJsonDocument doc(2048);
  JsonArray networks = doc.createNestedArray("networks");
  
  for (const auto& ssid : available) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = ssid;
    network["available"] = true;
    network["connected"] = (ssid == wifi_manager->getCurrentSSID());
    
    // 設定済みかチェック
    bool configured_network = false;
    for (const auto& config_net : configured) {
      if (config_net.ssid == ssid) {
        configured_network = true;
        network["priority"] = config_net.priority;
        break;
      }
    }
    network["configured"] = configured_network;
  }
  
  String response;
  serializeJson(doc, response);
  server->send(200, "application/json", response);
}

void handleWiFiToggle() {
  bool success = false;
  String mode;
  
  if (wifi_manager->isAPMode()) {
    success = wifi_manager->switchToSTAMode();
    mode = success ? "STAモード" : "切り替え失敗";
  } else {
    success = wifi_manager->switchToAPMode();
    mode = success ? "APモード" : "切り替え失敗";
  }
  
  DynamicJsonDocument doc(512);
  doc["success"] = success;
  doc["mode"] = mode;
  doc["ip"] = wifi_manager->getIPAddress();
  
  String response;
  serializeJson(doc, response);
  server->send(200, "application/json", response);
}

void handleWiFiAdd() {
  if (server->method() != HTTP_POST) {
    server->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server->arg("plain"));
  
  if (error) {
    server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String ssid = doc["ssid"];
  String password = doc["password"];
  int priority = doc["priority"] | 5; // デフォルト優先度
  
  if (ssid.isEmpty()) {
    server->send(400, "application/json", "{\"error\":\"SSID is required\"}");
    return;
  }
  
  comm_config.addWiFiNetwork(ssid, password, priority);
  
  M5_LOGI("WiFi network added: %s", ssid.c_str());
  
  server->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Network added\"}");
}

void handleWiFiConnect() {
  if (server->method() != HTTP_POST) {
    server->send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, server->arg("plain"));
  
  if (error) {
    server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String ssid = doc["ssid"];
  
  // 設定済みネットワークから検索
  auto networks = comm_config.getValidNetworks();
  for (const auto& network : networks) {
    if (network.ssid == ssid) {
      bool success = wifi_manager->connectToNetwork(network);
      
      DynamicJsonDocument response_doc(512);
      response_doc["success"] = success;
      response_doc["message"] = success ? "接続成功" : "接続失敗";
      response_doc["ssid"] = ssid;
      
      String response;
      serializeJson(response_doc, response);
      server->send(200, "application/json", response);
      return;
    }
  }
  
  server->send(404, "application/json", "{\"error\":\"Network not configured\"}");
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
            
            // 内蔵日本語フォントで画面表示
            M5.Display.fillRect(0, M5.Display.height() - 40, M5.Display.width(), 40, TFT_BLACK);
            M5.Display.setFont(&fonts::efontJA_16);
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(TFT_BLUE);
            M5.Display.setCursor(5, M5.Display.height() - 38);
            M5.Display.print("BT受信:");
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.setCursor(5, M5.Display.height() - 18);
            M5.Display.print(current_message);
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
          
          // シンプルな表示（英語フォント使用）
          M5.Display.fillRect(0, M5.Display.height() - 40, M5.Display.width(), 40, TFT_BLACK);
          M5.Display.setFont(&fonts::Font0);
          M5.Display.setTextSize(1);
          M5.Display.setTextColor(TFT_BLUE);
          M5.Display.setCursor(5, M5.Display.height() - 38);
          M5.Display.print("BT RX:");
          M5.Display.setTextColor(TFT_WHITE);
          M5.Display.setCursor(5, M5.Display.height() - 18);
          M5.Display.print(current_message);
        }
      } else {
        // プレーンテキストの場合
        current_message = receivedData;
        avatar.setSpeechText(current_message.c_str());
        
        // シンプルな表示（英語フォント使用）
        M5.Display.fillRect(0, M5.Display.height() - 40, M5.Display.width(), 40, TFT_BLACK);
        M5.Display.setFont(&fonts::Font0);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_BLUE);
        M5.Display.setCursor(5, M5.Display.height() - 38);
        M5.Display.print("BT RX:");
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setCursor(5, M5.Display.height() - 18);
        M5.Display.print(current_message);
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
  current_message = "WiFi初期化中...";
  avatar.setSpeechText(current_message.c_str());
  
  Serial.println("Creating WiFiManager...");
  wifi_manager = new WiFiManager(&comm_config);
  Serial.println("WiFiManager created");
  
  Serial.println("Starting WiFi begin...");
  if (wifi_manager->begin()) {
    current_message = "WiFi接続成功: " + wifi_manager->getCurrentSSID();
    M5_LOGI("WiFi setup completed: %s", wifi_manager->getStatus().c_str());
    Serial.println("WiFi connected successfully");
  } else {
    current_message = "WiFi接続失敗";
    M5_LOGI("WiFi setup failed");
    Serial.println("WiFi connection failed");
  }
  
  avatar.setSpeechText(current_message.c_str());
  Serial.println("setupWiFi completed");
}

void setupWebServer() {
  server->on("/", HTTP_GET, handleRoot);
  server->on("/api/message", HTTP_POST, handleMessage);
  server->on("/api/expression", HTTP_POST, handleExpression);
  server->on("/api/status", HTTP_GET, handleStatus);
  
  // WiFi管理API
  server->on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server->on("/api/wifi/toggle", HTTP_POST, handleWiFiToggle);
  server->on("/api/wifi/add", HTTP_POST, handleWiFiAdd);
  server->on("/api/wifi/connect", HTTP_POST, handleWiFiConnect);
  
  server->enableCORS(true);
  server->begin();
  M5_LOGI("Web server started on port %d", comm_config.webserver_port);
}

void setupBluetooth() {
  Serial.println("Starting Bluetooth initialization...");
  if (!SerialBT.begin(comm_config.bluetooth_device_name.c_str())) {
    M5_LOGE("Bluetooth initialization failed!");
    Serial.println("Bluetooth init failed!");
    return;
  }
  
  Serial.println("Registering Bluetooth callback...");
  SerialBT.register_callback(bluetoothCallback);
  current_message = "Bluetooth待機中...";
  avatar.setSpeechText(current_message.c_str());
  M5_LOGI("Bluetooth initialized: %s", comm_config.bluetooth_device_name.c_str());
  Serial.println("Bluetooth setup completed");
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
  M5.Display.setCursor(10, 110);
  M5.Display.print("Step 5: SD init...");
  if (SD.begin(GPIO_NUM_4, SPI, 25000000)) {
    M5.Display.setCursor(10, 130);
    M5.Display.print("Step 6: SD OK");
  } else {
    M5.Display.setCursor(10, 130);
    M5.Display.print("Step 6: SD SKIP");
  }
  delay(500);
  
  // 設定ファイル読み込み
  M5.Display.setCursor(10, 150);
  M5.Display.print("Step 7: Config load...");
  system_config.loadConfig(SD, "");
  comm_config.loadFromSystemConfig(system_config);
  M5.Display.setCursor(10, 170);
  M5.Display.print("Step 8: Config OK");
  
  // Webサーバーポート設定
  M5.Display.setCursor(10, 190);
  M5.Display.print("Step 9: WebServer obj...");
  Serial.println("DEBUG: Before WebServer creation");
  server = new WebServer(comm_config.webserver_port);
  Serial.println("DEBUG: After WebServer creation");
  M5.Display.setCursor(10, 210);
  M5.Display.print("Step 10: WebServer OK");
  
  // Avatar初期化
  Serial.println("DEBUG: Before Avatar init");
  delay(100);  // 安全のための短い遅延
  try {
    avatar.init();
    Serial.println("DEBUG: After Avatar init");
  } catch (...) {
    Serial.println("DEBUG: Avatar init FAILED");
    M5.Display.setCursor(10, 230);
    M5.Display.print("Avatar init FAILED");
    return;
  }
  
  // 段階的初期化メッセージ（デバッグ用）
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(10, 10);
  M5.Display.print("Stack-chan Init");
  M5.Display.setCursor(10, 30);
  M5.Display.print("Step 1: Display OK");
  delay(1000);
  
  // カラーパレット設定（白黒反転修正）
  M5.Display.setCursor(10, 50);
  M5.Display.print("Step 2: ColorPalette...");
  cps[0] = new ColorPalette();
  cps[0]->set(COLOR_PRIMARY, TFT_WHITE);      // 文字色を白に
  cps[0]->set(COLOR_BACKGROUND, TFT_BLACK);   // 背景色を黒に
  avatar.setColorPalette(*cps[0]);
  // Avatar初期化（簡素化）
  M5.Display.setCursor(10, 70);
  M5.Display.print("Step 3: Avatar basic...");
  delay(100);
  
  // 最小限のAvatar初期化
  try {
    avatar.init();
    M5.Display.setCursor(10, 90);
    M5.Display.print("Step 4: Avatar OK");
  } catch (...) {
    M5.Display.setCursor(10, 90);
    M5.Display.print("Step 4: Avatar SKIP");
  }
  
  // バッテリーアイコン設定
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(10, 10);
  M5.Display.print("Step 11: Battery...");
  if (M5.getBoard() != m5::board_t::board_M5Stack) {
    avatar.setBatteryIcon(true);
  }
  M5.Display.setCursor(10, 30);
  M5.Display.print("Step 12: Battery OK");
  
  M5.Display.setCursor(10, 50);
  M5.Display.print("Step 13: Speech test...");
  current_message = "システム初期化中...";
  avatar.setSpeechText(current_message.c_str());
  M5.Display.setCursor(10, 70);
  M5.Display.print("Step 14: Speech OK");
  delay(1000);
  
  // WiFi設定
  M5.Display.setCursor(10, 90);
  M5.Display.print("Step 15: WiFi setup...");
  setupWiFi();
  M5.Display.setCursor(10, 110);
  M5.Display.print("Step 16: WiFi done");
  delay(1000);
  
  // WebServer設定
  M5.Display.setCursor(10, 130);
  M5.Display.print("Step 17: WebServer...");
  setupWebServer();
  M5.Display.setCursor(10, 150);
  M5.Display.print("Step 18: WebServer OK");
  
  // Bluetooth設定
  M5.Display.setCursor(10, 170);
  M5.Display.print("Step 19: Bluetooth...");
  setupBluetooth();
  M5.Display.setCursor(10, 190);
  M5.Display.print("Step 20: BT done");
  
  current_message = "準備完了です！";
  avatar.setSpeechText(current_message.c_str());
  M5.Display.setCursor(10, 210);
  M5.Display.print("All systems ready!");
  
  last_display_update = millis();
  
  M5_LOGI("Setup completed");
}

void loop() {
  static unsigned long loop_counter = 0;
  static unsigned long last_debug_print = 0;
  
  // デバッグ用：5秒ごとにループカウンターを表示
  if (millis() - last_debug_print > 5000) {
    Serial.printf("Loop counter: %lu, Free heap: %u\n", loop_counter, ESP.getFreeHeap());
    last_debug_print = millis();
  }
  loop_counter++;
  
  M5.update();
  
  // Webサーバー処理
  server->handleClient();
  
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
    String status = wifi_manager->getStatus();
    current_message = status;
    avatar.setSpeechText(current_message.c_str());
  }
  
  if (M5.BtnC.wasPressed()) {
    // 表情をランダムに変更
    Expression expressions[] = {Expression::Neutral, Expression::Happy, 
                               Expression::Sleepy, Expression::Doubt};
    int randomIndex = random(0, 4);
    avatar.setExpression(expressions[randomIndex]);
    current_message = "表情を変更しました！";
    avatar.setSpeechText(current_message.c_str());
  }
  
  // 定期的な表示更新
  if ((millis() - last_display_update) > display_update_interval) {
    // WiFi接続状態チェック
    wifi_manager->reconnect();
    
    // バッテリー状態更新
    if (M5.getBoard() != m5::board_t::board_M5Stack) {
      avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
    }
    
    // ランダムメッセージ表示（カスタム日本語フォント使用）
    if (comm_config.lyrics.size() > 0) {
      const String& message = comm_config.lyrics[lyrics_idx++ % comm_config.lyrics.size()];
      avatar.setSpeechText(message.c_str());
      
      // 画面下部にシンプル表示（英語フォント使用）
      M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
      M5.Display.setFont(&fonts::Font0);
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(5, M5.Display.height() - 18);
      M5.Display.print(message);
    }
    
    // 口の動き
    avatar.setMouthOpenRatio(0.5);
    delay(100);
    avatar.setMouthOpenRatio(0.0);
    
    last_display_update = millis();
  }
  
  delay(50); // CPU負荷軽減
}
