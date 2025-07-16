/*
 * Stack-chan シンプル構成 Avatar + WiFi/BLE + WebServer
 * Avatar表示 + WiFi接続 + BLE接続 + 基本WebUI
 */

#include <M5Unified.h>
#include <Avatar.h>
#include <WiFi.h>
#include <WebServer.h>
#include "simple_wifi_config.h"
#include "ble_webui.h"

using namespace m5avatar;

// Avatar関連
Avatar avatar;
ColorPalette* cps[6];  // 6色に拡張
bool avatar_initialized = false;

// WiFi & WebServer関連
WebServer server(WEBSERVER_PORT);
bool wifi_connected = false;
String current_ip = "";
unsigned long last_wifi_check = 0;

// BLE関連
bool ble_enabled = false;
bool connection_mode_ble = false;  // true: BLEモード, false: WiFiモード
BLEWebUIHandler* bleWebUI = nullptr;  // BLE WebUIハンドラー

// 表示制御
String current_message = "スタックちゃん";
unsigned long last_expression_change = 0;
int current_expression = 0;
bool is_speaking = false;  // 音声状態管理

// セリフ自動制御
unsigned long last_speech_time = 0;
bool speech_set_by_user = false;
bool random_speech_enabled = false;

// 色制御
int current_color_index = 0;

// 関数プロトタイプ宣言
bool connectToWiFi();
void setupWebServer();
void handleRoot();
void handleApiExpression();
void handleApiColor();
void handleApiSetColor();
void handleApiSet();
void handle404();
String generateWebUIHTML();  // 追加: 共通HTML生成関数
void checkRandomSpeechConfig();
String getRandomSpeech();
void updateSpeechLoop();
void initializeBLE();
void toggleConnectionMode();

// BLE WebUI用の外部関数（ble_webui.cppから呼び出される）
void changeExpressionById(int id);
void changeColorById(int id);
void setSpeechText(const String& text, int expression = -1);
String getSystemStatusJSON();
void checkRandomSpeechConfig();
String getRandomSpeech();
void updateSpeechLoop();

void setup() {
  // M5Stack基本初期化
  Serial.begin(115200);
  delay(100); // シリアル安定化
  Serial.println("=== Stack-chan Avatar + WiFi + WebServer Edition ===");
  Serial.println("setup() 開始");
  Serial.printf("起動時メモリ: %d bytes\n", ESP.getFreeHeap());
  
  Serial.println("M5.config() 設定中...");
  auto cfg = M5.config();
  Serial.println("M5.begin() 実行中...");
  M5.begin(cfg);
  Serial.println("M5Stack初期化完了");
  Serial.printf("M5初期化後メモリ: %d bytes\n", ESP.getFreeHeap());
  
  // 初期表示
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Avatar初期化中...");
  
  Serial.println("Avatar初期化開始");
  Serial.printf("Free heap before Avatar: %d bytes\n", ESP.getFreeHeap());
  
  // Avatar初期化（シンプル構成）
  try {
    Serial.println("ColorPalette作成開始");
    // 6つの色パレットを作成
    for (int i = 0; i < 6; i++) {
      cps[i] = new ColorPalette();
    }
    Serial.println("ColorPalette作成完了");
    
    Serial.println("ColorPalette設定開始");
    // 0: 標準色（デフォルト）
    // 標準設定のまま
    
    // 1: 青系
    cps[1]->set(COLOR_PRIMARY, TFT_YELLOW);
    cps[1]->set(COLOR_BACKGROUND, TFT_BLUE);
    
    // 2: 緑系
    cps[2]->set(COLOR_PRIMARY, TFT_WHITE);
    cps[2]->set(COLOR_BACKGROUND, TFT_GREEN);
    
    // 3: 赤系
    cps[3]->set(COLOR_PRIMARY, TFT_WHITE);
    cps[3]->set(COLOR_BACKGROUND, TFT_RED);
    
    // 4: 紫系
    cps[4]->set(COLOR_PRIMARY, TFT_YELLOW);
    cps[4]->set(COLOR_BACKGROUND, TFT_PURPLE);
    
    // 5: オレンジ系
    cps[5]->set(COLOR_PRIMARY, TFT_BLACK);
    cps[5]->set(COLOR_BACKGROUND, TFT_ORANGE);
    
    Serial.println("ColorPalette設定完了");
    
    Serial.println("Avatar.init()実行開始");
    avatar.init();
    Serial.println("Avatar.init()実行完了");
    
    Serial.println("ColorPalette適用開始");
    avatar.setColorPalette(*cps[current_color_index]);
    Serial.println("ColorPalette適用完了");
    
    Serial.println("フォント設定開始");
    avatar.setSpeechFont(&fonts::efontJA_12);
    Serial.println("フォント設定完了");
    
    Serial.println("初期表情設定開始");
    avatar.setExpression(Expression::Neutral);
    Serial.println("初期表情設定完了");
    
    Serial.println("初期セリフ設定開始");
    avatar.setSpeechText(current_message.c_str());
    Serial.println("初期セリフ設定完了");
    
    avatar_initialized = true;
    Serial.println("Avatar初期化成功");
    Serial.printf("Free heap after Avatar: %d bytes\n", ESP.getFreeHeap());
    
  } catch (...) {
    Serial.println("Avatar初期化で例外が発生しました");
    avatar_initialized = false;
    
    // フォールバック表示
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Avatar Error");
    M5.Display.println("Basic Mode");
    Serial.println("Avatar初期化失敗 - フォールバックモードで継続");
  }
  
  // 接続モード決定（WiFi優先、Bボタンで割り込み可能）
  Serial.println("通信モード初期化開始");
  current_message = "WiFi接続中... (Bボタン=BLE切替)";
  if (avatar_initialized) {
    avatar.setSpeechText(current_message.c_str());
  }
  
  if (connectToWiFi()) {
    // WiFiモード
    connection_mode_ble = false;
    Serial.println("WiFiモードで起動");
    
    // WebServer初期化
    Serial.println("WebServer初期化開始");
    setupWebServer();
    
    current_message = String("WebUI: ") + current_ip;
    if (avatar_initialized) {
      avatar.setSpeechText(current_message.c_str());
    }
  } else {
    // BLEモード（WiFi失敗またはBボタン割り込み）
    connection_mode_ble = true;
    Serial.println("BLEペアリングモードで起動");
    
    current_message = "BLEペアリングモード初期化中...";
    if (avatar_initialized) {
      avatar.setSpeechText(current_message.c_str());
    }
    
    initializeBLE();
    
    current_message = "BLE: " + String(BLE_DEVICE_NAME) + " (ペアリング待機中)";
    if (avatar_initialized) {
      avatar.setSpeechText(current_message.c_str());
    }
  }
  
  Serial.println("初期化完了 - Avatar + WiFi/BLE + WebServer モード");
  
  // ランダムセリフ設定確認
  checkRandomSpeechConfig();
}

void loop() {
  M5.update();
  
  // 通信処理
  if (!connection_mode_ble && wifi_connected) {
    // WiFiモード
    server.handleClient();
  } else if (connection_mode_ble && ble_enabled) {
    // BLEモード
    if (bleWebUI) {
      bleWebUI->handleBLERequest();
    }
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
    // Button B: 通信モード切り替え（WiFi ⟷ BLE）
    if (M5.BtnB.wasPressed()) {
      Serial.println("Button B: 通信モード切り替え");
      
      // 切り替え中のメッセージ表示
      if (connection_mode_ble) {
        current_message = "WiFiモードに切り替え中...";
      } else {
        current_message = "BLEペアリングモードに切り替え中...";
      }
      
      if (avatar_initialized) {
        avatar.setSpeechText(current_message.c_str());
      }
      
      delay(1000); // 1秒待機してメッセージを表示
      
      toggleConnectionMode();
    }
    
    // Button C: IP/BLE状態表示
    if (M5.BtnC.wasPressed()) {
      Serial.println("Button C: 状態表示");
      if (connection_mode_ble) {
        current_message = String("BLE: ") + BLE_DEVICE_NAME;
        if (ble_enabled && bleWebUI && bleWebUI->isConnected()) {
          current_message += " (クライアント接続中)";
        } else {
          current_message += " (ペアリング待機中)";
        }
      } else if (wifi_connected) {
        current_message = String("WiFi: ") + current_ip;
      } else {
        current_message = "WiFi未接続";
      }
      avatar.setSpeechText(current_message.c_str());
      Serial.printf("状態表示: %s\n", current_message.c_str());
    }
    
    // 自動まばたき（10秒ごと）
    if (millis() - last_expression_change > 10000) {
      avatar.setExpression(Expression::Neutral);
      last_expression_change = millis();
    }
    
    // セリフ自動ループ処理
    updateSpeechLoop();
    
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

// WebServer設定関数
void setupWebServer() {
  // ルート設定
  server.on("/", handleRoot);
  server.on("/api/expression", HTTP_GET, handleApiExpression);
  server.on("/api/color", HTTP_GET, handleApiColor);
  server.on("/api/setcolor", HTTP_GET, handleApiSetColor);
  server.on("/api/set", HTTP_GET, handleApiSet);
  
  server.onNotFound(handle404);
  
  // サーバー開始
  server.begin();
  Serial.printf("WebServer開始: http://%s/\n", current_ip.c_str());
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
    
    // 接続待機（最大10秒、ボタン割り込み対応）
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && 
           (millis() - start_time) < CONNECTION_TIMEOUT) {
      
      // ボタンチェック（割り込み処理）
      M5.update();
      if (M5.BtnB.wasPressed()) {
        Serial.println("WiFi接続中にBボタン押下 - BLEモードに切り替え");
        WiFi.disconnect();
        return false; // WiFi接続を中止してBLEモードへ
      }
      
      delay(100); // 短い間隔でチェック
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

// 共通WebUI HTML生成関数
String generateWebUIHTML() {
  String html = "<html><head><title>Stack-chan</title>";
  html += "<meta charset='UTF-8'>";
  html += "<style>body{font-family:Arial;margin:20px;} ";
  html += "button{padding:10px;margin:5px;border:none;border-radius:5px;background:#007bff;color:white;cursor:pointer;} ";
  html += "button:hover{background:#0056b3;} ";
  html += "input,select{padding:8px;margin:5px;border:1px solid #ccc;border-radius:3px;} ";
  html += ".expression-control{border:1px solid #ddd;padding:15px;margin:10px 0;border-radius:5px;background:#f9f9f9;} ";
  html += "</style></head><body>";
  html += "<h1>Stack-chan WebUI</h1>";
  
  html += "<div class='expression-control'>";
  html += "<h3>表情とセリフの設定</h3>";
  html += "<select id='expressionSelect'>";
  html += "<option value='0'>普通 (Neutral)</option>";
  html += "<option value='1'>嬉しい (Happy)</option>";
  html += "<option value='2'>眠い (Sleepy)</option>";
  html += "<option value='3'>困った (Doubt)</option>";
  html += "</select>";
  html += "<input type='text' id='speechText' placeholder='セリフを入力してください...' maxlength='50' onkeypress='if(event.key===\"Enter\") setExpressionAndSpeech()'>";
  html += "<button onclick='setExpressionAndSpeech()'>表情とセリフを設定</button>";
  html += "</div>";
  
  html += "<div class='expression-control'>";
  html += "<h3>クイック操作</h3>";
  html += "<button onclick='changeExpression()'>表情サイクル</button> ";
  html += "<button onclick='changeColor()'>色変更</button> ";
  html += "<button onclick='clearSpeech()'>セリフクリア</button>";
  html += "</div>";
  
  html += "<div class='expression-control'>";
  html += "<h3>色テーマ選択</h3>";
  html += "<div style='display:flex;flex-wrap:wrap;gap:5px;'>";
  html += "<button onclick='setColor(0)' style='background:#666;'>標準</button>";
  html += "<button onclick='setColor(1)' style='background:#0066cc;'>青系</button>";
  html += "<button onclick='setColor(2)' style='background:#009900;'>緑系</button>";
  html += "<button onclick='setColor(3)' style='background:#cc0000;'>赤系</button>";
  html += "<button onclick='setColor(4)' style='background:#6600cc;'>紫系</button>";
  html += "<button onclick='setColor(5)' style='background:#ff6600;'>オレンジ</button>";
  html += "</div>";
  html += "</div>";
  
  html += "<h3>System Status</h3>";
  html += "<p>Free Memory: " + String(ESP.getFreeHeap() / 1024) + " KB</p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  
  // 接続モードに応じた情報表示
  if (connection_mode_ble) {
    html += "<p>Connection Mode: BLE</p>";
    html += "<p>BLE Device: " + String(BLE_DEVICE_NAME) + "</p>";
    if (ble_enabled && bleWebUI && bleWebUI->isConnected()) {
      html += "<p>BLE Status: Connected</p>";
    } else {
      html += "<p>BLE Status: Advertising</p>";
    }
  } else {
    html += "<p>Connection Mode: WiFi</p>";
    html += "<p>WiFi SSID: " + WiFi.SSID() + "</p>";
    html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Signal: " + String(WiFi.RSSI()) + " dBm</p>";
  }
  
  html += "<br>";
  html += "<button onclick='location.reload()'>ページ更新</button>";
  
  html += "<script>";
  html += "function showStatus(message, isError = false) {";
  html += "  var status = document.getElementById('status');";
  html += "  if (!status) {";
  html += "    status = document.createElement('div');";
  html += "    status.id = 'status';";
  html += "    status.style.cssText = 'position:fixed;top:10px;right:10px;padding:10px;border-radius:5px;z-index:1000;';";
  html += "    document.body.appendChild(status);";
  html += "  }";
  html += "  status.style.background = isError ? '#dc3545' : '#28a745';";
  html += "  status.style.color = 'white';";
  html += "  status.textContent = message;";
  html += "  setTimeout(() => status.style.display = 'none', 2000);";
  html += "  status.style.display = 'block';";
  html += "}";
  html += "function setExpressionAndSpeech(){";
  html += "  var expr = document.getElementById('expressionSelect').value;";
  html += "  var speech = document.getElementById('speechText').value;";
  html += "  if(!speech) speech = ''; ";
  html += "  fetch('/api/set?expression=' + expr + '&speech=' + encodeURIComponent(speech))";
  html += "    .then(response => response.text())";
  html += "    .then(data => {";
  html += "      showStatus('設定完了');";
  html += "      document.getElementById('speechText').value = '';";
  html += "    })";
  html += "    .catch(error => showStatus('エラーが発生しました', true));";
  html += "}";
  html += "function changeExpression(){fetch('/api/expression').then(()=>showStatus('表情変更')).catch(()=>showStatus('エラー', true));}";
  html += "function changeColor(){fetch('/api/color').then(()=>showStatus('色変更')).catch(()=>showStatus('エラー', true));}";
  html += "function setColor(index){fetch('/api/setcolor?index=' + index).then(()=>showStatus('色設定')).catch(()=>showStatus('エラー', true));}";
  html += "function clearSpeech(){fetch('/api/set?speech=').then(()=>showStatus('セリフクリア')).catch(()=>showStatus('エラー', true));}";
  html += "function playPreset(index){";
  html += "  fetch('/api/preset?index=' + index)";
  html += "    .then(response => response.text())";
  html += "    .then(data => showStatus('プリセット再生'))";
  html += "    .catch(error => showStatus('エラー', true));";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  
  return html;
}

// WebServerハンドラー関数
void handleRoot() {
  String html = generateWebUIHTML();
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
    // 次の色に切り替え（6色をサイクル）
    current_color_index = (current_color_index + 1) % 6;
    
    const char* color_names[] = {
      "標準色", "青系", "緑系", "赤系", "紫系", "オレンジ系"
    };
    
    avatar.setColorPalette(*cps[current_color_index]);
    current_message = String(color_names[current_color_index]);
    
    avatar.setSpeechText(current_message.c_str());
    server.send(200, "text/plain", "Color changed to: " + current_message);
    Serial.println("API: 色変更 -> " + current_message);
  } else {
    server.send(500, "text/plain", "Avatar not initialized");
  }
}

void handleApiSetColor() {
  if (!avatar_initialized) {
    server.send(500, "text/plain", "Avatar not initialized");
    return;
  }
  
  if (!server.hasArg("index")) {
    server.send(400, "text/plain", "Missing index parameter");
    return;
  }
  
  int color_index = server.arg("index").toInt();
  if (color_index < 0 || color_index >= 6) {
    server.send(400, "text/plain", "Invalid color index (0-5)");
    return;
  }
  
  const char* color_names[] = {
    "標準色", "青系", "緑系", "赤系", "紫系", "オレンジ系"
  };
  
  current_color_index = color_index;
  avatar.setColorPalette(*cps[current_color_index]);
  current_message = String(color_names[current_color_index]);
  avatar.setSpeechText(current_message.c_str());
  
  server.send(200, "text/plain", "Color set to: " + current_message);
  Serial.println("API: 色直接設定 -> " + current_message);
}

void handleApiSet() {
  if (!avatar_initialized) {
    server.send(500, "text/plain", "Avatar not initialized");
    return;
  }
  
  String response = "";
  
  // 表情パラメータの処理
  if (server.hasArg("expression")) {
    int expr = server.arg("expression").toInt();
    if (expr >= 0 && expr <= 3) {
      current_expression = expr;
      switch (expr) {
        case 0: 
          avatar.setExpression(Expression::Neutral); 
          response += "表情: 普通";
          break;
        case 1: 
          avatar.setExpression(Expression::Happy); 
          response += "表情: 嬉しい";
          break;
        case 2: 
          avatar.setExpression(Expression::Sleepy); 
          response += "表情: 眠い";
          break;
        case 3: 
          avatar.setExpression(Expression::Doubt); 
          response += "表情: 困った";
          break;
      }
    } else {
      server.send(400, "text/plain", "Invalid expression value (0-3)");
      return;
    }
  }
  
  // セリフパラメータの処理
  if (server.hasArg("speech")) {
    String speech = server.arg("speech");
    current_message = speech;
    avatar.setSpeechText(speech.c_str());
    
    // ユーザーがセリフを設定したことを記録
    speech_set_by_user = (speech.length() > 0);
    last_speech_time = millis();
    
    if (response.length() > 0) response += ", ";
    response += "セリフ: \"" + speech + "\"";
  }
  
  if (response.length() == 0) {
    response = "パラメータが指定されていません";
  }
  
  server.send(200, "text/plain", response);
  Serial.println("API: 設定変更 -> " + response);
}

void handle404() {
  server.send(404, "text/plain", "404 Not Found - Stack-chan WebUI");
}

// ランダムセリフ設定確認
void checkRandomSpeechConfig() {
  // random_speeches配列の最初の要素をチェック
  random_speech_enabled = (random_speeches[0] != nullptr);
  
  if (random_speech_enabled) {
    Serial.println("ランダムセリフ機能: 有効");
    int count = 0;
    while (random_speeches[count] != nullptr) count++;
    Serial.printf("登録されたセリフ数: %d個\n", count);
  } else {
    Serial.println("ランダムセリフ機能: 無効（設定なし）");
  }
}

// ランダムセリフ取得
String getRandomSpeech() {
  if (!random_speech_enabled) return "";
  
  // 配列のサイズを数える
  int count = 0;
  while (random_speeches[count] != nullptr) count++;
  
  if (count == 0) return "";
  
  // ランダムに選択
  int index = random(count);
  return String(random_speeches[index]);
}

// セリフ自動ループ更新
void updateSpeechLoop() {
  if (!avatar_initialized) return;
  
  unsigned long current_time = millis();
  
  // ユーザーがセリフを設定してから30秒経過した場合
  if (speech_set_by_user && 
      (current_time - last_speech_time) > SPEECH_AUTO_CLEAR_TIME) {
    
    Serial.println("セリフ自動クリア（30秒経過）");
    speech_set_by_user = false;
    
    // ランダムセリフが有効な場合は選択、無効な場合は標準メッセージ
    if (random_speech_enabled) {
      current_message = getRandomSpeech();
      Serial.println("ランダムセリフ開始: " + current_message);
    } else {
      current_message = "スタックちゃん";
      Serial.println("標準メッセージに戻る");
    }
    
    avatar.setSpeechText(current_message.c_str());
    last_speech_time = current_time;
  }
  
  // ランダムセリフが有効で、ユーザー設定でない場合の自動ループ
  if (random_speech_enabled && !speech_set_by_user &&
      (current_time - last_speech_time) > SPEECH_AUTO_CLEAR_TIME) {
    
    String new_speech = getRandomSpeech();
    if (new_speech != current_message) {  // 同じセリフの連続を避ける
      current_message = new_speech;
      avatar.setSpeechText(current_message.c_str());
      Serial.println("ランダムセリフ変更: " + current_message);
    }
    last_speech_time = current_time;
  }
}

// BLE初期化関数
void initializeBLE() {
  Serial.println("BLE初期化開始...");
  
  // BLE WebUIハンドラー作成
  bleWebUI = new BLEWebUIHandler();
  bleWebUI->begin();
  
  ble_enabled = true;
  Serial.println("BLE初期化完了");
}

// 通信モード切り替え関数
void toggleConnectionMode() {
  if (connection_mode_ble) {
    // BLE → WiFiモードに切り替え
    Serial.println("BLE → WiFiモードに切り替え中...");
    
    // BLE停止
    if (ble_enabled) {
      if (bleWebUI) {
        delete bleWebUI;
        bleWebUI = nullptr;
      }
      BLEDevice::deinit();
      ble_enabled = false;
    }
    
    connection_mode_ble = false;
    current_message = "WiFi接続中... (Bボタン=BLE切替)";
    if (avatar_initialized) {
      avatar.setSpeechText(current_message.c_str());
    }
    
    // WiFi接続試行（割り込み可能）
    if (connectToWiFi()) {
      setupWebServer();
      current_message = String("WiFi: ") + current_ip;
    } else {
      // WiFi失敗またはBボタン割り込み - BLEモードに戻る
      Serial.println("WiFi接続失敗またはBボタン割り込み - BLEモードに戻ります");
      connection_mode_ble = true;
      current_message = "BLEペアリングモード初期化中...";
      if (avatar_initialized) {
        avatar.setSpeechText(current_message.c_str());
      }
      initializeBLE();
      current_message = "BLE: " + String(BLE_DEVICE_NAME) + " (ペアリング待機中)";
    }
    
  } else {
    // WiFi → BLEモードに切り替え
    Serial.println("WiFi → BLEモードに切り替え中...");
    
    // WiFi停止
    if (wifi_connected) {
      server.stop();
      WiFi.disconnect();
      wifi_connected = false;
      current_ip = "";
    }
    
    connection_mode_ble = true;
    current_message = "BLEペアリングモード初期化中...";
    if (avatar_initialized) {
      avatar.setSpeechText(current_message.c_str());
    }
    
    // BLE開始
    initializeBLE();
    current_message = String("BLE: ") + BLE_DEVICE_NAME + " (ペアリング待機中)";
  }
  
  if (avatar_initialized) {
    avatar.setSpeechText(current_message.c_str());
  }
  
  Serial.println("通信モード切り替え完了: " + String(connection_mode_ble ? "BLE" : "WiFi"));
}

// === BLE WebUI用の外部関数実装 ===

void changeExpressionById(int id) {
  if (!avatar_initialized) return;
  
  if (id == -1) {
    // サイクル変更
    current_expression = (current_expression + 1) % 4;
  } else {
    current_expression = id % 4;
  }
  
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
  speech_set_by_user = true;
  last_speech_time = millis();
  
  Serial.println("BLE経由で表情変更: " + current_message);
}

void changeColorById(int id) {
  if (!avatar_initialized) return;
  
  if (id == -1) {
    // サイクル変更
    current_color_index = (current_color_index + 1) % 6;
  } else {
    current_color_index = id % 6;
  }
  
  avatar.setColorPalette(*cps[current_color_index]);
  
  String color_names[] = {"標準色", "青系", "緑系", "赤系", "紫系", "オレンジ系"};
  current_message = color_names[current_color_index];
  avatar.setSpeechText(current_message.c_str());
  speech_set_by_user = true;
  last_speech_time = millis();
  
  Serial.println("BLE経由で色変更: " + current_message);
}

void setSpeechText(const String& text, int expression) {
  if (!avatar_initialized) return;
  
  // 表情設定
  if (expression >= 0 && expression <= 3) {
    changeExpressionById(expression);
  }
  
  // セリフ設定
  if (text.length() > 0) {
    current_message = text;
    avatar.setSpeechText(current_message.c_str());
    speech_set_by_user = true;
    last_speech_time = millis();
    
    Serial.println("BLE経由でセリフ設定: " + current_message);
  } else {
    // セリフクリア
    current_message = "スタックちゃん";
    avatar.setSpeechText(current_message.c_str());
    speech_set_by_user = false;
    
    Serial.println("BLE経由でセリフクリア");
  }
}

String getSystemStatusJSON() {
  String status = "{";
  status += "\"mode\":\"" + String(connection_mode_ble ? "BLE" : "WiFi") + "\",";
  status += "\"wifi_connected\":" + String(wifi_connected ? "true" : "false") + ",";
  status += "\"ble_enabled\":" + String(ble_enabled ? "true" : "false") + ",";
  
  if (connection_mode_ble && ble_enabled && bleWebUI) {
    status += "\"ble_connected\":" + String(bleWebUI->isConnected() ? "true" : "false") + ",";
  } else {
    status += "\"ble_connected\":false,";
  }
  
  if (wifi_connected) {
    status += "\"ip_address\":\"" + current_ip + "\",";
  } else {
    status += "\"ip_address\":\"\",";
  }
  
  status += "\"current_message\":\"" + current_message + "\",";
  status += "\"expression\":" + String(current_expression) + ",";
  status += "\"color_index\":" + String(current_color_index) + ",";
  status += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
  status += "\"uptime\":" + String(millis() / 1000);
  status += "}";
  
  return status;
}
