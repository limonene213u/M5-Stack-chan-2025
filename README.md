# スタックチャン 通信エディション 2025

日本語 | [English](README_en.md)

スタックチャンの通信機能に特化した日本語対応ファームウェアです。WiFi複数AP対応、APモード切り替え、Bluetooth通信によって外部からメッセージを送信し、Avatar表示機能を含む完全なstack-chan実装です。

![Stack-chan Communication](docs/images/stack-chan-demo.png)

## ✨ 主要機能

### 🌐 通信機能
- **WiFi複数AP対応**: 最大10個のWiFiネットワークを登録・自動接続
- **APモード切り替え**: スタックチャン自体がWiFiアクセスポイントになる
- **自動フォールバック**: 全WiFi接続失敗時に自動でAPモードに切り替え
- **WebUI WiFi管理**: ブラウザからWiFi設定の追加・削除・切り替え
- **🔵 BLE WebUI**: WiFi不要でBluetooth経由でWebUIアクセス可能
- **通信モード切り替え**: ボタン操作でWiFi⟷BLEモードを動的切り替え
- **Bluetooth通信**: シリアル通信でのメッセージ受信

### 🎨 表示・Avatar機能
- **🗾 日本語完全対応**: M5GFX内蔵日本語フォント・文字化け解決済み
- **😊 Avatar表情システム**: リアルタイム表情変更（Happy、Sleepy、Doubt等）
- **📱 WebUI操作**: ブラウザからメッセージ送信・表情変更
- **🔊 音声準備**: 将来のTTS/音声認識機能に対応した設計

### ⚡ 技術的特徴
- **StackchanSystemConfig対応**: 公式stack-chan設定ファイル完全対応
- **FreeRTOS最適化**: Avatar機能との安定した共存
- **拡張性重視**: ChatGPT API等の高度な機能への発展基盤

## 📚 技術参考・比較

### 基盤実装
- **元実装**: TakaoAkaki/stack-chan-tester
- **参考事例**: M5Unified_StackChan_ChatGPT
- **発展方向**: 会話AI機能への拡張

### 実装比較表

| 機能 | 本実装 | ChatGPT版 | 元実装 |
|------|-------|---------|--------|
| WiFi管理 | ✅ 複数AP | ✅ SmartConfig | ❌ 単一のみ |
| Avatar表示 | ✅ 基本機能 | ✅ 表情連動 | ✅ 基本 |
| 日本語対応 | ✅ M5GFX内蔵 | ✅ efontJA_16 | ❌ 制限 |
| WebUI | ✅ 設定管理 | ✅ チャット | ❌ なし |
| 音声機能 | ⏳ 準備中 | ✅ TTS+認識 | ❌ なし |
| AI機能 | ⏳ 将来対応 | ✅ ChatGPT | ❌ なし |

## 🎯 対応デバイス

- **M5Stack Basic (Grey)** ✅ **推奨・開発基準**
- M5Stack Core2 ✅ **動作確認済み**
- M5Stack CoreS3 ✅ 
- M5Stack Fire ✅
- M5Stack Core1 (一部制限あり)

## ⚙️ セットアップ・使い方

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

### 🔵 BLE WebUI（WiFi不要モード）

WiFi環境がない場合やWiFi接続に失敗した場合、自動的にBLEモードで起動します。
BLEデバイス名は `StackChan-WebUI` です。

#### BLE接続手順

1. **自動BLEモード起動**: WiFi接続に失敗すると自動的にBLEモードに切り替わります
2. **手動切り替え**: ボタンBでWiFi⟷BLEモードを切り替え可能
3. **デバイス検索**: スマートフォンやPCのBluetooth設定で「StackChan-WebUI」を検索
4. **BLEアプリ使用**: 
   - iOS: LightBlue、BLE Scanner等
   - Android: nRF Connect、BLE Scanner等
   - PC: Web Bluetooth対応ブラウザ（Chrome等）

#### BLE WebUI操作

1. BLEアプリで「StackChan-WebUI」に接続
2. サービスUUID: `12345678-1234-1234-1234-123456789abc`
3. 特性UUID: `87654321-4321-4321-4321-cba987654321`
4. 特性にHTTPリクエスト形式で書き込み:
   ```
   GET /
   GET /api/expression
   GET /api/set?expression=1&speech=Hello
   ```

#### 対応BLEアプリ

**iOS**
- **LightBlue Explorer** (推奨)
- BLE Scanner 4.0
- Bluetooth Scanner

**Android**
- **nRF Connect** (推奨) 
- BLE Scanner
- Bluetooth LE Scanner

**PC/Mac**
- Web Bluetooth対応ブラウザ（Chrome、Edge等）
- Bluetooth LE Explorer (Windows)

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

現在のシンプル版で利用可能なAPIエンドポイント：

##### 表情サイクル変更

```http
GET /api/expression
```

レスポンス: `Expression changed to: 嬉しい`

##### 色テーマサイクル変更

```http
GET /api/color
```

レスポンス: `Color changed to: 青系`

##### 特定色テーマ設定

```http
GET /api/setcolor?index=0
```

パラメータ:

- `index`: 色テーマ番号 (0-5)
  - `0`: 標準色
  - `1`: 青系
  - `2`: 緑系  
  - `3`: 赤系
  - `4`: 紫系
  - `5`: オレンジ系

レスポンス: `Color set to: 赤系`

##### 表情とセリフの同時設定

```http
GET /api/set?expression=1&speech=こんにちは！
```

パラメータ:

- `expression`: 表情番号 (0-3)
  - `0`: 普通 (Neutral)
  - `1`: 嬉しい (Happy)
  - `2`: 眠い (Sleepy)
  - `3`: 困った (Doubt)
- `speech`: 表示するセリフ（日本語対応、URLエンコード推奨）

レスポンス: `表情: 嬉しい, セリフ: "こんにちは！"`

##### セリフクリア

```http
GET /api/set?speech=
```

レスポンス: `セリフ: ""`

### APIの使用例

#### cURLでの操作例

```bash
# 表情を嬉しいにしてメッセージを表示
curl "http://192.168.1.100/api/set?expression=1&speech=Hello%20World"

# 色を赤系に変更
curl "http://192.168.1.100/api/setcolor?index=3"

# セリフをクリア
curl "http://192.168.1.100/api/set?speech="
```

#### JavaScriptでの操作例

```javascript
// 表情とセリフを設定
fetch('/api/set?expression=1&speech=' + encodeURIComponent('こんにちは！'))
  .then(response => response.text())
  .then(data => console.log(data));

// 色をランダムに変更
const colorIndex = Math.floor(Math.random() * 6);
fetch(`/api/setcolor?index=${colorIndex}`)
  .then(response => response.text())
  .then(data => console.log(data));
```

#### Pythonでの操作例

```python
import requests
import urllib.parse

# Stack-chanのIPアドレス
base_url = "http://192.168.1.100"

# 表情とセリフを設定
speech = "Pythonからこんにちは！"
encoded_speech = urllib.parse.quote(speech)
response = requests.get(f"{base_url}/api/set?expression=1&speech={encoded_speech}")
print(response.text)

# 色を青系に変更
response = requests.get(f"{base_url}/api/setcolor?index=1")
print(response.text)
```

### より詳しいPythonプログラム例

以下は実用的なStack-chan制御プログラムのサンプルです：

#### 基本的なStack-chanクライアントクラス

```python
import requests
import urllib.parse
import time
import random
from typing import Optional

class StackChanClient:
    """Stack-chan制御用クライアントクラス"""
    
    EXPRESSIONS = {
        'neutral': 0,    # 普通
        'happy': 1,      # 嬉しい
        'sleepy': 2,     # 眠い
        'doubt': 3       # 困った
    }
    
    COLORS = {
        'default': 0,    # 標準色
        'blue': 1,       # 青系
        'green': 2,      # 緑系
        'red': 3,        # 赤系
        'purple': 4,     # 紫系
        'orange': 5      # オレンジ系
    }
    
    def __init__(self, ip_address: str = "192.168.1.100", timeout: int = 5):
        """
        Stack-chanクライアントを初期化
        
        Args:
            ip_address: Stack-chanのIPアドレス
            timeout: リクエストタイムアウト（秒）
        """
        self.base_url = f"http://{ip_address}"
        self.timeout = timeout
    
    def _make_request(self, endpoint: str) -> Optional[str]:
        """APIリクエストを実行"""
        try:
            response = requests.get(f"{self.base_url}{endpoint}", timeout=self.timeout)
            response.raise_for_status()
            return response.text
        except requests.exceptions.RequestException as e:
            print(f"エラー: {e}")
            return None
    
    def set_expression_and_speech(self, expression: str, speech: str) -> bool:
        """表情とセリフを同時に設定"""
        if expression not in self.EXPRESSIONS:
            print(f"無効な表情: {expression}")
            return False
        
        exp_num = self.EXPRESSIONS[expression]
        encoded_speech = urllib.parse.quote(speech)
        endpoint = f"/api/set?expression={exp_num}&speech={encoded_speech}"
        
        result = self._make_request(endpoint)
        if result:
            print(f"設定完了: {result}")
            return True
        return False
    
    def set_color(self, color: str) -> bool:
        """色テーマを設定"""
        if color not in self.COLORS:
            print(f"無効な色: {color}")
            return False
        
        color_num = self.COLORS[color]
        endpoint = f"/api/setcolor?index={color_num}"
        
        result = self._make_request(endpoint)
        if result:
            print(f"色変更完了: {result}")
            return True
        return False
    
    def cycle_expression(self) -> bool:
        """表情をサイクル変更"""
        result = self._make_request("/api/expression")
        if result:
            print(f"表情変更: {result}")
            return True
        return False
    
    def cycle_color(self) -> bool:
        """色をサイクル変更"""
        result = self._make_request("/api/color")
        if result:
            print(f"色変更: {result}")
            return True
        return False
    
    def clear_speech(self) -> bool:
        """セリフをクリア"""
        result = self._make_request("/api/set?speech=")
        if result:
            print("セリフクリア完了")
            return True
        return False

# 使用例
if __name__ == "__main__":
    # Stack-chanクライアントを作成
    stackchan = StackChanClient("192.168.1.100")
    
    # 基本的な操作
    stackchan.set_expression_and_speech("happy", "こんにちは！")
    time.sleep(2)
    
    stackchan.set_color("blue")
    time.sleep(2)
    
    stackchan.clear_speech()
```

#### デモプログラム例

```python
import time
import random

def demo_expressions():
    """表情デモ"""
    stackchan = StackChanClient("192.168.1.100")
    
    expressions = [
        ("happy", "嬉しいです！"),
        ("sleepy", "ちょっと眠いかも..."),
        ("doubt", "うーん、どうしよう？"),
        ("neutral", "普通の状態です")
    ]
    
    print("表情デモを開始...")
    for expression, speech in expressions:
        stackchan.set_expression_and_speech(expression, speech)
        time.sleep(3)

def demo_colors():
    """色テーマデモ"""
    stackchan = StackChanClient("192.168.1.100")
    
    colors = ["blue", "green", "red", "purple", "orange", "default"]
    speeches = ["青色だよ", "緑色！", "赤色です", "紫色〜", "オレンジ色", "標準色に戻ったよ"]
    
    print("色テーマデモを開始...")
    for color, speech in zip(colors, speeches):
        stackchan.set_color(color)
        stackchan.set_expression_and_speech("happy", speech)
        time.sleep(3)

def interactive_mode():
    """対話モード"""
    stackchan = StackChanClient("192.168.1.100")
    
    print("対話モードを開始します。'quit'で終了。")
    
    while True:
        try:
            user_input = input("\nセリフを入力してください: ")
            if user_input.lower() == 'quit':
                break
            
            # ランダムに表情を選択
            expressions = list(StackChanClient.EXPRESSIONS.keys())
            random_expression = random.choice(expressions)
            
            stackchan.set_expression_and_speech(random_expression, user_input)
            
        except KeyboardInterrupt:
            break
    
    print("対話モードを終了します。")

# メイン実行部
if __name__ == "__main__":
    print("Stack-chan制御プログラム")
    print("1: 表情デモ")
    print("2: 色テーマデモ") 
    print("3: 対話モード")
    
    choice = input("選択してください (1-3): ")
    
    if choice == "1":
        demo_expressions()
    elif choice == "2":
        demo_colors()
    elif choice == "3":
        interactive_mode()
    else:
        print("無効な選択です。")
```

#### 定期実行プログラム例

```python
import schedule
import time
from datetime import datetime

def hourly_greeting():
    """毎時の挨拶"""
    stackchan = StackChanClient("192.168.1.100")
    now = datetime.now()
    
    if 6 <= now.hour < 12:
        stackchan.set_expression_and_speech("happy", "おはようございます！")
    elif 12 <= now.hour < 18:
        stackchan.set_expression_and_speech("neutral", "こんにちは！")
    else:
        stackchan.set_expression_and_speech("sleepy", "お疲れ様です")

def random_color_change():
    """ランダム色変更"""
    stackchan = StackChanClient("192.168.1.100")
    colors = list(StackChanClient.COLORS.keys())
    random_color = random.choice(colors)
    stackchan.set_color(random_color)

# スケジュール設定
schedule.every().hour.do(hourly_greeting)
schedule.every(30).minutes.do(random_color_change)

print("定期実行プログラムを開始...")
while True:
    schedule.run_pending()
    time.sleep(1)
```

#### 必要なライブラリのインストール

```bash
pip install requests schedule
```

### 完全なPythonプログラム例

詳細なPythonプログラム例は`examples/python/`フォルダに用意されています：

- **`stackchan_client.py`** - 対話的制御プログラム（表情デモ、色デモ、対話モード）
- **`stackchan_scheduler.py`** - 定期実行プログラム（時間別挨拶、自動色変更）
- **`requirements.txt`** - 必要ライブラリリスト

使用方法:

```bash
cd examples/python
pip install -r requirements.txt
python stackchan_client.py
```

### セリフ自動ループ機能

- ユーザーが設定したセリフは30秒後に自動的にクリアされます
- 設定ファイル（`simple_wifi_config.h`）にランダムセリフが登録されている場合、自動的にループ表示されます
- ランダムセリフが無効な場合は「スタックちゃん」に戻ります

### ボタン操作

- **ボタンA**: 表情サイクル変更（普通→嬉しい→眠い→困った）
- **ボタンB**: 通信モード切り替え（WiFi ⟷ BLE）
- **ボタンC**: 接続状態表示（WiFi IP / BLE状態）

### WebUI操作

ブラウザでStack-chanのIPアドレスにアクセスすると、以下の機能が利用できます：

- **表情とセリフの設定**: プルダウンで表情を選択し、テキストでセリフを入力
- **クイック操作**: 表情サイクル、色変更、セリフクリアボタン
- **色テーマ選択**: 6種類の色テーマを直接選択
- **システム状態**: メモリ使用量、稼働時間、WiFi情報の表示

## 設定カスタマイズ

### WiFi設定変更

`src/simple_wifi_config.h` でWiFi設定を変更できます：

```cpp
const WiFiCredentials wifi_networks[] = {
  {"あなたのSSID", "パスワード", 1},
  {"予備のSSID", "パスワード", 2}, 
  {nullptr, nullptr, 0}  // 終端マーカー
};
```

### ランダムセリフ設定

同じファイルでランダムセリフを設定できます：

```cpp
const char* random_speeches[] = {
  "カスタムセリフ1",
  "カスタムセリフ2",
  "お好みのセリフ",
  nullptr  // 終端マーカー
};
```

ランダムセリフを無効にする場合：

```cpp
const char* random_speeches[] = {
  nullptr  // 終端マーカーのみ
};
```

## トラブルシューティング

### WiFi接続できない

1. SSIDとパスワードを確認
2. 2.4GHz帯のWiFiを使用
3. シリアルモニターでエラー確認

### WebUIにアクセスできない

1. WiFi接続を確認
2. シリアルモニターでIPアドレス確認
3. ブラウザのキャッシュをクリア

### メッセージが表示されない

1. URLエンコードが正しいか確認
2. API応答ステータスを確認
3. シリアルモニターでログ確認

## ライセンス

MIT License

## 作者

Based on TakaoAkaki's stack-chan-tester
