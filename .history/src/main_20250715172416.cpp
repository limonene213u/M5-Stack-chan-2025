/*
 * Stack-chan 最小構成 with Avatar
 * Avatar表示機能付きの最小実装
 */

#include <M5Unified.h>
#include <Avatar.h>

using namespace m5avatar;

// Avatar関連
Avatar avatar;
ColorPalette* cps[2];
bool avatar_initialized = false;

// 状態管理
String current_message = "こんにちは";
unsigned long last_expression_change = 0;
int current_expression = 0;

void setup() {
  // M5Stack初期化
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("=== Stack-chan 最小構成 with Avatar ===");
  
  // 画面クリア
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Avatar初期化中...");
  
  Serial.println("Avatar初期化開始");
  
  try {
    // ColorPalette初期化
    cps[0] = new ColorPalette();
    cps[1] = new ColorPalette();
    cps[1]->set(COLOR_PRIMARY, TFT_YELLOW);
    cps[1]->set(COLOR_BACKGROUND, TFT_BLUE);
    
    // Avatar初期化（AGENTS.mdの安定化パターンに基づく）
    avatar.init();
    avatar.setColorPalette(*cps[0]);
    
    // 日本語フォント設定（AGENTS.mdの正しいAPI使用）
    avatar.setSpeechFont(&fonts::efontJA_16);
    
    // 基本表情設定
    avatar.setExpression(Expression::Neutral);
    avatar.setSpeechText(current_message.c_str());
    
    avatar_initialized = true;
    Serial.println("Avatar初期化成功");
    
  } catch (...) {
    Serial.println("Avatar初期化でエラーが発生しました");
    avatar_initialized = false;
    
    // フォールバック表示
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Avatar Error");
    M5.Display.println("Fallback Mode");
  }
  
  Serial.println("初期化完了");
}

void loop() {
  M5.update();
  
  // Button A: WiFi再接続
  if (M5.BtnA.wasPressed()) {
    Serial.println("Button A pressed - WiFi再接続");
    wifi_connected = false;
    connectWiFi();
  }
  
  // Button B: 日本語フォントテスト再実行
  if (M5.BtnB.wasPressed()) {
    Serial.println("Button B pressed - 日本語フォントテスト");
    displayJapaneseTest();
  }
  
  // Button C: システム情報表示
  if (M5.BtnC.wasPressed()) {
    Serial.println("Button C pressed - システム情報");
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, 10);
    
    if (wifi_connected) {
      M5.Display.printf("WiFi: %s\n", WiFi.localIP().toString().c_str());
      M5.Display.printf("SSID: %s\n", WiFi.SSID().c_str());
      M5.Display.printf("RSSI: %d dBm\n", WiFi.RSSI());
    } else {
      M5.Display.println("WiFi: 未接続");
    }
    
    M5.Display.printf("フリーメモリ: %d KB\n", ESP.getFreeHeap() / 1024);
    M5.Display.printf("アップタイム: %lu 秒\n", millis() / 1000);
  }
  
  // WiFi接続状態の監視
  static unsigned long last_wifi_check = 0;
  if (millis() - last_wifi_check > 10000) {  // 10秒ごとにチェック
    if (wifi_connected && WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi接続が切断されました");
      wifi_connected = false;
      wifi_status = "WiFi切断検出";
      displayJapaneseTest();
    }
    last_wifi_check = millis();
  }
  
  // ハートビート
  static unsigned long last_print = 0;
  if (millis() - last_print > 5000) {
    Serial.printf("System running... WiFi: %s\n", wifi_connected ? "接続" : "未接続");
    last_print = millis();
  }
  
  delay(50);
}
