# Stack-chan セットアップガイド

## クイックスタート

### 1. リポジトリをクローン
```bash
git clone https://github.com/limonene213u/M5-Stack-chan-2025.git
cd M5-Stack-chan-2025
```

### 2. WiFi設定ファイルを作成
```bash
# サンプルファイルから実際の設定ファイルを作成
cp src/simple_wifi_config.h.example src/simple_wifi_config.h
```

### 3. WiFi認証情報を編集
`src/simple_wifi_config.h` を開いて、実際の認証情報に変更：

```cpp
const WiFiCredentials wifi_networks[] = {
  {"あなたのWiFi_SSID", "あなたのパスワード", 1},
  {"予備のネットワーク", "予備のパスワード", 2},
  {nullptr, nullptr, 0}  // 終端マーカー
};
```

### 4. ビルド & アップロード
```bash
# PlatformIO でビルド
pio run -e m5stack-grey

# M5Stack にアップロード
pio run -e m5stack-grey -t upload
```

## 重要な注意事項

### セキュリティ

- ✅ **安全**: `simple_wifi_config.h.example` - サンプルファイル（Git管理対象）
- ⚠️ **機密**: `simple_wifi_config.h` - 実際の設定ファイル（Git管理対象外）

### .gitignore 設定

以下のファイルは自動的にGit履歴から除外されます：

```
# 機密設定ファイル
src/simple_wifi_config.h
data/wifi_config.yaml
.history/
*.secret
*.private
```

### トラブルシューティング

1. **設定ファイルが見つからない**:
   ```bash
   cp src/simple_wifi_config.h.example src/simple_wifi_config.h
   ```

2. **WiFi接続できない**:
   - `simple_wifi_config.h` の認証情報を確認
   - シリアルモニターでログを確認

3. **コンパイルエラー**:
   - M5UnifiedとAvatarライブラリのバージョン確認
   - PlatformIOの依存関係を更新

## 機能一覧

- 🤖 Avatar表示（表情・色変更）
- 📡 WiFi接続（複数ネットワーク対応）
- 🔵 BLE WebUI（WiFi不要でアクセス可能）
- 🌐 WebUI（ブラウザから操作）
- ⚡ 動的モード切り替え（WiFi⟷BLE）

## ボタン操作

- **Button A**: 表情変更
- **Button A 長押し**: 接続モード選択メニュー
- **Button B**: WiFi⟷BLE即座切り替え
- **Button C**: システム状態表示

## 開発者向け

### ブランチ構成
- `main`: 安定版（機密情報除去済み）
- `dev`: 開発版（機密情報除去済み）

### セキュリティコミット
このリポジトリは過去の機密情報を完全に除去した状態で再構成されています。
