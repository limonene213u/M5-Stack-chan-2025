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


# Appendix
## 日本語サブセットに含まれる文字

=== 完全なかなセット ===

ひらがな完全セット (77文字):
あいうえおかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよらりるれろわゐゑをんゃゅょっ

カタカナ完全セット (78文字):
アイウエオカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨラリルレロワヰヱヲンャュョッー

=== 使用される漢字 Unicode情報 ===
'丈' : U+4E08 (19976)
'両' : U+4E21 (20001)
'中' : U+4E2D (20013)
'了' : U+4E86 (20102)
'今' : U+4ECA (20170)
'伝' : U+4F1D (20253)
'何' : U+4F55 (20309)
'保' : U+4FDD (20445)
'信' : U+4FE1 (20449)
'停' : U+505C (20572)
'備' : U+5099 (20633)
'充' : U+5145 (20805)
'再' : U+518D (20877)
'分' : U+5206 (20998)
'切' : U+5207 (20999)
'初' : U+521D (21021)
'削' : U+524A (21066)
'前' : U+524D (21069)
'功' : U+529F (21151)
'加' : U+52A0 (21152)
'動' : U+52D5 (21205)
'化' : U+5316 (21270)
'午' : U+5348 (21320)
'受' : U+53D7 (21463)
'告' : U+544A (21578)
'問' : U+554F (21839)
'圧' : U+5727 (22311)
'変' : U+5909 (22793)
'大' : U+5927 (22823)
'天' : U+5929 (22825)
'夫' : U+592B (22827)
'失' : U+5931 (22833)
'始' : U+59CB (22987)
'存' : U+5B58 (23384)
'完' : U+5B8C (23436)
'定' : U+5B9A (23450)
'容' : U+5BB9 (23481)
'常' : U+5E38 (24120)
'年' : U+5E74 (24180)
'度' : U+5EA6 (24230)
'張' : U+5F35 (24373)
'待' : U+5F85 (24453)
'後' : U+5F8C (24460)
'情' : U+60C5 (24773)
'意' : U+610F (24847)
'成' : U+6210 (25104)
'手' : U+624B (25163)
'接' : U+63A5 (25509)
'敗' : U+6557 (25943)
'断' : U+65AD (26029)
'新' : U+65B0 (26032)
'方' : U+65B9 (26041)
'日' : U+65E5 (26085)
'時' : U+6642 (26178)
'曜' : U+66DC (26332)
'更' : U+66F4 (26356)
'月' : U+6708 (26376)
'期' : U+671F (26399)
'未' : U+672A (26410)
'本' : U+672C (26412)
'様' : U+69D8 (27096)
'機' : U+6A5F (27231)
'止' : U+6B62 (27490)
'正' : U+6B63 (27491)
'気' : U+6C17 (27671)
'注' : U+6CE8 (27880)
'済' : U+6E08 (28168)
'温' : U+6E29 (28201)
'湿' : U+6E7F (28287)
'源' : U+6E90 (28304)
'準' : U+6E96 (28310)
'疲' : U+75B2 (30130)
'確' : U+78BA (30906)
'秒' : U+79D2 (31186)
'終' : U+7D42 (32066)
'続' : U+7D9A (32154)
'表' : U+8868 (34920)
'設' : U+8A2D (35373)
'証' : U+8A3C (35388)
'認' : U+8A8D (35469)
'語' : U+8A9E (35486)
'読' : U+8AAD (35501)
'警' : U+8B66 (35686)
'起' : U+8D77 (36215)
'込' : U+8FBC (36796)
'追' : U+8FFD (36861)
'送' : U+9001 (36865)
'量' : U+91CF (37327)
'開' : U+958B (38283)
'間' : U+9593 (38291)
'除' : U+9664 (38500)
'電' : U+96FB (38651)
'頑' : U+9811 (38929)
'題' : U+984C (38988)

=== フォント実装優先度 ===
1. 完全ひらがなセット (74文字)
2. 完全カタカナセット (75文字)
3. 実際に使用される漢字 (94文字)
4. 基本的な記号・数字

合計推定文字数: 263文字
推定メモリ使用量: 4734バイト (4.6KB)
(3.11.6) 