/*
 * Simple WiFi Configuration for Stack-chan
 * YAMLファイルを使わずに直接設定する軽量版
 */

#ifndef SIMPLE_WIFI_CONFIG_H
#define SIMPLE_WIFI_CONFIG_H

#include <Arduino.h>

// WiFi設定構造体
struct WiFiCredentials {
  const char* ssid;
  const char* password;
  int priority;
};

// WiFi設定一覧（優先順位順）
// ここを編集してWiFi設定を変更してください
const WiFiCredentials wifi_networks[] = {
  {"cisco-aktk-6", "ti463dhevu57n", 1},
  {"cisco-aktk", "ti463dhevu57n", 2}, 
  {"りもiPhone", "ti463dhevu57n", 3},
  {nullptr, nullptr, 0}  // 終端マーカー
};

// WebServer設定
const int WEBSERVER_PORT = 80;
const char* HOSTNAME = "stackchan";
const unsigned long CONNECTION_TIMEOUT = 10000;  // 10秒
const unsigned long RETRY_INTERVAL = 5000;       // 5秒

#endif // SIMPLE_WIFI_CONFIG_H
