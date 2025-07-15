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
  
  if (avatar_initialized) {
    // Button A: 表情変更
    if (M5.BtnA.wasPressed()) {
      Serial.println("Button A pressed - 表情変更");
      current_expression = (current_expression + 1) % 4;
      
      switch (current_expression) {
        case 0:
          avatar.setExpression(Expression::Neutral);
          current_message = "普通の顔";
          break;
        case 1:
          avatar.setExpression(Expression::Happy);
          current_message = "嬉しい顔";
          break;
        case 2:
          avatar.setExpression(Expression::Sleepy);
          current_message = "眠そうな顔";
          break;
        case 3:
          avatar.setExpression(Expression::Doubt);
          current_message = "困った顔";
          break;
      }
      
      avatar.setSpeechText(current_message.c_str());
      Serial.printf("表情変更: %s\n", current_message.c_str());
    }
    
    // Button B: 色変更
    if (M5.BtnB.wasPressed()) {
      Serial.println("Button B pressed - 色変更");
      static bool use_alt_color = false;
      use_alt_color = !use_alt_color;
      
      if (use_alt_color) {
        avatar.setColorPalette(*cps[1]);
        current_message = "色変更：青";
      } else {
        avatar.setColorPalette(*cps[0]);
        current_message = "色変更：標準";
      }
      
      avatar.setSpeechText(current_message.c_str());
    }
    
    // Button C: 口の動き
    if (M5.BtnC.wasPressed()) {
      Serial.println("Button C pressed - 口の動き");
      current_message = "話しています";
      avatar.setSpeechText(current_message.c_str());
      
      // 簡単な口の動きアニメーション
      for (int i = 0; i < 5; i++) {
        avatar.setMouthOpenRatio(0.8);
        delay(200);
        avatar.setMouthOpenRatio(0.2);
        delay(200);
      }
      avatar.setMouthOpenRatio(0.0);
      
      current_message = "話し終わり";
      avatar.setSpeechText(current_message.c_str());
    }
    
    // 自動表情変更（10秒ごと）
    if (millis() - last_expression_change > 10000) {
      avatar.setExpression(Expression::Neutral);
      last_expression_change = millis();
    }
    
  } else {
    // Avatar無効時のフォールバック操作
    if (M5.BtnA.wasPressed()) {
      M5.Display.fillScreen(TFT_GREEN);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Button A");
      M5.Display.println("(No Avatar)");
    }
    
    if (M5.BtnB.wasPressed()) {
      M5.Display.fillScreen(TFT_BLUE);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Button B");
      M5.Display.println("(No Avatar)");
    }
    
    if (M5.BtnC.wasPressed()) {
      M5.Display.fillScreen(TFT_YELLOW);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Button C");
      M5.Display.println("(No Avatar)");
    }
  }
  
  // システム情報出力（5秒ごと）
  static unsigned long last_print = 0;
  if (millis() - last_print > 5000) {
    Serial.printf("システム状態: Avatar=%s, フリーメモリ=%d KB, アップタイム=%lu秒\n", 
                  avatar_initialized ? "有効" : "無効", 
                  ESP.getFreeHeap() / 1024, 
                  millis() / 1000);
    last_print = millis();
  }
  
  delay(50);
}
