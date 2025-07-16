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

// BLE設定
#define BLE_SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define BLE_CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"
#define BLE_DEVICE_NAME         "StackChan-WebUI"

class BLEWebUIHandler {
private:
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
    void onConnect(BLEServer* pServer) {
        // グローバルハンドラーへの参照が必要
    }
    
    void onDisconnect(BLEServer* pServer) {
        // グローバルハンドラーへの参照が必要
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
