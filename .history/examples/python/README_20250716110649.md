# Stack-chan Python Examples

Stack-chanのAPIを使用したPythonプログラム例集です。

## ファイル構成

- `stackchan_client.py` - 基本的なStack-chan制御クライアント
- `stackchan_scheduler.py` - 定期実行プログラム
- `requirements.txt` - 必要なPythonライブラリ

## セットアップ

### 1. 必要なライブラリのインストール

```bash
pip install -r requirements.txt
```

または個別にインストール:

```bash
pip install requests schedule
```

### 2. Stack-chanのIPアドレス確認

Stack-chanが接続されているWiFiネットワークでのIPアドレスを確認してください。
シリアルモニターまたはStack-chanの画面に表示されます。

## 使用方法

### 基本クライアント (`stackchan_client.py`)

対話的にStack-chanを制御できるプログラムです。

```bash
python stackchan_client.py
```

機能:
- 表情デモ（4種類の表情を順番に表示）
- 色テーマデモ（6種類の色を順番に表示）
- 対話モード（リアルタイムでメッセージ送信）
- 接続テスト

### 定期実行プログラム (`stackchan_scheduler.py`)

時間に応じて自動的にStack-chanの表情や色を変更するプログラムです。

```bash
python stackchan_scheduler.py
```

スケジュール:
- **毎時0分**: 時間帯に応じた挨拶
- **30分ごと**: ランダム色変更
- **45分ごと**: ランダム表情変更
- **平日 9時/13時/17時**: 励ましメッセージ
- **毎日 12時/18時**: 天気チェック風メッセージ

## API使用例

### 基本的な使用方法

```python
from stackchan_client import StackChanClient

# クライアント作成
stackchan = StackChanClient("192.168.1.100")

# 表情とセリフを設定
stackchan.set_expression_and_speech("happy", "こんにちは！")

# 色を変更
stackchan.set_color("blue")

# セリフをクリア
stackchan.clear_speech()
```

### 利用可能な表情

- `neutral` - 普通
- `happy` - 嬉しい
- `sleepy` - 眠い
- `doubt` - 困った

### 利用可能な色

- `default` - 標準色
- `blue` - 青系
- `green` - 緑系
- `red` - 赤系
- `purple` - 紫系
- `orange` - オレンジ系

## カスタマイズ

### 独自のプログラム作成

`StackChanClient`クラスを使用して独自のプログラムを作成できます：

```python
from stackchan_client import StackChanClient
import time

def my_custom_program():
    stackchan = StackChanClient("192.168.1.100")
    
    # カスタム処理
    stackchan.set_expression_and_speech("happy", "カスタムプログラム開始！")
    time.sleep(2)
    
    # 色をサイクル
    colors = ["red", "green", "blue"]
    for color in colors:
        stackchan.set_color(color)
        time.sleep(1)

if __name__ == "__main__":
    my_custom_program()
```

### エラーハンドリング

```python
try:
    stackchan = StackChanClient("192.168.1.100")
    if stackchan.test_connection():
        stackchan.set_expression_and_speech("happy", "接続成功！")
    else:
        print("Stack-chanに接続できません")
except Exception as e:
    print(f"エラーが発生しました: {e}")
```

## トラブルシューティング

### 接続できない場合

1. **IPアドレスを確認**: Stack-chanの正しいIPアドレスを使用しているか確認
2. **ネットワーク接続**: PCとStack-chanが同じネットワークに接続されているか確認
3. **ファイアウォール**: ファイアウォールがHTTP通信をブロックしていないか確認

### よくあるエラー

- `requests.exceptions.ConnectTimeout`: タイムアウト → IPアドレスまたはネットワーク接続を確認
- `requests.exceptions.ConnectionError`: 接続エラー → Stack-chanの電源とWiFi接続を確認
- `ModuleNotFoundError`: ライブラリ未インストール → `pip install -r requirements.txt`を実行

## ライセンス

MIT License

## 作者

Based on Stack-chan API
