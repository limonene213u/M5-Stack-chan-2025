/*
 * Stack-chan Communication Edition 
 * Simple Avatar + WiFi Communication System
 * Based on M5Unified_StackChan_ChatGPT (引き算方式)
 * 
 * @author Based on robo8080/M5Unified_StackChan_ChatGPT
 * Simplified for communication edition by TakaoAkaki
 */

#include <M5Unified.h>
#include <Avatar.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Stackchan_system_config.h>

// Avatar & Basic Setup
using namespace m5avatar;
Avatar avatar;

// Expression table for avatar
const Expression expressions_table[] = {
  Expression::Neutral,
  Expression::Happy,
  Expression::Sleepy,
  Expression::Doubt,
  Expression::Sad,
  Expression::Angry
};

// WebServer
WebServer server(80);
// StackchanSystemConfig for YAML settings
StackchanSystemConfig system_config;

// Display and communication state
String current_message = "スタックチャン起動中...";
String last_received_data = "";

// Communication mode
enum CommunicationMode {
  WIFI_MODE = 0,
  BLUETOOTH_MODE = 1,
  BOTH_MODE = 2
};
CommunicationMode current_mode = BOTH_MODE;

// Basic HTML page for web interface
static const char INDEX_HTML[] PROGMEM = R"KEWL(
<!DOCTYPE html>
<html lang="ja">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>スタックチャン 通信パネル</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
    .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
    .button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }
    .button:hover { background: #45a049; }
    .input-group { margin: 15px 0; }
    label { display: block; margin-bottom: 5px; }
    input, textarea, select { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>スタックチャン 通信パネル</h1>
    
    <div class="input-group">
      <label for="message">メッセージ送信:</label>
      <textarea id="message" rows="3" placeholder="メッセージを入力"></textarea>
      <button class="button" onclick="sendMessage()">送信</button>
    </div>
    
    <div class="input-group">
      <label for="expression">表情変更:</label>
      <select id="expression">
        <option value="0">普通</option>
        <option value="1">嬉しい</option>
        <option value="2">眠い</option>
        <option value="3">疑問</option>
        <option value="4">悲しい</option>
        <option value="5">怒り</option>
      </select>
      <button class="button" onclick="changeExpression()">表情変更</button>
    </div>
    
    <div class="input-group">
      <h3>現在の状態</h3>
      <p id="status">Loading...</p>
      <button class="button" onclick="getStatus()">更新</button>
    </div>
  </div>

  <script>
    function sendMessage() {
      const message = document.getElementById('message').value;
      if (!message) return;
      
      fetch('/api/message', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ message: message })
      }).then(r => r.text()).then(data => {
        alert('送信完了: ' + data);
        document.getElementById('message').value = '';
      });
    }
    
    function changeExpression() {
      const expr = document.getElementById('expression').value;
      
      fetch('/api/expression', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ expression: expr })
      }).then(r => r.text()).then(data => {
        alert('表情変更: ' + data);
      });
    }
    
    function getStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('status').innerHTML = 
            'メッセージ: ' + data.current_message + '<br>' +
            'WiFi: ' + (data.wifi_connected ? '接続済み' : '未接続') + '<br>' +
            'モード: ' + data.mode;
        });
    }
    
    // Auto-update status
    setInterval(getStatus, 5000);
    getStatus();
  </script>
</body>
</html>)KEWL";
// Web server handlers
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleMessage() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }
  
  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, body);
  
  String message = doc["message"];
  if (message.length() > 0) {
    current_message = message;
    avatar.setSpeechText(message.c_str());
    Serial.println("Message received: " + message);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Invalid message");
  }
}

void handleExpression() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }
  
  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, body);
  
  int expr = doc["expression"];
  if (expr >= 0 && expr <= 5) {
    avatar.setExpression(expressions_table[expr]);
    Serial.println("Expression changed to: " + String(expr));
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Invalid expression");
  }
}

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["current_message"] = current_message;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifi_status"] = (WiFi.status() == WL_CONNECTED) ? 
    "接続済み: " + WiFi.SSID() + " (" + WiFi.localIP().toString() + ")" : "未接続";
  doc["ip_address"] = WiFi.localIP().toString();
  doc["mode"] = current_mode;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

// WiFi Setup (SmartConfig pattern from ChatGPT version)
void setupWiFi() {
  // Try previous connection first
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    M5.Display.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    // Use SmartConfig if no previous connection
    Serial.println("\nStarting SmartConfig...");
    M5.Display.println("\nSmartConfig開始");
    
    WiFi.beginSmartConfig();
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print("#");
      M5.Display.print("#");
    }
    
    Serial.println("\nSmartConfig完了");
    M5.Display.println("\nSmartConfig完了");
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      M5.Display.print(".");
    }
  }
  
  Serial.println("\nWiFi接続完了");
  Serial.println("IP: " + WiFi.localIP().toString());
  M5.Display.println("\nWiFi接続完了");
  M5.Display.println("IP: " + WiFi.localIP().toString());
}
// Setup function
void setup() {
  // M5Stack initialization
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("Stack-chan Communication Edition Starting...");
  
  M5.Display.setTextSize(2);
  M5.Display.println("Stack-chan");
  M5.Display.println("Communication");
  
  // Initialize SPIFFS for system config
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    M5.Display.println("SPIFFS Error");
    return;
  }
  
  // Load system configuration (YAML support)
  if (system_config.loadConfig("/yaml/SC_BasicConfig.yaml")) {
    Serial.println("System config loaded successfully");
  } else {
    Serial.println("Using default configuration");
  }
  
  // WiFi setup
  M5.Display.println("WiFi connecting...");
  setupWiFi();
  
  // Setup web server
  server.on("/", handleRoot);
  server.on("/api/message", HTTP_POST, handleMessage);
  server.on("/api/expression", HTTP_POST, handleExpression);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  M5.Display.println("Server started");
  
  // Initialize Avatar
  avatar.init(8); // Color depth 8 for better performance
  avatar.setSpeechFont(&fonts::efontJA_16); // Japanese font support
  avatar.setExpression(Expression::Happy);
  avatar.setSpeechText("こんにちは！");
  
  Serial.println("Setup completed");
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  M5.Display.println("Ready!");
  M5.Display.println("IP: " + WiFi.localIP().toString());
  
  delay(2000);
}

// Main loop
void loop() {
  M5.update();
  server.handleClient();
  
  // Button controls
  if (M5.BtnA.wasPressed()) {
    // Cycle through expressions
    static int expr_index = 0;
    avatar.setExpression(expressions_table[expr_index]);
    expr_index = (expr_index + 1) % 6;
    Serial.println("Expression changed to: " + String(expr_index));
  }
  
  if (M5.BtnB.wasPressed()) {
    // Display status
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("Status:");
    M5.Display.println("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "OK" : "NG"));
    M5.Display.println("IP: " + WiFi.localIP().toString());
    M5.Display.println("Message:");
    M5.Display.println(current_message);
    delay(3000);
  }
  
  if (M5.BtnC.wasPressed()) {
    // Test message
    current_message = "テストメッセージです";
    avatar.setSpeechText(current_message.c_str());
    avatar.setExpression(Expression::Happy);
    Serial.println("Test message displayed");
  }
  
  delay(10); // Small delay for stability
}
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
  
  // Avatar機能での表示を試行（初期化済みの場合のみ）
  if (avatar_initialized) {
    try {
      avatar.setSpeechText(message.c_str());
      Serial.println("DEBUG: Avatar speech text set successfully");
    } catch (...) {
      Serial.println("DEBUG: Avatar speech text failed, using display fallback");
    }
  }
  
  last_received_data = "WiFi: " + message;
  
  // 日本語対応表示（M5GFXの内蔵フォント使用）
  M5.Display.fillRect(0, M5.Display.height() - 40, M5.Display.width(), 40, TFT_BLACK);
  
  // 日本語フォント試行（正しいAPI使用、小さめサイズ）
  try {
    M5.Lcd.setTextFont(&fonts::efontJA_16);  // 日本語フォント（正しいAPI）
    M5.Display.setTextSize(0.5);  // サイズを半分に
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(5, M5.Display.height() - 38);
    M5.Display.print("WiFi受信:");
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, M5.Display.height() - 18);
    M5.Display.print(message);
    Serial.println("DEBUG: Japanese font display successful");
  } catch (...) {
    // フォールバック：英語フォント使用
    M5.Display.setFont(&fonts::Font0);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(5, M5.Display.height() - 38);
    M5.Display.print("WiFi RX:");
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, M5.Display.height() - 18);
    M5.Display.print(message);
    Serial.println("DEBUG: Japanese font failed, using English fallback");
  }
  
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
  
  // Avatar表情変更（初期化済みの場合のみ）
  if (avatar_initialized) {
    try {
      if (expression == "happy") {
        avatar.setExpression(Expression::Happy);
      } else if (expression == "sleepy") {
        avatar.setExpression(Expression::Sleepy);
      } else if (expression == "doubt") {
        avatar.setExpression(Expression::Doubt);
      } else {
        avatar.setExpression(Expression::Neutral);
      }
      Serial.printf("DEBUG: Avatar expression changed to: %s\n", expression.c_str());
    } catch (...) {
      Serial.printf("DEBUG: Avatar expression change failed for: %s\n", expression.c_str());
    }
  } else {
    Serial.printf("DEBUG: Avatar not initialized, expression change skipped: %s\n", expression.c_str());
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
            
            // 内蔵日本語フォントで画面表示（正しいAPI使用、小さめサイズ）
            M5.Display.fillRect(0, M5.Display.height() - 40, M5.Display.width(), 40, TFT_BLACK);
            M5.Lcd.setTextFont(&fonts::efontJA_16);
            M5.Display.setTextSize(0.5);  // サイズを半分に
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
  // 元stack-chan-tester準拠の初期化順序
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  M5.setTouchButtonHeight(40);
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  
  // 通信エディション向けメモリ事前確保（参考実装より）
  preallocateBuffer = (uint8_t *)malloc(preallocateBufferSize);
  if (!preallocateBuffer) {
    M5.Display.printf("Memory allocation failed: %d bytes\n", preallocateBufferSize);
    Serial.printf("ERROR: Unable to preallocate %d bytes for communication buffer\n", preallocateBufferSize);
    // 継続（通信エディションでは致命的でない）
  } else {
    Serial.printf("SUCCESS: Preallocated %d bytes for communication buffer\n", preallocateBufferSize);
  }
  
  M5_LOGI("Stack-chan Communication Edition Started");
  Serial.println("DEBUG: M5 initialized successfully");
  delay(100);
  
  // SDカード初期化（元実装準拠）
  Serial.println("DEBUG: Before SD initialization");
  bool sd_and_config_ok = false;
  
  if (SD.begin(GPIO_NUM_4, SPI, 25000000)) {
    delay(2000);  // 元実装準拠：SDカード安定化待機
    
    // StackchanSystemConfig読み込み（元実装準拠）
    try {
      system_config.loadConfig(SD, "");  // 元実装と同じ呼び出し
      comm_config.loadFromSystemConfig(system_config);
      Serial.println("DEBUG: StackchanSystemConfig loaded successfully");
      
      // 参考実装パターン：シンプル設定ファイルも読み込み（追加設定）
      loadSimpleWiFiConfig();  // wifi.txt があれば追加
      loadSimpleAPIConfig();   // apikey.txt があれば将来用に保存
      
      sd_and_config_ok = true;
    } catch (...) {
      Serial.println("DEBUG: StackchanSystemConfig failed - using defaults");
    }
  } else {
    Serial.println("DEBUG: SD card not available - using defaults");
  }
  
  // デフォルト設定（SD未使用時）
  if (!sd_and_config_ok) {
    comm_config.webserver_port = 80;
    comm_config.bluetooth_device_name = "M5Stack-StackChan";
    comm_config.bluetooth_starting_state = true;
    comm_config.lyrics.clear();
    comm_config.lyrics.push_back("こんにちは");
    comm_config.lyrics.push_back("元気です");
    comm_config.lyrics.push_back("よろしく");
    Serial.println("DEBUG: Default config applied successfully");
  }
  
  // I2C管理（元実装準拠）
  bool core_port_a = false;
  if (M5.getBoard() == m5::board_t::board_M5Stack) {
    // Core1での特別処理（元実装準拠）
    Serial.println("DEBUG: Core1 detected - I2C management");
    // Note: サーボなしの通信版ではI2C解放は不要だが、将来のサーボ対応のため記録
  }
  
  // WebServer初期化
  Serial.println("DEBUG: Before WebServer creation");
  server = new WebServer(comm_config.webserver_port);
  Serial.println("DEBUG: After WebServer creation");
  
  // WiFi設定
  Serial.println("DEBUG: Setting up WiFi");
  setupWiFi();
  
  // WebServer設定
  setupWebServer();
  
  // Avatar初期化（元実装準拠：setup()内で完了）
  // � 参考実装準拠のAvatar初期化（ChatGPT版パターン適用）
  Serial.println("DEBUG: Avatar initialization (ChatGPT reference pattern)");
  
  // 参考実装と同じ初期化パラメータを使用
  avatar.init(8);  // Color Depth 8 指定（参考実装準拠）
  
  cps[0] = new ColorPalette();
  cps[0]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[0]->set(COLOR_BACKGROUND, TFT_WHITE);
  avatar.setColorPalette(*cps[0]);
  avatar.setSpeechFont(&fonts::efontJA_16);  // 日本語フォント設定
  
  // 🎯 タスク追加なし：基本Avatar機能のみでFreeRTOS競合回避
  Serial.println("DEBUG: Avatar basic initialization completed (no tasks)");
  
  avatar_initialized = true;  // 初期化完了フラグ
  last_display_update = millis();
  M5_LOGI("Setup completed");
}

void loop() {
  // 🚨 FreeRTOSキュー競合回避：Avatar操作を最小限に制限
  static uint32_t last_mouth_millis = 0;
  static int lyrics_idx = 0;
  static uint32_t mouth_wait = 10000;  // 10秒間隔に延長（安定性優先）
  static bool avatar_safe_mode = true;  // 安全モード有効
  
  M5.update();  // 元実装準拠
  
  // Webサーバー処理（追加機能）
  server->handleClient();
  
  // Bluetooth通信処理（追加機能）
  handleBluetoothData();
  
  // ボタン処理（Avatar状態適応版）
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
    
    // Avatar機能が初期化されている場合のみ使用
    if (avatar_initialized) {
      try {
        avatar.setSpeechText(current_message.c_str());
        Serial.println("DEBUG: Avatar speech text set successfully");
      } catch (...) {
        Serial.println("DEBUG: Avatar speech text failed");
      }
    }
    
    Serial.println("Mode changed");
    M5_LOGI("Mode changed to: %d", current_mode);
  }
  
  if (M5.BtnB.wasPressed()) {
    // ステータス表示
    String status = wifi_manager->getStatus();
    current_message = status;
    
    // Avatar機能が初期化されている場合のみ使用
    if (avatar_initialized) {
      try {
        avatar.setSpeechText(current_message.c_str());
        Serial.println("DEBUG: Avatar status display successful");
      } catch (...) {
        Serial.println("DEBUG: Avatar status display failed");
      }
    }
    
    Serial.println("Status requested");
  }
  
  if (M5.BtnC.wasPressed()) {
    // 表情をランダムに変更（Avatar状態適応版）
    if (avatar_initialized) {
      try {
        Expression expressions[] = {Expression::Neutral, Expression::Happy, 
                                   Expression::Sleepy, Expression::Doubt};
        int randomIndex = random(0, 4);
        avatar.setExpression(expressions[randomIndex]);
        current_message = "表情を変更しました！";
        avatar.setSpeechText(current_message.c_str());
        Serial.println("DEBUG: Avatar expression changed successfully");
      } catch (...) {
        current_message = "ボタンCが押されました！";
        Serial.println("DEBUG: Avatar expression change failed");
      }
    } else {
      current_message = "ボタンCが押されました！";
      Serial.println("DEBUG: Avatar not initialized, simple button response");
    }
    
    Serial.println("Button C pressed");
  }
  
  // 定期的な表示更新（Avatar機能復旧版）
  if ((millis() - last_display_update) > display_update_interval) {
    // WiFi接続状態チェック
    wifi_manager->reconnect();
    
    // バッテリー状態更新（Avatar状態適応版）
    if (avatar_initialized) {
      try {
        if (M5.getBoard() != m5::board_t::board_M5Stack) {
          avatar.setBatteryStatus(M5.Power.isCharging(), M5.Power.getBatteryLevel());
        }
      } catch (...) {
        Serial.println("DEBUG: Avatar battery status update failed");
      }
    }
    
    // メッセージ表示（Avatar + フォールバック対応）
    if (comm_config.lyrics.size() > 0) {
      const String& message = comm_config.lyrics[lyrics_idx++ % comm_config.lyrics.size()];
      
      // Avatarでの表示を試行（初期化済みの場合のみ）
      if (avatar_initialized) {
        try {
          avatar.setSpeechText(message.c_str());
          Serial.println("DEBUG: Avatar message display successful");
        } catch (...) {
          Serial.println("DEBUG: Avatar message display failed, using screen fallback");
        }
      }
      
      // 画面下部にフォールバック表示（常に実行）
      M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
      M5.Display.setFont(&fonts::Font0);
      M5.Display.setTextSize(1);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setCursor(5, M5.Display.height() - 18);
      M5.Display.print(message);
    }
    
    // 🚨 FreeRTOSキュー競合回避：Avatar操作を大幅制限
    static unsigned long last_mouth_animation = 0;
    if (avatar_initialized && (millis() - last_mouth_animation) > 30000) {  // 30秒間隔に大幅延長
      try {
        // Avatar操作を最小限に制限（setSpeechTextを無効化）
        Serial.println("DEBUG: Minimal avatar operation start");
        // avatar.setMouthOpenRatio(0.1);  // 一時的に無効化
        delay(50);  // 短縮
        // avatar.setMouthOpenRatio(0.0);   // 一時的に無効化
        last_mouth_animation = millis();
        Serial.println("DEBUG: Minimal avatar operation completed");
      } catch (...) {
        Serial.println("ERROR: Avatar operation failed - disabling");
        avatar_initialized = false;  // エラー時は完全停止
      }
    }
    
    last_display_update = millis();
  }
  
  delay(50); // CPU負荷軽減
}

// ------------------------
// 将来拡張：シンプル設定ファイル読み込み（参考実装パターン）
void loadSimpleWiFiConfig() {
  // 参考実装スタイル：wifi.txt読み込み（将来対応）
  auto fs = SD.open("/wifi.txt", FILE_READ);
  if(fs) {
    size_t sz = fs.size();
    char buf[sz + 1];
    fs.read((uint8_t*)buf, sz);
    buf[sz] = 0;
    fs.close();

    int y = 0;
    for(int x = 0; x < sz; x++) {
      if(buf[x] == 0x0a || buf[x] == 0x0d)
        buf[x] = 0;
      else if (!y && x > 0 && !buf[x - 1] && buf[x])
        y = x;
    }
    
    // 追加設定として保存（既存StackchanSystemConfigと併用）
    comm_config.addWiFiNetwork(String(buf), String(&buf[y]), 10);  // 最高優先度
    Serial.printf("Simple WiFi config loaded: %s\n", buf);
  }
}

void loadSimpleAPIConfig() {
  // 参考実装スタイル：apikey.txt読み込み（将来のChatGPT対応準備）
  auto fs = SD.open("/apikey.txt", FILE_READ);
  if(fs) {
    size_t sz = fs.size();
    char buf[sz + 1];
    fs.read((uint8_t*)buf, sz);
    buf[sz] = 0;
    fs.close();
    
    // 将来のAPI設定保存準備（現在は未使用）
    Serial.println("Simple API config found (future use)");
  }
}

// ------------------------
// 通信エディション用 Avatar タスク関数（参考実装準拠）

// ------------------------
// WebServer & API Handler関数
