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
  // M5Stack basic initialization
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("=== Stack-chan Communication Edition Phase 1 ===");
  
  // 初期画面表示（日本語フォントテスト）
  displayJapaneseTest();
  
  Serial.println("日本語フォント表示テスト完了");
  delay(2000);
  
  // WiFi接続開始
  connectWiFi();
  
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
