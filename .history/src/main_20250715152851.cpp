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
  
  // Button test
  if (M5.BtnA.wasPressed()) {
    Serial.println("Button A pressed");
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Button A");
    delay(1000);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack");
    M5.Display.println("Test");
    M5.Display.println("OK!");
  }
  
  if (M5.BtnB.wasPressed()) {
    Serial.println("Button B pressed");
    M5.Display.fillScreen(TFT_GREEN);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Button B");
    delay(1000);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack");
    M5.Display.println("Test");
    M5.Display.println("OK!");
  }
  
  if (M5.BtnC.wasPressed()) {
    Serial.println("Button C pressed");
    M5.Display.fillScreen(TFT_YELLOW);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Button C");
    delay(1000);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack");
    M5.Display.println("Test");
    M5.Display.println("OK!");
  }
  
  // Heart beat
  static unsigned long last_print = 0;
  if (millis() - last_print > 5000) {
    Serial.println("System running...");
    last_print = millis();
  }
  
  delay(50);
}
