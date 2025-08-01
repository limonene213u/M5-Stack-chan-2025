/*
 * BLE WebUI Handler for Stack-chan
 * BLE経由でWebUIアクセスを可能にする
 */

#ifndef BLE_WEBUI_H
#define BLE_WEBUI_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "esp_gap_ble_api.h"

// main.cppの関数宣言
extern String generateWebUIHTML();

// BLE設定（最もシンプルで確実な設定）
#define BLE_SERVICE_UUID        "12345678-1234-1234-1234-123456789ABC"  // シンプルなカスタムUUID
#define BLE_CHARACTERISTIC_UUID "87654321-4321-4321-4321-CBA987654321"  // シンプルなカスタムUUID
#define BLE_DEVICE_NAME         "StackChan"

class BLEWebUIHandler {
public:
    BLEServer* pServer;
    BLECharacteristic* pCharacteristic;
    bool deviceConnected;
    String pendingResponse;
    
    // HTTP風レスポンス生成
    String generateHttpResponse(int statusCode, const String& contentType, const String& body);
    String generateWebUIHTML();
    String generateAPIResponse(const String& endpoint, const String& params);
    
public:
    BLEWebUIHandler();
    void begin();
    void restart();  // BLE再起動機能
    void handleBLERequest();
    bool isConnected() { return deviceConnected; }
    void setDeviceConnected(bool connected) { deviceConnected = connected; }
    
    // 外部関数へのコールバック（main.cppで実装）
    static void onExpressionChange(int expression);
    static void onColorChange(int colorIndex);
    static void onSpeechSet(const String& speech, int expression = -1);
    static String getSystemStatus();
};

// BLEサーバーコールバック
class MyServerCallbacks: public BLEServerCallbacks {
private:
    BLEWebUIHandler* handler;
    
public:
    MyServerCallbacks(BLEWebUIHandler* h = nullptr) : handler(h) {}
    
    void setHandler(BLEWebUIHandler* h) { handler = h; }
    
    void onConnect(BLEServer* pServer) {
        Serial.println("BLEクライアント接続");
        if (handler) {
            handler->setDeviceConnected(true);
        }
    }
    
    void onDisconnect(BLEServer* pServer) {
        Serial.println("BLEクライアント切断 - アドバタイズ再開");
        if (handler) {
            handler->setDeviceConnected(false);
        }
        BLEDevice::startAdvertising();
    }
};

// BLE特性コールバック
class MyCallbacks: public BLECharacteristicCallbacks {
private:
    BLEWebUIHandler* handler;
    
public:
    MyCallbacks(BLEWebUIHandler* h) : handler(h) {}
    
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        
        if (value.length() > 0) {
            String request = String(value.c_str());
            Serial.println("BLE Request: " + request);
            
            // HTTPリクエストパース
            if (request.startsWith("GET ")) {
                processHTTPRequest(request);
            }
        }
    }
    
private:
    void processHTTPRequest(const String& request);
};

extern BLEWebUIHandler* bleWebUI;

#endif
