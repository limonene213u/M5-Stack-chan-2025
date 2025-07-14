# スタックチャン 通信エディション

日本語 | [English](README_communication_en.md)

スタックチャンの通信機能に特化した日本語対応ファームウェアです。WiFi複数AP対応、APモード切り替え、Bluetooth通信によって外部からメッセージを送信し、画面に表示できます。

## ✨ 特徴

- **🌐 WiFi複数AP対応**: 最大10個のWiFiネットワークを登録・自動接続
- **📡 APモード切り替え**: スタックチャン自体がWiFiアクセスポイントになる
- **🔄 自動フォールバック**: 全WiFi接続失敗時に自動でAPモードに切り替え
- **📱 WebUI WiFi管理**: ブラウザからWiFi設定の追加・削除・切り替え
- **📞 Bluetooth通信**: シリアル通信でのメッセージ受信
- **🗾 日本語完全対応**: カスタム日本語フォント内蔵・文字化け解決済み
- **🎨 カスタムフォント**: 組み込み型ビットマップフォントで日本語表示
- **🔊 音声機能準備**: 将来のWAV/MP3再生機能に対応した設計
- **🎨 リアルタイム画面表示**: Avatar表情変更・受信メッセージ表示
- **🚀 拡張性重視**: 最小限のコードで様々な機能追加が可能

## 🎨 カスタム日本語フォント

文字化けを完全に解決するため、カスタムビットマップフォントを内蔵しています：

- **完全埋め込み型**: 外部フォントファイル不要
- **基本文字対応**: ひらがな・カタカナ・基本漢字
- **軽量設計**: 8x16ピクセル、最小メモリ使用量
- **UTF-8対応**: 日本語テキストの正確な解析・表示
- **フォールバック機能**: 未対応文字は標準フォントで表示

### 対応文字例
- ひらがな: あいうえお
- カタカナ: アイウエオ  
- 漢字: 日本語
- 記号: ！？

## 🎯 対応デバイス

- **M5Stack Basic (Grey)** ✅ **推奨**
- M5Stack Core2 ✅ 
- M5Stack CoreS3 ✅
- M5Stack Fire ✅
- M5Stack Core1 (一部制限あり)

## ⚙️ セットアップ

### 1. WiFi設定

`data/yaml/SC_BasicConfig.yaml`ファイルでWiFi設定を行います：

```yaml
wifi:
  # WiFiクライアント設定（最大10個まで登録可能）
  networks:
    - ssid: "Home-WiFi"         # 自宅WiFi
      password: "password123"
      priority: 1               # 優先度（1が最高）
    - ssid: "Office-WiFi"       # オフィスWiFi
      password: "office456" 
      priority: 2
    - ssid: "Mobile-Hotspot"    # モバイルホットスポット
      password: "mobile789"
      priority: 3
  
  # アクセスポイントモード設定
  ap_mode:
    enable: false               # 起動時のAPモード有効化
    ssid: "Stack-chan"          # APのSSID
    password: "Stack-chan-88"   # APのパスワード
    auto_fallback: true         # クライアント接続失敗時の自動APモード切替
```

### 2. ビルドと書き込み

```bash
# M5Stack Basic (Grey) - 推奨
pio run -e m5stack-grey

# M5Stack Core2
pio run -e m5stack-core2

# 書き込み (デバイスに応じて環境名を変更)
pio run -e m5stack-grey -t upload

# ファイルシステム書き込み（設定ファイル用）
pio run -e m5stack-grey -t uploadfs
```

## 📱 使い方

### WiFi接続

1. **自動接続**: 起動時に設定されたWiFiネットワークに優先度順で自動接続
2. **APモード**: 全接続失敗時は自動でAPモード（SSID: Stack-chan, PASS: Stack-chan-88）
3. **手動切り替え**: Webインターフェースまたはボタンでモード切り替え可能

### Webインターフェース

接続成功後、シリアルモニターまたは画面に表示されるIPアドレスにブラウザでアクセス。

#### 📋 制御パネル機能

- **メッセージ送信**: テキスト入力してスタックチャンに表示
- **表情変更**: 普通・嬉しい・眠い・疑問の4種類
- **WiFi管理**: ネットワークスキャン・追加・接続・モード切り替え
- **システム状態**: リアルタイム状態表示

#### 🔌 REST API

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

##### WiFiネットワークスキャン

```http
GET /api/wifi/scan
```

##### WiFiモード切り替え

```http
POST /api/wifi/toggle
```

##### WiFiネットワーク追加

```http
POST /api/wifi/add
Content-Type: application/json

{
  "ssid": "新しいWiFi",
  "password": "パスワード",
  "priority": 1
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
  "wifi_status": "接続済み: Home-WiFi (192.168.1.100)",
  "ip_address": "192.168.1.100",
  "connected_clients": 2,
  "is_ap_mode": false,
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
