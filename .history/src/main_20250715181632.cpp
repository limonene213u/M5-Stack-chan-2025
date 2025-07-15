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

void setup() {
  // M5Stack基本初期化のみ
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("=== Stack-chan 最小構成 Avatar専用版 ===");
  
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
    Serial.println("✅ Avatar初期化成功");
    
  } catch (...) {
    Serial.println("❌ Avatar初期化失敗");
    avatar_initialized = false;
    
    // フォールバック表示
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Avatar Error");
    M5.Display.println("Basic Mode");
  }
  
  Serial.println("初期化完了 - Avatar専用モード");
}

void loop() {
  M5.update();
  
  if (avatar_initialized) {
    // Button A: 表情変更（4種類をサイクル）
    if (M5.BtnA.wasPressed()) {
      Serial.println("🔄 表情変更");
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
    
    // Button B: 色変更（標準⇔青）
    if (M5.BtnB.wasPressed()) {
      Serial.println("🎨 色変更");
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
    }
    
    // Button C: 話すアニメーション
    if (M5.BtnC.wasPressed()) {
      Serial.println("💬 話すアニメーション");
      current_message = "話してます";
      avatar.setSpeechText(current_message.c_str());
      
      // 口の動きアニメーション（最小限）
      for (int i = 0; i < 3; i++) {
        avatar.setMouthOpenRatio(0.7);
        delay(150);
        avatar.setMouthOpenRatio(0.0);
        delay(150);
      }
      
      current_message = "話し終わり";
      avatar.setSpeechText(current_message.c_str());
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
      M5.Display.println("Button B");
      delay(500);
    }
    
    if (M5.BtnC.wasPressed()) {
      M5.Display.fillScreen(TFT_YELLOW);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Button C");
      delay(500);
    }
  }
  
  // 最小限のシステム監視（10秒ごと）
  static unsigned long last_heartbeat = 0;
  if (millis() - last_heartbeat > 10000) {
    Serial.printf("💓 Avatar=%s, Memory=%dKB, Uptime=%lus\n", 
                  avatar_initialized ? "OK" : "NG", 
                  ESP.getFreeHeap() / 1024, 
                  millis() / 1000);
    last_heartbeat = millis();
  }
  
  delay(50);
}
