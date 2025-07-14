/*
 * Communication Configuration Helper
 * WiFi and Bluetooth settings loader
 */

#ifndef _COMMUNICATION_CONFIG_H
#define _COMMUNICATION_CONFIG_H

#include <Arduino.h>
#include <Stackchan_system_config.h>

struct CommunicationConfig {
  // WiFi設定
  String wifi_ssid;
  String wifi_password;
  bool enable_ap_mode;
  String ap_ssid;
  String ap_password;
  
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
  
  CommunicationConfig() {
    // デフォルト値
    wifi_ssid = "YourWiFiSSID";
    wifi_password = "YourWiFiPassword";
    enable_ap_mode = false;
    ap_ssid = "StackChan-AP";
    ap_password = "stackchan123";
    
    webserver_port = 80;
    enable_cors = true;
    
    bluetooth_device_name = "StackChan-Comm";
    bluetooth_starting_state = true;
    enable_classic = true;
    enable_ble = false;
    
    communication_mode = "both";
    auto_response = true;
    message_timeout = 30000;
  }
  
  void loadFromSystemConfig(StackchanSystemConfig& system_config) {
    // 基本設定から読み込み（可能な範囲で）
    // StackchanSystemConfigから通信設定を読み込む
    // 実際の実装では、system_configのAPIに合わせて調整が必要
    
    // 例：YAMLファイルから直接読み込む場合
    // この部分は実際のstackchan-arduinoライブラリのAPIに合わせて実装
  }
};

#endif
