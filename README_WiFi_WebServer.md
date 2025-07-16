# Stack-chan Avatar + WiFi + WebServer Edition

## 概要
Avatar表示 + WiFi接続 + WebUI機能を持つStack-chan実装です。

## 機能
- 🤖 **Avatar表示**: 表情変更、色変更、日本語表示
- 📡 **WiFi接続**: 複数ネットワーク対応、自動再接続
- 🌐 **WebUI**: ブラウザからの操作、システム状態表示

## WiFi設定方法

### 簡単設定（推奨）
`src/simple_wifi_config.h` ファイルを編集してください：

```cpp
const WiFiCredentials wifi_networks[] = {
  {"あなたのWiFi_SSID", "あなたのパスワード", 1},
  {"モバイルホットスポット", "モバイルパスワード", 2}, 
  {"ゲストネットワーク", "ゲストパスワード", 3},
  {nullptr, nullptr, 0}  // この行は削除しないでください
};
```

### YAML設定（高度な設定）
`data/wifi_config.yaml` ファイルも利用可能です（現在は参考用）。

## ボタン操作
- **Button A**: 表情変更（普通→嬉しい→眠い→困った）
- **Button A 長押し**: 接続モード選択画面
- **Button B**: WiFi⟷BLEモード切り替え
- **Button C**: 詳細接続状態表示

## BLE機能
WiFi環境がない場合やBボタンでBLEモードに切り替え可能：
- **BLEデバイス名**: `StackChan`
- **ペアリング時間**: 120秒（従来の倍以上）
- **推奨アプリ**: LightBlue (iOS)、nRF Connect (Android)

## WebUI機能
WiFi接続後、ブラウザで以下にアクセス：
- `http://[表示されたIPアドレス]/`

### WebUI機能
- 📊 システム状態表示（メモリ、アップタイム、WiFi情報）
- 😊 表情変更（ブラウザから）
- 🎨 色変更（ブラウザから）
- 🔄 リアルタイム更新

### API エンドポイント
- `GET /api/expression` - 表情変更
- `GET /api/color` - 色変更

## ビルド & アップロード
1. `src/simple_wifi_config.h` のWiFi設定を編集
2. PlatformIOでビルド: `pio run -e m5stack-grey`
3. アップロード: `pio run -e m5stack-grey -t upload`

## 依存ライブラリ
- M5Unified (画面・ボタン制御)
- M5Stack-Avatar (顔表示)
- WiFi (標準ライブラリ)
- WebServer (標準ライブラリ)

## トラブルシューティング
- WiFi接続できない → `simple_wifi_config.h` の設定を確認
- Avatar表示されない → メモリ不足の可能性、シリアルモニタでログ確認
- WebUIにアクセスできない → IPアドレスが正しく表示されているか確認

## バックアップ
- クリーンなAvatar専用版: `src_backup_clean_avatar/`
- 現在のWiFi+WebServer版: `src/`
