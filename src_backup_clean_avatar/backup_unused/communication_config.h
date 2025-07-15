/*
 * Communication Configuration Helper
 * WiFi Multi-AP and Access Point settings loader
 */

#ifndef _COMMUNICATION_CONFIG_H
#define _COMMUNICATION_CONFIG_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include <WiFi.h>
#include <Stackchan_system_config.h>

#define MAX_WIFI_NETWORKS 10

struct WiFiNetwork {
  String ssid;
  String password;
  int priority;
  
  WiFiNetwork() : ssid(""), password(""), priority(0) {}
  WiFiNetwork(String s, String p, int pr) : ssid(s), password(p), priority(pr) {}
  
  bool isValid() const {
    return !ssid.isEmpty() && priority > 0;
  }
};

struct APModeConfig {
  bool enable;
  String ssid;
  String password;
  int channel;
  int max_connections;
  bool auto_fallback;
  
  APModeConfig() {
    enable = false;
    ssid = "Stack-chan";
    password = "Stack-chan-88";
    channel = 1;
    max_connections = 4;
    auto_fallback = true;
  }
};

struct AudioConfig {
  bool enable;
  int volume;
  String format;
  int sample_rate;
  String voice_files_path;
  bool auto_speech;
  uint32_t speech_interval;
  
  AudioConfig() {
    enable = false;
    volume = 80;
    format = "wav";
    sample_rate = 16000;
    voice_files_path = "/audio/";
    auto_speech = false;
    speech_interval = 30000;
  }
};

struct CommunicationConfig {
  // WiFi設定
  std::vector<WiFiNetwork> wifi_networks;
  APModeConfig ap_mode;
  uint32_t connection_timeout;
  int retry_count;
  uint32_t scan_timeout;
  
  // Webサーバー設定
  int webserver_port;
  bool enable_cors;
  
  // Bluetooth設定
  String bluetooth_device_name;
  bool bluetooth_starting_state;
  bool enable_classic;
  bool enable_ble;
  
  // 通信設定
  String communication_mode;
  bool auto_response;
  uint32_t message_timeout;
  
  // 音声設定
  AudioConfig audio;
  
  // 言語設定
  String language;
  std::vector<String> lyrics;
  
  CommunicationConfig() {
    // WiFiデフォルト値
    wifi_networks.reserve(MAX_WIFI_NETWORKS);
    connection_timeout = 10000;
    retry_count = 3;
    scan_timeout = 5000;
    
    // Webサーバーデフォルト値
    webserver_port = 80;
    enable_cors = true;
    
    // Bluetoothデフォルト値
    bluetooth_device_name = "StackChan-Comm";
    bluetooth_starting_state = true;
    enable_classic = true;
    enable_ble = false;
    
    // 通信デフォルト値
    communication_mode = "both";
    auto_response = true;
    message_timeout = 30000;
    
    // 言語デフォルト値
    language = "JA";
    initDefaultLyrics();
  }
  
  void initDefaultLyrics() {
    lyrics = {
      "こんにちは",
      "今日もいい天気ですね", 
      "何かお手伝いできることはありますか？",
      "スタックチャンです",
      "WiFi接続を確認中...",
      "Bluetooth待機中...",
      "準備完了です！",
      "メッセージをお待ちしています"
    };
  }
  
  void addWiFiNetwork(const String& ssid, const String& password, int priority = 1) {
    if (wifi_networks.size() < MAX_WIFI_NETWORKS && !ssid.isEmpty()) {
      wifi_networks.emplace_back(ssid, password, priority);
      sortNetworksByPriority();
    }
  }
  
  void removeWiFiNetwork(const String& ssid) {
    wifi_networks.erase(
      std::remove_if(wifi_networks.begin(), wifi_networks.end(),
        [&ssid](const WiFiNetwork& net) { return net.ssid == ssid; }),
      wifi_networks.end()
    );
  }
  
  void sortNetworksByPriority() {
    std::sort(wifi_networks.begin(), wifi_networks.end(),
      [](const WiFiNetwork& a, const WiFiNetwork& b) {
        return a.priority < b.priority; // 小さい数字が高優先度
      });
  }
  
  std::vector<WiFiNetwork> getValidNetworks() const {
    std::vector<WiFiNetwork> valid;
    for (const auto& net : wifi_networks) {
      if (net.isValid()) {
        valid.push_back(net);
      }
    }
    return valid;
  }
  
  void loadFromSystemConfig(StackchanSystemConfig& system_config) {
    // stackchan-arduinoライブラリからの設定読み込み
    // 実際の実装では、system_configのAPIに合わせて調整が必要
    
    // 基本設定から言語設定を読み込み
    // language = system_config.getBalloonConfig().font_language;
    
    // デフォルト設定でサンプルネットワークを追加
    addWiFiNetwork("Home-WiFi", "password123", 1);
    addWiFiNetwork("Office-WiFi", "office456", 2);
  }
};

#endif
