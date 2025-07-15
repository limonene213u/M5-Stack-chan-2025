/*
 * M5Stack Simple Test for Arduino IDE
 * 最小構成でのテスト用コード
 */

#include <M5Unified.h>

void setup() {
  // M5Stack basic initialization
  auto cfg = M5.config();
  M5.begin(cfg);
  
  Serial.begin(115200);
  Serial.println("=== M5Stack Arduino IDE Test ===");
  
  // Display test
  M5.Display.fillScreen(TFT_BLUE);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(10, 10);
  M5.Display.println("M5Stack");
  M5.Display.println("Arduino IDE");
  M5.Display.println("Test OK!");
  
  Serial.println("Display initialized successfully");
}

void loop() {
  M5.update();
  
  // Button test
  if (M5.BtnA.wasPressed()) {
    Serial.println("Button A pressed");
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 50);
    M5.Display.println("Button A");
    delay(1000);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack");
    M5.Display.println("Arduino IDE");
    M5.Display.println("Test OK!");
  }
  
  if (M5.BtnB.wasPressed()) {
    Serial.println("Button B pressed");
    M5.Display.fillScreen(TFT_GREEN);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 50);
    M5.Display.println("Button B");
    delay(1000);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack");
    M5.Display.println("Arduino IDE");
    M5.Display.println("Test OK!");
  }
  
  if (M5.BtnC.wasPressed()) {
    Serial.println("Button C pressed");
    M5.Display.fillScreen(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 50);
    M5.Display.println("Button C");
    delay(1000);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack");
    M5.Display.println("Arduino IDE");
    M5.Display.println("Test OK!");
  }
  
  // Heart beat
  static unsigned long last_print = 0;
  if (millis() - last_print > 5000) {
    Serial.println("System running...");
    last_print = millis();
  }
  
  delay(50);
}
