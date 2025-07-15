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
void handleApiSet();
void handle404();

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
  
  // Avatar初期化をテスト無効化
  Serial.println("Avatar初期化スキップ - テストモード");
  M5.Display.fillScreen(TFT_GREEN);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Avatar Disabled");
  M5.Display.println("Test Mode");
  M5.Display.println("Check Serial");
  
  avatar_initialized = false;  // Avatarは使用しない
  
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
  html += "<input type='text' id='speechText' placeholder='セリフを入力してください...' maxlength='50'>";
  html += "<button onclick='setExpressionAndSpeech()'>表情とセリフを設定</button>";
  html += "</div>";
  
  html += "<div class='expression-control'>";
  html += "<h3>クイック操作</h3>";
  html += "<button onclick='changeExpression()'>表情サイクル</button> ";
  html += "<button onclick='changeColor()'>色変更</button> ";
  html += "<button onclick='clearSpeech()'>セリフクリア</button>";
  html += "</div>";
  
  html += "<div class='expression-control'>";
  html += "<h3>SDカード機能</h3>";
  if (sd_initialized) {
    html += "<p style='color:green;'>SDカード: 利用可能</p>";
    html += "<button onclick='location.href=\"/upload\"'>ファイルアップロード</button> ";
    html += "<button onclick='location.href=\"/files\"'>ファイル一覧</button>";
  } else {
    html += "<p style='color:red;'>SDカード: 利用不可</p>";
  }
  html += "</div>";
  
  if (sd_initialized) {
    html += "<div class='expression-control'>";
    html += "<h3>音声プリセット</h3>";
    html += "<p>プリセットされた音声付きセリフを再生できます</p>";
    
    // プリセット一覧表示
    int preset_count = 0;
    for (int i = 0; voice_presets[i].text != nullptr; i++) {
      preset_count++;
      html += "<button onclick='playPreset(" + String(i) + ")'>" + String(voice_presets[i].text) + "</button> ";
      if (preset_count % 3 == 0) html += "<br>"; // 3つずつ改行
    }
    html += "</div>";
  }
  
  html += "<h3>System Status</h3>";
  html += "<p>Free Memory: " + String(ESP.getFreeHeap() / 1024) + " KB</p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
  html += "<p>WiFi SSID: " + WiFi.SSID() + "</p>";
  html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Signal: " + String(WiFi.RSSI()) + " dBm</p>";
  html += "<br>";
  html += "<button onclick='location.reload()'>ページ更新</button>";
  
  html += "<script>";
  html += "function setExpressionAndSpeech(){";
  html += "  var expr = document.getElementById('expressionSelect').value;";
  html += "  var speech = document.getElementById('speechText').value;";
  html += "  if(!speech) speech = ''; ";
  html += "  fetch('/api/set?expression=' + expr + '&speech=' + encodeURIComponent(speech))";
  html += "    .then(response => response.text())";
  html += "    .then(data => alert('設定完了: ' + data))";
  html += "    .catch(error => alert('エラー: ' + error));";
  html += "}";
  html += "function changeExpression(){fetch('/api/expression').then(()=>alert('表情変更')).catch(()=>alert('エラー'));}";
  html += "function changeColor(){fetch('/api/color').then(()=>alert('色変更')).catch(()=>alert('エラー'));}";
  html += "function clearSpeech(){fetch('/api/set?speech=').then(()=>alert('セリフクリア')).catch(()=>alert('エラー'));}";
  html += "function playPreset(index){";
  html += "  fetch('/api/preset?index=' + index)";
  html += "    .then(response => response.text())";
  html += "    .then(data => console.log('プリセット再生: ' + data))";
  html += "    .catch(error => alert('エラー: ' + error));";
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
    
    if (response.length() > 0) response += ", ";
    response += "セリフ: \"" + speech + "\"";
  }
  
  if (response.length() == 0) {
    response = "パラメータが指定されていません";
  }
  
  server.send(200, "text/plain", response);
  Serial.println("API: 設定変更 -> " + response);
}

// SDカード初期化関数
bool initializeSD() {
  if (!SD.begin()) {
    Serial.println("SDカード初期化失敗");
    return false;
  }
  
  Serial.println("SDカード初期化成功");
  
  // audioディレクトリを作成
  if (!SD.exists("/audio")) {
    if (SD.mkdir("/audio")) {
      Serial.println("audioディレクトリ作成成功");
    } else {
      Serial.println("audioディレクトリ作成失敗");
    }
  }
  
  // voiceディレクトリを作成（プリセット音声用）
  if (!SD.exists("/voice")) {
    if (SD.mkdir("/voice")) {
      Serial.println("voiceディレクトリ作成成功");
    } else {
      Serial.println("voiceディレクトリ作成失敗");
    }
  }
  
  return true;
}

// ファイルアップロードページ
void handleFileUpload() {
  if (!sd_initialized) {
    server.send(500, "text/html", 
      "<h1>Error</h1><p>SDカードが利用できません</p>"
      "<p><a href='/'>戻る</a></p>");
    return;
  }
  
  String html = "<html><head><title>ファイルアップロード</title>";
  html += "<meta charset='UTF-8'>";
  html += "<style>body{font-family:Arial;margin:20px;} ";
  html += "button{padding:10px;margin:5px;border:none;border-radius:5px;background:#007bff;color:white;cursor:pointer;} ";
  html += "input[type=file]{padding:10px;margin:10px 0;} ";
  html += ".upload-area{border:2px dashed #ccc;padding:20px;margin:20px 0;text-align:center;} ";
  html += "</style></head><body>";
  html += "<h1>音声ファイルアップロード</h1>";
  html += "<p>音声ファイル（.wav, .mp3等）をアップロードできます</p>";
  
  html += "<form method='POST' enctype='multipart/form-data' action='/upload'>";
  html += "<div class='upload-area'>";
  html += "<input type='file' name='file' accept='audio/*' required>";
  html += "<br><br>";
  html += "<button type='submit'>アップロード</button>";
  html += "</div>";
  html += "</form>";
  
  html += "<p><a href='/files'>ファイル一覧を見る</a></p>";
  html += "<p><a href='/'>メインページに戻る</a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// ファイルアップロード処理
void handleFileUploadPost() {
  if (!sd_initialized) {
    server.send(500, "text/plain", "SDカードが利用できません");
    return;
  }
  
  HTTPUpload& upload = server.upload();
  static File uploadFile;
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/audio/" + upload.filename;
    Serial.printf("ファイルアップロード開始: %s\n", filename.c_str());
    
    // 既存ファイルがあれば削除
    if (SD.exists(filename)) {
      SD.remove(filename);
    }
    
    uploadFile = SD.open(filename, FILE_WRITE);
    if (!uploadFile) {
      Serial.println("ファイル作成失敗");
      return;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      Serial.printf("アップロード完了: %s (%d bytes)\n", upload.filename.c_str(), upload.totalSize);
      
      String html = "<html><head><title>アップロード完了</title>";
      html += "<meta charset='UTF-8'></head><body>";
      html += "<h1>アップロード完了</h1>";
      html += "<p>ファイル: " + upload.filename + "</p>";
      html += "<p>サイズ: " + String(upload.totalSize) + " bytes</p>";
      html += "<p><a href='/upload'>続けてアップロード</a></p>";
      html += "<p><a href='/files'>ファイル一覧を見る</a></p>";
      html += "<p><a href='/'>メインページに戻る</a></p>";
      html += "</body></html>";
      
      server.send(200, "text/html", html);
      
      // Avatar表示更新
      if (avatar_initialized) {
        current_message = "ファイルアップロード完了";
        avatar.setSpeechText(current_message.c_str());
      }
    }
  }
}

// ファイル一覧表示
void handleFileList() {
  if (!sd_initialized) {
    server.send(500, "text/html", 
      "<h1>Error</h1><p>SDカードが利用できません</p>"
      "<p><a href='/'>戻る</a></p>");
    return;
  }
  
  String html = "<html><head><title>ファイル一覧</title>";
  html += "<meta charset='UTF-8'>";
  html += "<style>body{font-family:Arial;margin:20px;} ";
  html += "table{border-collapse:collapse;width:100%;} ";
  html += "th,td{border:1px solid #ddd;padding:8px;text-align:left;} ";
  html += "th{background-color:#f2f2f2;} ";
  html += "button{padding:5px;margin:2px;border:none;border-radius:3px;background:#dc3545;color:white;cursor:pointer;} ";
  html += "</style></head><body>";
  html += "<h1>音声ファイル一覧</h1>";
  
  File root = SD.open("/audio");
  if (!root || !root.isDirectory()) {
    html += "<p>audioディレクトリが見つかりません</p>";
  } else {
    html += "<table>";
    html += "<tr><th>ファイル名</th><th>サイズ</th><th>操作</th></tr>";
    
    File file = root.openNextFile();
    int fileCount = 0;
    while (file) {
      if (!file.isDirectory()) {
        html += "<tr>";
        html += "<td>" + String(file.name()) + "</td>";
        html += "<td>" + String(file.size()) + " bytes</td>";
        html += "<td><button onclick='deleteFile(\"" + String(file.name()) + "\")'>削除</button></td>";
        html += "</tr>";
        fileCount++;
      }
      file = root.openNextFile();
    }
    root.close();
    
    if (fileCount == 0) {
      html += "<tr><td colspan='3'>ファイルがありません</td></tr>";
    }
    html += "</table>";
  }
  
  html += "<br>";
  html += "<p><a href='/upload'>ファイルアップロード</a></p>";
  html += "<p><a href='/'>メインページに戻る</a></p>";
  
  html += "<script>";
  html += "function deleteFile(filename) {";
  html += "  if (confirm('ファイル \"' + filename + '\" を削除しますか？')) {";
  html += "    location.href = '/delete?file=' + encodeURIComponent(filename);";
  html += "  }";
  html += "}";
  html += "</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// ファイル削除
void handleFileDelete() {
  if (!sd_initialized) {
    server.send(500, "text/plain", "SDカードが利用できません");
    return;
  }
  
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "ファイル名が指定されていません");
    return;
  }
  
  String filename = "/audio/" + server.arg("file");
  
  if (SD.exists(filename)) {
    if (SD.remove(filename)) {
      Serial.printf("ファイル削除成功: %s\n", filename.c_str());
      
      // Avatar表示更新
      if (avatar_initialized) {
        current_message = "ファイル削除完了";
        avatar.setSpeechText(current_message.c_str());
      }
      
      // ファイル一覧ページにリダイレクト
      server.sendHeader("Location", "/files");
      server.send(302);
    } else {
      server.send(500, "text/plain", "ファイル削除失敗");
    }
  } else {
    server.send(404, "text/plain", "ファイルが見つかりません");
  }
}

// 音声再生関数（シンプル版）
bool playVoiceFile(const char* filename) {
  if (!sd_initialized) {
    Serial.println("SDカードが初期化されていません");
    return false;
  }
  
  String filepath = "/voice/" + String(filename);
  
  if (!SD.exists(filepath)) {
    Serial.printf("音声ファイルが見つかりません: %s\n", filepath.c_str());
    return false;
  }
  
  // M5Unified のスピーカー機能を使用
  File audioFile = SD.open(filepath);
  if (!audioFile) {
    Serial.printf("ファイルオープン失敗: %s\n", filepath.c_str());
    return false;
  }
  
  Serial.printf("音声再生開始: %s\n", filepath.c_str());
  
  // ファイルサイズを確認
  size_t fileSize = audioFile.size();
  Serial.printf("ファイルサイズ: %d bytes\n", fileSize);
  
  // 簡易WAVファイル再生（M5Unified音声機能を使用）
  // 注：実際の実装では音声フォーマットの解析が必要
  audioFile.close();
  
  // M5のスピーカーでビープ音を鳴らす（プレースホルダー）
  M5.Speaker.tone(1000, 200);
  delay(250);
  M5.Speaker.tone(800, 200);
  delay(250);
  
  Serial.println("音声再生完了（簡易版）");
  return true;
}

// プリセット音声再生APIハンドラー
void handleApiPreset() {
  if (!avatar_initialized) {
    server.send(500, "text/plain", "Avatar not initialized");
    return;
  }
  
  if (!server.hasArg("index")) {
    server.send(400, "text/plain", "プリセットインデックスが指定されていません");
    return;
  }
  
  int index = server.arg("index").toInt();
  
  // インデックスの有効性をチェック
  int preset_count = 0;
  while (voice_presets[preset_count].text != nullptr) {
    preset_count++;
  }
  
  if (index < 0 || index >= preset_count) {
    server.send(400, "text/plain", "無効なプリセットインデックス");
    return;
  }
  
  const VoicePreset& preset = voice_presets[index];
  
  // 表情を設定
  current_expression = preset.expression;
  switch (preset.expression) {
    case 0: avatar.setExpression(Expression::Neutral); break;
    case 1: avatar.setExpression(Expression::Happy); break;
    case 2: avatar.setExpression(Expression::Sleepy); break;
    case 3: avatar.setExpression(Expression::Doubt); break;
  }
  
  // セリフを表示
  current_message = String(preset.text);
  avatar.setSpeechText(current_message.c_str());
  
  // 音声ファイルを再生
  bool voice_played = playVoiceFile(preset.voice_file);
  
  String response = "プリセット再生: " + String(preset.text);
  if (voice_played) {
    response += " (音声再生成功)";
  } else {
    response += " (音声ファイルなし)";
  }
  
  server.send(200, "text/plain", response);
  Serial.println("API: " + response);
}

void handle404() {
  server.send(404, "text/plain", "404 Not Found - Stack-chan WebUI");
}
