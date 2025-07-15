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
  JsonDocument doc;
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
  JsonDocument doc;
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
