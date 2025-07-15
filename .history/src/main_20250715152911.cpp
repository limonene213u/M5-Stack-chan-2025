/*
 * Stack-chan Communication Edition - Phase 1: Basic WiFi
 * 段階的にコミュニケーション機能を追加（安定性重視）
 */

#include <M5Unified.h>
#include <WiFi.h>

// WiFi設定
const char* ssid_list[] = {
  "your_wifi_ssid",      // 設定1: メインWiFi
  "your_backup_wifi",    // 設定2: バックアップWiFi
  nullptr
};
const char* password_list[] = {
  "your_wifi_password",
  "your_backup_password",
  nullptr
};

bool wifi_connected = false;
String wifi_status = "初期化中...";

// 日本語フォント表示テスト用関数
void displayJapaneseTest() {
  M5.Display.fillScreen(TFT_BLACK);
  
  // AGENTS.mdに基づく正しいAPI使用
  M5.Lcd.setTextFont(&fonts::efontJA_16);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE);
  
  M5.Display.setCursor(10, 10);
  M5.Lcd.println("スタックチャン");
  M5.Display.setCursor(10, 40);
  M5.Lcd.println("通信版 Phase1");
  M5.Display.setCursor(10, 70);
  M5.Lcd.println(wifi_status);
}

// WiFi接続関数
void connectWiFi() {
  wifi_status = "WiFi接続中...";
  displayJapaneseTest();
  
  for (int i = 0; ssid_list[i] != nullptr; i++) {
    Serial.printf("WiFi接続試行: %s\n", ssid_list[i]);
    WiFi.begin(ssid_list[i], password_list[i]);
    
    int retry_count = 0;
    while (WiFi.status() != WL_CONNECTED && retry_count < 20) {
      delay(500);
      Serial.print(".");
      retry_count++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      wifi_connected = true;
      wifi_status = "WiFi接続成功!";
      Serial.printf("\nWiFi接続成功: %s\n", WiFi.localIP().toString().c_str());
      break;
    } else {
      Serial.printf("\nWiFi接続失敗: %s\n", ssid_list[i]);
    }
  }
  
  if (!wifi_connected) {
    wifi_status = "WiFi接続失敗";
    Serial.println("全てのWiFi接続に失敗");
  }
  
  displayJapaneseTest();
}

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
