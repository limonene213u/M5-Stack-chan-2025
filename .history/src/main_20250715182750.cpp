/*
 * Stack-chan 最小構成 Avatar + WiFi + WebServer
 * Avatar表示 + WiFi接続 + 簡単なWebUI
 */

#include <M5Unified.h>
#include <Avatar.h>
#include <WiFi.h>
#include <WebServer.h>
#include "simple_wifi_config.h"

using namespace m5avatar;

// Avatar関連
Avatar avatar;
ColorPalette* cps[2];
bool avatar_initialized = false;

// WiFi & WebServer関連
WebServer server(WEBSERVER_PORT);
bool wifi_connected = false;
String current_ip = "";
unsigned long last_wifi_check = 0;

// 表示制御
String current_message = "スタックちゃん";
unsigned long last_expression_change = 0;
int current_expression = 0;

// 関数プロトタイプ宣言
bool connectToWiFi();
void handleRoot();
void handleApiExpression();
void handleApiColor();
void handle404();

void setup() {
  // M5Stack基本初期化
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("=== Stack-chan Avatar + WiFi + WebServer Edition ===");
  
  // 初期表示
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Avatar初期化中...");
  
  Serial.println("Avatar初期化開始");
  
  // Avatar初期化（エラーハンドリング付き）
  try {
    // ColorPalette作成
    cps[0] = new ColorPalette();
    cps[1] = new ColorPalette();
    cps[1]->set(COLOR_PRIMARY, TFT_YELLOW);
    cps[1]->set(COLOR_BACKGROUND, TFT_BLUE);
    
    // Avatar基本初期化
    avatar.init();
    avatar.setColorPalette(*cps[0]);
    
    // 日本語フォント設定
    avatar.setSpeechFont(&fonts::efontJA_16);
    
    // 初期表情と発話設定
    avatar.setExpression(Expression::Neutral);
    avatar.setSpeechText(current_message.c_str());
    
    avatar_initialized = true;
    Serial.println("Avatar初期化成功");
    
  } catch (...) {
    Serial.println("Avatar初期化失敗");
    avatar_initialized = false;
    
    // フォールバック表示
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Avatar Error");
    M5.Display.println("Basic Mode");
  }
  
  // WiFi接続開始
  Serial.println("WiFi接続開始");
  current_message = "WiFi接続中...";
  if (avatar_initialized) {
    avatar.setSpeechText(current_message.c_str());
  }
  
  if (connectToWiFi()) {
    // WebServer初期化
    Serial.println("WebServer初期化開始");
    
    // ルート設定
    server.on("/", handleRoot);
    server.on("/api/expression", HTTP_GET, handleApiExpression);
    server.on("/api/color", HTTP_GET, handleApiColor);
    server.onNotFound(handle404);
    
    // サーバー開始
    server.begin();
    Serial.printf("WebServer開始: http://%s/\n", current_ip.c_str());
    
    current_message = String("WebUI: ") + current_ip;
    if (avatar_initialized) {
      avatar.setSpeechText(current_message.c_str());
    }
  } else {
    Serial.println("WiFi接続失敗 - WebServerは無効");
  }
  
  Serial.println("初期化完了 - Avatar + WiFi + WebServer モード");
}

void loop() {
  M5.update();
  
  // WebServer処理（WiFi接続時のみ）
  if (wifi_connected) {
    server.handleClient();
  }
  
  if (avatar_initialized) {
    // Button A: 表情変更（4種類をサイクル）
    if (M5.BtnA.wasPressed()) {
      Serial.println("Button A: 表情変更");
      current_expression = (current_expression + 1) % 4;
      
      switch (current_expression) {
        case 0:
          avatar.setExpression(Expression::Neutral);
          current_message = "普通";
          break;
        case 1:
          avatar.setExpression(Expression::Happy);
          current_message = "嬉しい";
          break;
        case 2:
          avatar.setExpression(Expression::Sleepy);
          current_message = "眠い";
          break;
        case 3:
          avatar.setExpression(Expression::Doubt);
          current_message = "困った";
          break;
      }
      
      avatar.setSpeechText(current_message.c_str());
      Serial.printf("表情: %s\n", current_message.c_str());
    }
    
    // Button B: WiFi再接続
    if (M5.BtnB.wasPressed()) {
      Serial.println("Button B: WiFi再接続");
      connectToWiFi();
    }
    
    // Button C: IP表示
    if (M5.BtnC.wasPressed()) {
      Serial.println("Button C: IP表示");
      if (wifi_connected) {
        current_message = String("IP: ") + current_ip;
        avatar.setSpeechText(current_message.c_str());
        Serial.printf("IP表示: %s\n", current_ip.c_str());
      } else {
        current_message = "WiFi未接続";
        avatar.setSpeechText(current_message.c_str());
      }
    }
    
    // 自動まばたき（10秒ごと）
    if (millis() - last_expression_change > 10000) {
      avatar.setExpression(Expression::Neutral);
      last_expression_change = millis();
    }
    
  } else {
    // Avatar失敗時の基本操作
    if (M5.BtnA.wasPressed()) {
      M5.Display.fillScreen(TFT_GREEN);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Button A");
      delay(500);
    }
    
    if (M5.BtnB.wasPressed()) {
      M5.Display.fillScreen(TFT_BLUE);
      M5.Display.setCursor(10, 10);
      M5.Display.println("WiFi Retry");
      connectToWiFi();
      delay(500);
    }
    
    if (M5.BtnC.wasPressed()) {
      M5.Display.fillScreen(TFT_YELLOW);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Button C");
      delay(500);
    }
  }
  
  // WiFi接続状態監視（30秒ごと）
  if (millis() - last_wifi_check > 30000) {
    if (wifi_connected && WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi接続が切断されました");
      wifi_connected = false;
      current_ip = "";
      current_message = "WiFi切断";
      if (avatar_initialized) {
        avatar.setSpeechText(current_message.c_str());
      }
    }
    last_wifi_check = millis();
  }
  
  // システム監視（10秒ごと）
  static unsigned long last_heartbeat = 0;
  if (millis() - last_heartbeat > 10000) {
    Serial.printf("Avatar=%s, WiFi=%s, Memory=%dKB, Uptime=%lus\n", 
                  avatar_initialized ? "OK" : "NG",
                  wifi_connected ? "OK" : "NG", 
                  ESP.getFreeHeap() / 1024, 
                  millis() / 1000);
    last_heartbeat = millis();
  }
  
  delay(50);
}

// WiFi接続関数
bool connectToWiFi() {
  wifi_connected = false;
  current_ip = "";
  
  // 既存の接続があれば切断
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(100);
  }
  
  // 設定されたWiFiネットワークを順番に試行
  for (int i = 0; wifi_networks[i].ssid != nullptr; i++) {
    Serial.printf("WiFi接続試行: %s (優先度:%d)\n", 
                  wifi_networks[i].ssid, wifi_networks[i].priority);
    
    // Avatar表示更新
    if (avatar_initialized) {
      current_message = String("接続中: ") + wifi_networks[i].ssid;
      avatar.setSpeechText(current_message.c_str());
    }
    
    WiFi.begin(wifi_networks[i].ssid, wifi_networks[i].password);
    
    // 接続待機（最大10秒）
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && 
           (millis() - start_time) < CONNECTION_TIMEOUT) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      wifi_connected = true;
      current_ip = WiFi.localIP().toString();
      current_message = String("IP: ") + current_ip;
      
      Serial.printf("\nWiFi接続成功: %s\n", current_ip.c_str());
      Serial.printf("   SSID: %s\n", WiFi.SSID().c_str());
      Serial.printf("   RSSI: %d dBm\n", WiFi.RSSI());
      
      if (avatar_initialized) {
        avatar.setSpeechText(current_message.c_str());
      }
      
      return true;
    } else {
      Serial.printf("\nWiFi接続失敗: %s\n", wifi_networks[i].ssid);
    }
  }
  
  // 全て失敗
  current_message = "WiFi接続失敗";
  if (avatar_initialized) {
    avatar.setSpeechText(current_message.c_str());
  }
  Serial.println("全てのWiFiネットワークへの接続に失敗");
  return false;
}

// WebServerハンドラー関数
void handleRoot() {
  String html = "<html><head><title>Stack-chan</title></head><body>";
  html += "<h1>Stack-chan WebUI</h1>";
  html += "<h3>System Status</h3>";
  html += "<p>Free Memory: " + String(ESP.getFreeHeap() / 1024) + " KB</p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  html += "<p>WiFi SSID: " + WiFi.SSID() + "</p>";
  html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Signal: " + String(WiFi.RSSI()) + " dBm</p>";
  html += "<br>";
  html += "<button onclick='location.reload()'>Reload</button> ";
  html += "<button onclick='changeExpression()'>Expression</button> ";
  html += "<button onclick='changeColor()'>Color</button>";
  html += "<script>";
  html += "function changeExpression(){fetch('/api/expression').then(()=>alert('Expression changed')).catch(()=>alert('Error'));}";
  html += "function changeColor(){fetch('/api/color').then(()=>alert('Color changed')).catch(()=>alert('Error'));}";
  html += "</script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
  Serial.println("WebUI Access: " + server.client().remoteIP().toString());
}

void handleApiExpression() {
  if (avatar_initialized) {
    current_expression = (current_expression + 1) % 4;
    switch (current_expression) {
      case 0: avatar.setExpression(Expression::Neutral); current_message = "普通"; break;
      case 1: avatar.setExpression(Expression::Happy); current_message = "嬉しい"; break;
      case 2: avatar.setExpression(Expression::Sleepy); current_message = "眠い"; break;
      case 3: avatar.setExpression(Expression::Doubt); current_message = "困った"; break;
    }
    avatar.setSpeechText(current_message.c_str());
    server.send(200, "text/plain", "Expression changed to: " + current_message);
    Serial.println("API: 表情変更 -> " + current_message);
  } else {
    server.send(500, "text/plain", "Avatar not initialized");
  }
}

void handleApiColor() {
  if (avatar_initialized) {
    static bool use_alt_color = false;
    use_alt_color = !use_alt_color;
    
    if (use_alt_color) {
      avatar.setColorPalette(*cps[1]);
      current_message = "青色";
    } else {
      avatar.setColorPalette(*cps[0]);
      current_message = "標準色";
    }
    
    avatar.setSpeechText(current_message.c_str());
    server.send(200, "text/plain", "Color changed to: " + current_message);
    Serial.println("API: 色変更 -> " + current_message);
  } else {
    server.send(500, "text/plain", "Avatar not initialized");
  }
}

void handle404() {
  server.send(404, "text/plain", "404 Not Found - Stack-chan WebUI");
}
