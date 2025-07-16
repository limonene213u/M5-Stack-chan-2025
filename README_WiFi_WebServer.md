# Stack-chan 通信エディション 2025

## 概要
スタックチャンの通信機能に特化した日本語対応ファームウェアです。WiFi複数AP対応、BLE WebUI、Avatar表示機能を含む完全なstack-chan実装です。

## 主要機能
- 🤖 **Avatar表示**: 表情変更、色変更、日本語表示
- 📡 **WiFi接続**: 複数ネットワーク対応、自動再接続
- 🔵 **BLE WebUI**: WiFi不要でBluetooth経由でWebUIアクセス可能
- 🌐 **WebUI**: ブラウザからの操作、システム状態表示
- ⚡ **動的モード切り替え**: ボタン操作でWiFi⟷BLEモードを動的切り替え

## 起動時の接続モード選択

起動時に5秒間の選択画面が表示されます：

- **Aボタン**: WiFiモード
- **Bボタン**: BLEモード  
- **自動選択**: 5秒後にWiFiモード（デフォルト）
- **フォールバック**: WiFi失敗時は自動でBLEモード

## WiFi設定方法

### 簡単設定（推奨）

**重要**: このリポジトリは機密情報を含まない安全な構成になっています。

1. **設定ファイルを作成**:
   ```bash
   cp src/simple_wifi_config.h.example src/simple_wifi_config.h
   ```

2. **WiFi認証情報を編集**:
   `src/simple_wifi_config.h` ファイルを開いて、以下を変更してください：

   ```cpp
   const WiFiCredentials wifi_networks[] = {
     {"あなたのWiFi_SSID", "あなたのパスワード", 1},
     {"モバイルホットスポット", "モバイルパスワード", 2}, 
     {"ゲストネットワーク", "ゲストパスワード", 3},
     {nullptr, nullptr, 0}  // この行は削除しないでください
   };
   ```

3. **ランダムセリフ設定（オプション）**:
   同じファイル内でセリフをカスタマイズできます：

   ```cpp
   const char* random_speeches[] = {
     "こんにちは！",
     "今日もいい天気ですね", 
     "お疲れ様です",
     nullptr  // 終端マーカー
   };
   ```

⚠️ **注意**: `simple_wifi_config.h` ファイルは `.gitignore` に含まれているため、Git履歴に含まれません。

### 高度な設定
将来的に`data/yaml/SC_BasicConfig.yaml`での設定にも対応予定です。

## ボタン操作

- **Button A**: 表情変更（普通→嬉しい→眠い→困った）
- **Button A 長押し**: 接続モード選択メニュー
- **Button B**: WiFi⟷BLEモード即座切り替え
- **Button C**: システム状態表示（IP、BLE状態、動作時間）

## BLE機能

WiFi環境がない場合やBボタンでBLEモードに切り替え可能：

- **BLEデバイス名**: `StackChan`
- **タイムアウト**: 120秒（2分）自動停止
- **注意**: BLEは通常のブラウザからアクセスできません

### BLE WebUIアクセス方法

**Web Bluetooth APIを使用したアクセス：**
```javascript
// Webページから接続する例
navigator.bluetooth.requestDevice({
    filters: [{ name: 'StackChan' }],
    optionalServices: ['12345678-1234-5678-9abc-123456789abc']
}).then(device => {
    console.log('StackChanに接続しました');
    return device.gatt.connect();
}).then(server => {
    // サービスとキャラクタリスティックにアクセス
    return server.getPrimaryService('12345678-1234-5678-9abc-123456789abc');
}).catch(error => {
    console.log('接続エラー: ' + error);
});
```

**BLEアプリでのアクセス：**
- **Android**: nRF Connect、BLE Scanner等
- **iOS**: LightBlue Explorer等
- **PC**: BLE対応アプリ

## WebUI機能

WiFi接続後、ブラウザで以下にアクセス：

- `http://[表示されたIPアドレス]/`

### 機能一覧

- 📊 システム状態表示（メモリ、アップタイム、WiFi情報）
- 😊 表情コントロール（4種類の表情）
- 💬 セリフ機能（ランダム表示）
- 🔄 リアルタイム状態更新

### API エンドポイント

- `GET /api/expression` - 表情変更
- `GET /api/speech` - セリフ表示

## ビルド & アップロード

1. `src/simple_wifi_config.h` のWiFi設定を編集
2. PlatformIO IDEでビルド
3. M5Stack Basicに書き込み
4. シリアルモニターでIP確認

## 依存ライブラリ

- M5Unified (画面・ボタン制御)
- M5Stack-Avatar (表情表示)
- WiFi (ESP32標準)
- WebServer (ESP32標準)
- ESP32 BLE Arduino (BLE機能)

## トラブルシューティング

- WiFi接続できない → `simple_wifi_config.h` の設定を確認
- BLE接続できない → デバイス名「StackChan」を検索
- WebUIにアクセスできない → シリアルモニターでIPアドレス確認
- コンパイルエラー → M5UnifiedとAvatarライブラリのバージョン確認

## バックアップ

- クリーンなAvatar専用版: `src_backup_clean_avatar/`
- WiFi専用版（BLE無し）: 各自でバックアップ推奨
