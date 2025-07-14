# Stack-chan Communication Edition

日本語 | [English](README_communication_en.md)

スタックチャンの通信機能に特化したファームウェアです。WiFi WebサーバーAPIとBluetooth通信によって外部からメッセージを送信し、画面に表示することができます。

## 特徴

- **WiFi WebサーバーAPI**: HTTP APIでメッセージ送信・表情変更
- **Bluetooth通信**: シリアル通信でのメッセージ受信
- **リアルタイム画面表示**: Avatar表示 + 受信メッセージ表示
- **拡張性**: 最小限のコードで様々な機能追加が可能

## 対応デバイス

- M5Stack Core2
- M5Stack CoreS3  
- M5Stack Core1 (一部制限あり)

## セットアップ

### 1. WiFi設定

`data/yaml/SC_BasicConfig.yaml`ファイルでWiFi設定を行います：

```yaml
wifi:
  ssid: "YourWiFiSSID"          # お使いのWiFiネットワーク名
  password: "YourWiFiPassword"  # WiFiパスワード
  enable_ap_mode: false         # APモード有効化
```

### 2. ビルドと書き込み

PlatformIOを使用してビルドします：

```bash
# ビルド
pio run -e m5stack-core2

# 書き込み
pio run -e m5stack-core2 -t upload

# ファイルシステム書き込み（設定ファイル用）
pio run -e m5stack-core2 -t uploadfs
```

## 使い方

### WiFi WebサーバーAPI

起動後、シリアルモニターまたは画面に表示されるIPアドレスにブラウザでアクセス。

#### Webインターface

`http://[IPアドレス]/` にアクセスすると制御パネルが表示されます。

#### REST API

##### メッセージ送信
```http
POST /api/message
Content-Type: application/json

{
  "message": "こんにちはスタックチャン！"
}
```

##### 表情変更
```http
POST /api/expression
Content-Type: application/json

{
  "expression": "happy"  // normal, happy, sleepy, doubt
}
```

##### ステータス取得
```http
GET /api/status
```

レスポンス例：
```json
{
  "current_message": "こんにちは",
  "last_received": "WiFi: テストメッセージ", 
  "bluetooth_connected": true,
  "wifi_connected": true,
  "ip_address": "192.168.1.100",
  "mode": 2
}
```

### Bluetooth通信

デバイス名: `StackChan-Comm`

#### プレーンテキスト送信
```
こんにちはスタックチャン！
```

#### JSON形式送信
```json
{
  "message": "メッセージ",
  "expression": "happy"
}
```

#### Pythonサンプル

```python
import bluetooth
import json

# Bluetooth接続
sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
sock.connect(("XX:XX:XX:XX:XX:XX", 1))  # MACアドレスを設定

# メッセージ送信
message = {
    "message": "Pythonからこんにちは！",
    "expression": "happy"
}
sock.send(json.dumps(message).encode('utf-8'))

# 応答受信
response = sock.recv(1024)
print(f"応答: {response.decode('utf-8')}")

sock.close()
```

#### Node.js サンプル

```javascript
const SerialPort = require('serialport');

// Bluetooth Serial接続（OS固有の設定が必要）
const port = new SerialPort('/dev/rfcomm0', { baudRate: 9600 });

// メッセージ送信
const message = {
    message: "Node.jsからこんにちは！",
    expression: "sleepy"
};

port.write(JSON.stringify(message));

// 応答受信
port.on('data', (data) => {
    console.log('応答:', data.toString());
});
```

### ボタン操作

- **ボタンA**: 通信モード切替（WiFi/Bluetooth/両方）
- **ボタンB**: システムステータス表示
- **ボタンC**: ランダム表情変更

## カスタマイズ

### 新しいAPIエンドポイント追加

```cpp
// main_communication.cpp の setupWebServer() に追加
server.on("/api/custom", HTTP_POST, []() {
  // カスタム処理
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, server.arg("plain"));
  
  String customParam = doc["custom"];
  // 処理を追加
  
  server.send(200, "application/json", "{\"status\":\"success\"}");
});
```

### Bluetooth コマンド拡張

```cpp
// handleBluetoothData() 関数内に追加
if (doc.containsKey("custom_command")) {
  String command = doc["custom_command"];
  // カスタムコマンド処理
}
```

### 新しい表情追加

Avatar ライブラリの表情一覧：
- `Expression::Neutral`
- `Expression::Happy` 
- `Expression::Sleepy`
- `Expression::Doubt`
- `Expression::Sad`
- `Expression::Angry`

## API仕様詳細

### エラーレスポンス

```json
{
  "error": "エラーメッセージ",
  "code": 400
}
```

### 通信モード

- `0`: WiFiモード（Bluetoothオフ）
- `1`: Bluetoothモード（WiFiオフ）
- `2`: 両方モード（デフォルト）

## トラブルシューティング

### WiFi接続できない
1. SSIDとパスワードを確認
2. 2.4GHz帯のWiFiを使用
3. シリアルモニターでエラー確認

### Bluetooth接続できない
1. デバイスが検出可能状態か確認
2. 他のデバイスとのペアリングを解除
3. デバイス名 `StackChan-Comm` で検索

### メッセージが表示されない
1. JSON形式が正しいか確認
2. API応答ステータスを確認
3. シリアルモニターでログ確認

## ライセンス

MIT License

## 作者

Based on TakaoAkaki's stack-chan-tester
