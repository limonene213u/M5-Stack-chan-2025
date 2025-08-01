/*
 * BLE WebUI Handler for Stack-chan
 * BLE経由でWebUIアクセスを可能にする
 */

#include "ble_webui.h"

// main.cppのグローバル変数（外部参照）
extern bool is_speaking;
extern bool connection_mode_ble;

BLEWebUIHandler::BLEWebUIHandler() {
    pServer = nullptr;
    pCharacteristic = nullptr;
    deviceConnected = false;
    pendingResponse = "";
}

void BLEWebUIHandler::begin() {
    Serial.println("BLE WebUI初期化開始");
    
    // BLEモードフラグを設定
    connection_mode_ble = true;
    
    // BLEデバイス初期化（確実な発見のための設定）
    BLEDevice::init(BLE_DEVICE_NAME);
    
    // デバイス名を明示的に設定（発見しやすくするため）
    esp_ble_gap_set_device_name(BLE_DEVICE_NAME);
    
    // 送信パワーを最大に設定
    BLEDevice::setPower(ESP_PWR_LVL_P9);
    
    Serial.println("BLE電波強度: 最大");
    Serial.println("BLEデバイス名: " + String(BLE_DEVICE_NAME));
    
    // BLEサーバー作成
    pServer = BLEDevice::createServer();
    MyServerCallbacks* serverCallbacks = new MyServerCallbacks(this);
    pServer->setCallbacks(serverCallbacks);
    
    // BLEサービス作成
    BLEService* pService = pServer->createService(BLE_SERVICE_UUID);
    
    // BLE特性作成
    pCharacteristic = pService->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    // コールバック設定
    MyCallbacks* charCallbacks = new MyCallbacks(this);
    pCharacteristic->setCallbacks(charCallbacks);
    pCharacteristic->addDescriptor(new BLE2902());
    
    // サービス開始
    pService->start();
    
    // アドバタイズ開始（シンプルで確実な設定）
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    
    BLEDevice::startAdvertising();
    
    Serial.println("BLE WebUI準備完了");
    Serial.println("デバイス名: " + String(BLE_DEVICE_NAME));
    Serial.println("サービスUUID: " + String(BLE_SERVICE_UUID));
    Serial.println("特性UUID: " + String(BLE_CHARACTERISTIC_UUID));
    Serial.println("BLEアドバタイズ開始 - 標準設定");
    Serial.println("発見のヒント:");
    Serial.println("1. スマートフォンのBluetooth設定を開く");
    Serial.println("2. '新しいデバイス' または 'デバイス検索' を選択");
    Serial.println("3. '" + String(BLE_DEVICE_NAME) + "' を探してタップ");
    Serial.println("4. 見つからない場合は、Aボタン長押しでBLE再起動");
}

void BLEWebUIHandler::restart() {
    Serial.println("BLE再起動中...");
    
    // 既存のBLE接続を停止
    if (pServer) {
        BLEDevice::stopAdvertising();
        BLEDevice::deinit();
        delay(1000); // 1秒待機
    }
    
    // BLEを再初期化
    begin();
}

void BLEWebUIHandler::handleBLERequest() {
    // 定期的な処理（必要に応じて追加）
    delay(10);
}

// 外部関数の実装（weakリンクされたデフォルト実装）
void __attribute__((weak)) BLEWebUIHandler::onExpressionChange(int expression) {
    Serial.println("Expression change callback not implemented: " + String(expression));
}

void __attribute__((weak)) BLEWebUIHandler::onColorChange(int colorIndex) {
    Serial.println("Color change callback not implemented: " + String(colorIndex));
}

void __attribute__((weak)) BLEWebUIHandler::onSpeechSet(const String& speech, int expression) {
    Serial.println("Speech set callback not implemented: " + speech);
}

String __attribute__((weak)) BLEWebUIHandler::getSystemStatus() {
    return "System OK";
}

String BLEWebUIHandler::generateHttpResponse(int statusCode, const String& contentType, const String& body) {
    String response = "HTTP/1.1 " + String(statusCode) + " OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + String(body.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type\r\n";
    response += "\r\n";
    response += body;
    return response;
}

String BLEWebUIHandler::generateWebUIHTML() {
    // main.cppの共有HTML生成関数を使用
    return ::generateWebUIHTML();
}

String BLEWebUIHandler::generateAPIResponse(const String& endpoint, const String& params) {
    String response = "OK";
    
    if (endpoint == "/api/expression") {
        onExpressionChange(-1); // サイクル変更
        response = "Expression cycled";
    } else if (endpoint == "/api/color") {
        onColorChange(-1); // サイクル変更
        response = "Color cycled";
    } else if (endpoint == "/api/set") {
        // パラメータ解析
        int expressionParam = -1;
        String speechParam = "";
        
        // シンプルなパラメータ解析
        int expPos = params.indexOf("expression=");
        if (expPos >= 0) {
            int expStart = expPos + 11;
            int expEnd = params.indexOf("&", expStart);
            if (expEnd == -1) expEnd = params.length();
            expressionParam = params.substring(expStart, expEnd).toInt();
        }
        
        int speechPos = params.indexOf("speech=");
        if (speechPos >= 0) {
            int speechStart = speechPos + 7;
            int speechEnd = params.indexOf("&", speechStart);
            if (speechEnd == -1) speechEnd = params.length();
            speechParam = params.substring(speechStart, speechEnd);
            // URL decode (basic)
            speechParam.replace("%20", " ");
            speechParam.replace("%21", "!");
        }
        
        if (expressionParam >= 0) {
            onExpressionChange(expressionParam);
        }
        if (speechParam.length() > 0) {
            onSpeechSet(speechParam, expressionParam);
        }
        
        response = "Set expression=" + String(expressionParam) + ", speech=" + speechParam;
    } else if (endpoint == "/api/setcolor") {
        int colorIndex = 0;
        int colorPos = params.indexOf("index=");
        if (colorPos >= 0) {
            int colorStart = colorPos + 6;
            int colorEnd = params.indexOf("&", colorStart);
            if (colorEnd == -1) colorEnd = params.length();
            colorIndex = params.substring(colorStart, colorEnd).toInt();
        }
        onColorChange(colorIndex);
        response = "Color set to " + String(colorIndex);
    } else if (endpoint == "/api/status") {
        response = getSystemStatus();
    } else {
        response = "Unknown endpoint";
    }
    
    return response;
}

// MyCallbacksクラスの実装
void MyCallbacks::processHTTPRequest(const String& request) {
    Serial.println("Processing HTTP request: " + request);
    
    String response = "";
    
    // HTTPリクエストを解析
    if (request.startsWith("GET /")) {
        int pathEnd = request.indexOf(" HTTP");
        if (pathEnd == -1) pathEnd = request.indexOf("\r");
        if (pathEnd == -1) pathEnd = request.indexOf("\n");
        if (pathEnd == -1) pathEnd = request.length();
        
        String path = request.substring(4, pathEnd); // "GET " を除く
        
        if (path == "/" || path == "/index.html" || path == "") {
            // WebUI HTML を返す
            String html = handler->generateWebUIHTML();
            response = handler->generateHttpResponse(200, "text/html", html);
        } else if (path.startsWith("/api/")) {
            // API エンドポイント処理
            String endpoint = path;
            String params = "";
            int paramPos = path.indexOf("?");
            if (paramPos >= 0) {
                endpoint = path.substring(0, paramPos);
                params = path.substring(paramPos + 1);
            }
            String apiResponse = handler->generateAPIResponse(endpoint, params);
            response = handler->generateHttpResponse(200, "text/plain", apiResponse);
        } else {
            // 404 Not Found
            response = handler->generateHttpResponse(404, "text/plain", "Not Found");
        }
        
        // レスポンスをBLE特性に送信
        handler->pCharacteristic->setValue(response.c_str());
        handler->pCharacteristic->notify();
        
        Serial.println("Response sent via BLE");
    }
}
