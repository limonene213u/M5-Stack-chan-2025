/*
 * WiFi Manager for Stack-chan
 * Multi-AP support and Access Point mode management
 */

#ifndef _WIFI_MANAGER_H
#define _WIFI_MANAGER_H

#include <WiFi.h>
#include <vector>
#include "communication_config.h"

class WiFiManager {
private:
  CommunicationConfig* config;
  bool is_ap_mode;
  bool is_connected;
  String current_ssid;
  int current_attempt;
  unsigned long last_scan_time;
  std::vector<String> available_networks;

public:
  WiFiManager(CommunicationConfig* cfg) : config(cfg) {
    is_ap_mode = false;
    is_connected = false;
    current_attempt = 0;
    last_scan_time = 0;
  }

  bool begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // 利用可能ネットワークをスキャン
    scanNetworks();
    
    // 設定されたネットワークに順次接続試行
    if (connectToKnownNetworks()) {
      return true;
    }
    
    // 接続失敗時、自動フォールバック設定があればAPモードに切り替え
    if (config->ap_mode.auto_fallback) {
      Serial.println("全てのWiFi接続に失敗。APモードに切り替えます。");
      return startAPMode();
    }
    
    return false;
  }

  void scanNetworks() {
    Serial.println("WiFiネットワークをスキャン中...");
    int n = WiFi.scanNetworks();
    available_networks.clear();
    
    if (n == 0) {
      Serial.println("ネットワークが見つかりませんでした");
    } else {
      Serial.printf("%d個のネットワークが見つかりました:\n", n);
      for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        available_networks.push_back(ssid);
        Serial.printf("%d: %s (%ddBm) %s\n", 
                     i + 1, 
                     ssid.c_str(), 
                     WiFi.RSSI(i),
                     (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "開放" : "暗号化");
      }
    }
    last_scan_time = millis();
  }

  bool connectToKnownNetworks() {
    auto valid_networks = config->getValidNetworks();
    
    for (const auto& network : valid_networks) {
      // 利用可能ネットワークに含まれているかチェック
      bool found = false;
      for (const auto& available : available_networks) {
        if (available == network.ssid) {
          found = true;
          break;
        }
      }
      
      if (!found) {
        Serial.printf("ネットワーク '%s' が見つかりません。スキップします。\n", network.ssid.c_str());
        continue;
      }
      
      if (connectToNetwork(network)) {
        return true;
      }
    }
    
    return false;
  }

  bool connectToNetwork(const WiFiNetwork& network) {
    Serial.printf("'%s' に接続を試行中...\n", network.ssid.c_str());
    
    WiFi.begin(network.ssid.c_str(), network.password.c_str());
    
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && 
           millis() - start_time < config->connection_timeout) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      is_connected = true;
      is_ap_mode = false;
      current_ssid = network.ssid;
      Serial.printf("WiFi接続成功: %s\n", network.ssid.c_str());
      Serial.printf("IPアドレス: %s\n", WiFi.localIP().toString().c_str());
      return true;
    } else {
      Serial.printf("'%s' への接続に失敗しました\n", network.ssid.c_str());
      return false;
    }
  }

  bool startAPMode() {
    Serial.println("アクセスポイントモードを開始します...");
    
    WiFi.mode(WIFI_AP);
    delay(100);
    
    bool result = WiFi.softAP(
      config->ap_mode.ssid.c_str(),
      config->ap_mode.password.c_str(),
      config->ap_mode.channel,
      0, // hidden (0=visible, 1=hidden)
      config->ap_mode.max_connections
    );
    
    if (result) {
      is_ap_mode = true;
      is_connected = true;
      current_ssid = config->ap_mode.ssid;
      Serial.printf("APモード開始成功\n");
      Serial.printf("SSID: %s\n", config->ap_mode.ssid.c_str());
      Serial.printf("パスワード: %s\n", config->ap_mode.password.c_str());
      Serial.printf("IPアドレス: %s\n", WiFi.softAPIP().toString().c_str());
      return true;
    } else {
      Serial.println("APモードの開始に失敗しました");
      return false;
    }
  }

  bool switchToAPMode() {
    if (is_ap_mode) {
      Serial.println("既にAPモードです");
      return true;
    }
    
    WiFi.disconnect();
    delay(500);
    return startAPMode();
  }

  bool switchToSTAMode() {
    if (!is_ap_mode) {
      Serial.println("既にSTAモードです");
      return is_connected;
    }
    
    WiFi.softAPdisconnect(true);
    delay(500);
    return begin();
  }

  void reconnect() {
    if (is_ap_mode) {
      return; // APモードでは再接続不要
    }
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi再接続を試行中...");
      begin();
    }
  }

  String getStatus() {
    if (is_ap_mode) {
      return "APモード: " + current_ssid + " (" + WiFi.softAPIP().toString() + ")";
    } else if (is_connected) {
      return "接続済み: " + current_ssid + " (" + WiFi.localIP().toString() + ")";
    } else {
      return "未接続";
    }
  }

  bool isConnected() const { return is_connected; }
  bool isAPMode() const { return is_ap_mode; }
  String getCurrentSSID() const { return current_ssid; }
  String getIPAddress() const {
    if (is_ap_mode) {
      return WiFi.softAPIP().toString();
    } else {
      return WiFi.localIP().toString();
    }
  }

  int getConnectedClients() const {
    if (is_ap_mode) {
      return WiFi.softAPgetStationNum();
    }
    return 0;
  }

  std::vector<String> getAvailableNetworks() const {
    return available_networks;
  }

  void refreshNetworkScan() {
    if (millis() - last_scan_time > 30000) { // 30秒に1回スキャン
      scanNetworks();
    }
  }
};

#endif
