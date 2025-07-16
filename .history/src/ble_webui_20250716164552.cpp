/*
 * BLE WebUI Implementation for Stack-chan
 * BLE経由でWebUIアクセスを可能にする実装
 */

#include "ble_webui.h"

// グローバルインスタンス
BLEWebUIHandler* bleWebUI = nullptr;

// 外部関数の宣言（main.cppで実装される）
extern void changeExpressionById(int id);
extern void changeColorById(int id);
extern void setSpeechText(const String& text, int expression = -1);
extern String getSystemStatusJSON();

BLEWebUIHandler::BLEWebUIHandler() {
    pServer = nullptr;
    pCharacteristic = nullptr;
    deviceConnected = false;
    pendingResponse = "";
}

void BLEWebUIHandler::begin() {
    Serial.println("BLE WebUI初期化開始...");
    
    // BLEデバイス初期化
    BLEDevice::init(BLE_DEVICE_NAME);
    
    // BLEサーバー作成
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // BLEサービス作成
    BLEService *pService = pServer->createService(BLE_SERVICE_UUID);
    
    // BLE特性作成
    pCharacteristic = pService->createCharacteristic(
                        BLE_CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    
    pCharacteristic->setCallbacks(new MyCallbacks(this));
    pCharacteristic->addDescriptor(new BLE2902());
    
    // サービス開始
    pService->start();
    
    // アドバタイズ開始
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE WebUI開始完了 - デバイス名: " + String(BLE_DEVICE_NAME));
}

void BLEWebUIHandler::handleBLERequest() {
    // ここでは特に処理なし（コールバックで処理）
    if (!pendingResponse.isEmpty() && deviceConnected) {
        // レスポンス送信
        pCharacteristic->setValue(pendingResponse.c_str());
        pCharacteristic->notify();
        pendingResponse = "";
    }
}

String BLEWebUIHandler::generateHttpResponse(int statusCode, const String& contentType, const String& body) {
    String response = "HTTP/1.1 " + String(statusCode);
    switch(statusCode) {
        case 200: response += " OK"; break;
        case 404: response += " Not Found"; break;
        default: response += " Unknown"; break;
    }
    response += "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + String(body.length()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    return response;
}

String BLEWebUIHandler::generateWebUIHTML() {
    String html = R"(
<!DOCTYPE html>
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
            background: #4CAF50; 
            color: white; 
            border: none; 
            padding: 10px 20px; 
            margin: 5px; 
            border-radius: 5px; 
            cursor: pointer;
            transition: all 0.3s;
        }
        button:hover { background: #45a049; transform: translateY(-2px); }
        input[type="text"] { 
            width: 100%; 
            padding: 10px; 
            margin: 5px 0; 
            border: none;
            border-radius: 5px;
            background: rgba(255,255,255,0.9);
            color: #333;
        }
        select { 
            width: 100%; 
            padding: 10px; 
            margin: 5px 0; 
            border: none;
            border-radius: 5px;
            background: rgba(255,255,255,0.9);
            color: #333;
        }
        .status { 
            background: rgba(0,0,0,0.3); 
            padding: 10px; 
            border-radius: 5px; 
            margin: 10px 0;
            font-family: monospace;
        }
        .note {
            background: rgba(255,165,0,0.3);
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🤖 Stack-chan BLE WebUI</h1>
        
        <div class="note">
            <strong>📱 BLE接続モード</strong><br>
            WiFi接続なしでBluetooth経由でアクセスしています。
        </div>
        
        <div class="section">
            <h3>💬 メッセージ & 表情</h3>
            <select id="expression">
                <option value="0">😐 普通</option>
                <option value="1">😊 嬉しい</option>
                <option value="2">😴 眠い</option>
                <option value="3">🤔 困った</option>
            </select>
            <input type="text" id="speech" placeholder="メッセージを入力..." maxlength="50">
            <button onclick="setSpeech()">送信</button>
            <button onclick="clearSpeech()">クリア</button>
        </div>
        
        <div class="section">
            <h3>🎨 クイック操作</h3>
            <button onclick="cycleExpression()">表情変更</button>
            <button onclick="cycleColor()">色変更</button>
            <br>
            <button onclick="setColor(1)">🔵 青</button>
            <button onclick="setColor(2)">🟢 緑</button>
            <button onclick="setColor(3)">🔴 赤</button>
            <button onclick="setColor(4)">🟣 紫</button>
            <button onclick="setColor(5)">🟠 橙</button>
            <button onclick="setColor(0)">⚪ 標準</button>
        </div>
        
        <div class="section">
            <h3>📊 システム状態</h3>
            <div class="status" id="status">接続状態を取得中...</div>
            <button onclick="updateStatus()">状態更新</button>
        </div>
    </div>

    <script>
        // BLE WebUIは直接的なJavaScript実行が制限されるため、
        // 代わりにBLE特性への書き込みでAPI呼び出しを行う
        
        function bleRequest(path) {
            // 実際のBLEアプリでは、この部分でBLE特性に書き込む
            console.log('BLE Request: GET ' + path);
            
            // 模擬的にアラート表示（実際のBLEアプリでは適切な実装が必要）
            alert('BLE経由でリクエスト送信: ' + path);
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
        
        // 初期状態更新
        setTimeout(updateStatus, 1000);
    </script>
</body>
</html>
)";
    return html;
}

String BLEWebUIHandler::generateAPIResponse(const String& endpoint, const String& params) {
    String response = "OK";
    
    if (endpoint == "/api/expression") {
        changeExpressionById(-1); // サイクル変更
        response = "Expression cycled";
        
    } else if (endpoint == "/api/color") {
        changeColorById(-1); // サイクル変更
        response = "Color cycled";
        
    } else if (endpoint == "/api/setcolor") {
        int index = params.indexOf("index=");
        if (index >= 0) {
            int colorIndex = params.substring(index + 6).toInt();
            changeColorById(colorIndex);
            response = "Color set to index " + String(colorIndex);
        }
        
    } else if (endpoint == "/api/set") {
        int expIndex = params.indexOf("expression=");
        int speechIndex = params.indexOf("speech=");
        
        String speech = "";
        int expression = -1;
        
        if (speechIndex >= 0) {
            int ampIndex = params.indexOf("&", speechIndex);
            if (ampIndex >= 0) {
                speech = params.substring(speechIndex + 7, ampIndex);
            } else {
                speech = params.substring(speechIndex + 7);
            }
            // URLデコード簡易版
            speech.replace("%20", " ");
            speech.replace("%21", "!");
            speech.replace("%3F", "?");
        }
        
        if (expIndex >= 0) {
            int ampIndex = params.indexOf("&", expIndex);
            if (ampIndex >= 0) {
                expression = params.substring(expIndex + 11, ampIndex).toInt();
            } else {
                expression = params.substring(expIndex + 11).toInt();
            }
        }
        
        setSpeechText(speech, expression);
        response = "Speech set: \"" + speech + "\"";
        if (expression >= 0) {
            response += ", Expression: " + String(expression);
        }
        
    } else if (endpoint == "/api/status") {
        response = getSystemStatusJSON();
    }
    
    return response;
}

// MyCallbacks実装
void MyCallbacks::processHTTPRequest(const String& request) {
    // HTTPリクエストをパース
    int spaceIndex = request.indexOf(' ');
    int secondSpaceIndex = request.indexOf(' ', spaceIndex + 1);
    
    if (spaceIndex > 0 && secondSpaceIndex > 0) {
        String path = request.substring(spaceIndex + 1, secondSpaceIndex);
        Serial.println("BLE HTTP Path: " + path);
        
        String response;
        
        if (path == "/" || path == "/index.html") {
            // WebUIのHTML返却
            String html = handler->generateWebUIHTML();
            response = handler->generateHttpResponse(200, "text/html", html);
            
        } else if (path.startsWith("/api/")) {
            // API処理
            int questionIndex = path.indexOf('?');
            String endpoint = (questionIndex >= 0) ? path.substring(0, questionIndex) : path;
            String params = (questionIndex >= 0) ? path.substring(questionIndex + 1) : "";
            
            String apiResponse = handler->generateAPIResponse(endpoint, params);
            response = handler->generateHttpResponse(200, "text/plain", apiResponse);
            
        } else {
            // 404
            response = handler->generateHttpResponse(404, "text/plain", "Not Found");
        }
        
        // レスポンスを特性に設定して通知
        handler->pCharacteristic->setValue(response.c_str());
        handler->pCharacteristic->notify();
    }
}
