/*
 * Stack-chan シンプル構成 Avatar + WiFi + WebServer
 * Avatar表示 + WiFi接続 + 基本WebUI
 */

#include <M5Unified.h>
#include <Avatar.h>
#include <WiFi.h>
#include <WebServer.h>
#include "simple_wifi_config.h"

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

// 表示制御
String current_message = "スタックちゃん";
unsigned long last_expression_change = 0;
int current_expression = 0;

// セリフ自動制御
unsigned long last_speech_time = 0;
bool speech_set_by_user = false;
bool random_speech_enabled = false;

// 色制御
int current_color_index = 0;

// 関数プロトタイプ宣言
bool connectToWiFi();
void handleRoot();
void handleApiExpression();
void handleApiColor();
void handleApiSet();
void handle404();
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
    avatar.setColorPalette(*cps[0]);
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
    server.on("/api/set", HTTP_GET, handleApiSet);
    
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
  
  // ランダムセリフ設定確認
  checkRandomSpeechConfig();
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
  
  html += "<h3>System Status</h3>";
  html += "<p>Free Memory: " + String(ESP.getFreeHeap() / 1024) + " KB</p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  html += "<p>WiFi SSID: " + WiFi.SSID() + "</p>";
  html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Signal: " + String(WiFi.RSSI()) + " dBm</p>";
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
  html += "function clearSpeech(){fetch('/api/set?speech=').then(()=>showStatus('セリフクリア')).catch(()=>showStatus('エラー', true));}";
  html += "function playPreset(index){";
  html += "  fetch('/api/preset?index=' + index)";
  html += "    .then(response => response.text())";
  html += "    .then(data => showStatus('プリセット再生'))";
  html += "    .catch(error => showStatus('エラー', true));";
  html += "}";
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
