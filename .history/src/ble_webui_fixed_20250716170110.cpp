/*
 * BLE WebUI Handler for Stack-chan
 * BLE経由でWebUIアクセスを可能にする
 */

#include "ble_webui.h"

// グローバルなBLEWebUIHandlerインスタンス
BLEWebUIHandler* bleWebUI = nullptr;

BLEWebUIHandler::BLEWebUIHandler() {
    pServer = nullptr;
    pCharacteristic = nullptr;
    deviceConnected = false;
    pendingResponse = "";
}

void BLEWebUIHandler::begin() {
    Serial.println("BLE WebUI初期化開始");
    
    // BLEデバイス初期化
    BLEDevice::init(BLE_DEVICE_NAME);
    
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
    
    // アドバタイズ開始
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE WebUI準備完了");
    Serial.println("デバイス名: " + String(BLE_DEVICE_NAME));
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
    String html = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Stack-chan BLE WebUI</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .container { 
            max-width: 500px; 
            margin: 0 auto; 
            background: rgba(255,255,255,0.1);
            padding: 20px;
            border-radius: 15px;
            backdrop-filter: blur(10px);
        }
        h1 { text-align: center; margin-bottom: 30px; }
        .section { 
            margin: 20px 0; 
            padding: 15px;
            background: rgba(255,255,255,0.1);
            border-radius: 10px;
        }
        button { 
            padding: 10px 15px; 
            margin: 5px; 
            border: none; 
            border-radius: 5px; 
            background: linear-gradient(45deg, #f093fb 0%, #f5576c 100%);
            color: white; 
            cursor: pointer;
            font-weight: bold;
        }
        button:hover { 
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.3);
        }
        input, select { 
            padding: 8px; 
            margin: 5px; 
            border: none; 
            border-radius: 5px; 
            background: rgba(255,255,255,0.9);
            color: #333;
        }
        .status {
            background: rgba(255,255,255,0.2);
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Stack-chan BLE Control</h1>
        
        <div class="section">
            <h3>Expression & Speech</h3>
            <select id="expression">
                <option value="0">Normal</option>
                <option value="1">Happy</option>
                <option value="2">Sleepy</option>
                <option value="3">Doubt</option>
            </select>
            <input type="text" id="speech" placeholder="Enter message..." maxlength="50">
            <button onclick="setSpeech()">Send</button>
            <button onclick="clearSpeech()">Clear</button>
        </div>
        
        <div class="section">
            <h3>Quick Actions</h3>
            <button onclick="cycleExpression()">Change Expression</button>
            <button onclick="cycleColor()">Change Color</button>
            <br>
            <button onclick="setColor(1)">Blue</button>
            <button onclick="setColor(2)">Green</button>
            <button onclick="setColor(3)">Red</button>
            <button onclick="setColor(4)">Purple</button>
            <button onclick="setColor(5)">Orange</button>
            <button onclick="setColor(0)">Default</button>
        </div>
        
        <div class="section">
            <h3>System Status</h3>
            <div class="status" id="status">Loading status...</div>
            <button onclick="updateStatus()">Update Status</button>
        </div>
    </div>

    <script>
        function bleRequest(path) {
            console.log('BLE Request: GET ' + path);
            // In real BLE app, this would write to BLE characteristic
            alert('BLE request sent: ' + path);
        }
        
        function setSpeech() {
            const expression = document.getElementById('expression').value;
            const speech = encodeURIComponent(document.getElementById('speech').value);
            bleRequest('/api/set?expression=' + expression + '&speech=' + speech);
        }
        
        function clearSpeech() {
            bleRequest('/api/set?speech=');
            document.getElementById('speech').value = '';
        }
        
        function cycleExpression() {
            bleRequest('/api/expression');
        }
        
        function cycleColor() {
            bleRequest('/api/color');
        }
        
        function setColor(index) {
            bleRequest('/api/setcolor?index=' + index);
        }
        
        function updateStatus() {
            bleRequest('/api/status');
        }
        
        setTimeout(updateStatus, 1000);
    </script>
</body>
</html>)";
    return html;
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
