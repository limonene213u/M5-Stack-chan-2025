#!/usr/bin/env python3
"""
Stack-chan制御用Pythonクライアント

使用方法:
1. requestsライブラリをインストール: pip install requests
2. Stack-chanのIPアドレスを確認
3. このスクリプトを実行

作者: Based on Stack-chan API
ライセンス: MIT
"""

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
        print(f"Stack-chanクライアント初期化: {self.base_url}")
    
    def _make_request(self, endpoint: str) -> Optional[str]:
        """APIリクエストを実行"""
        try:
            url = f"{self.base_url}{endpoint}"
            print(f"リクエスト: {url}")
            response = requests.get(url, timeout=self.timeout)
            response.raise_for_status()
            return response.text
        except requests.exceptions.RequestException as e:
            print(f"エラー: {e}")
            return None
    
    def set_expression_and_speech(self, expression: str, speech: str) -> bool:
        """表情とセリフを同時に設定"""
        if expression not in self.EXPRESSIONS:
            print(f"無効な表情: {expression}")
            print(f"利用可能な表情: {list(self.EXPRESSIONS.keys())}")
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
            print(f"利用可能な色: {list(self.COLORS.keys())}")
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
    
    def test_connection(self) -> bool:
        """接続テスト"""
        print("接続テスト中...")
        result = self._make_request("/")
        if result:
            print("接続成功！")
            return True
        else:
            print("接続失敗。IPアドレスとネットワーク接続を確認してください。")
            return False

def demo_expressions(stackchan: StackChanClient):
    """表情デモ"""
    expressions = [
        ("happy", "嬉しいです！"),
        ("sleepy", "ちょっと眠いかも..."),
        ("doubt", "うーん、どうしよう？"),
        ("neutral", "普通の状態です")
    ]
    
    print("\n=== 表情デモを開始 ===")
    for expression, speech in expressions:
        print(f"表情: {expression}, セリフ: {speech}")
        stackchan.set_expression_and_speech(expression, speech)
        time.sleep(3)
    print("表情デモ完了")

def demo_colors(stackchan: StackChanClient):
    """色テーマデモ"""
    colors = ["blue", "green", "red", "purple", "orange", "default"]
    speeches = ["青色だよ", "緑色！", "赤色です", "紫色〜", "オレンジ色", "標準色に戻ったよ"]
    
    print("\n=== 色テーマデモを開始 ===")
    for color, speech in zip(colors, speeches):
        print(f"色: {color}, セリフ: {speech}")
        stackchan.set_color(color)
        stackchan.set_expression_and_speech("happy", speech)
        time.sleep(3)
    print("色テーマデモ完了")

def interactive_mode(stackchan: StackChanClient):
    """対話モード"""
    print("\n=== 対話モードを開始 ===")
    print("セリフを入力してください。'quit'で終了。")
    print("特殊コマンド:")
    print("  'cycle' - 表情サイクル")
    print("  'color' - 色サイクル")
    print("  'clear' - セリフクリア")
    
    while True:
        try:
            user_input = input("\n> ")
            if user_input.lower() == 'quit':
                break
            elif user_input.lower() == 'cycle':
                stackchan.cycle_expression()
            elif user_input.lower() == 'color':
                stackchan.cycle_color()
            elif user_input.lower() == 'clear':
                stackchan.clear_speech()
            else:
                # ランダムに表情を選択
                expressions = list(StackChanClient.EXPRESSIONS.keys())
                random_expression = random.choice(expressions)
                stackchan.set_expression_and_speech(random_expression, user_input)
                
        except KeyboardInterrupt:
            break
    
    print("対話モードを終了します。")

def main():
    """メイン関数"""
    print("Stack-chan制御プログラム")
    print("=" * 40)
    
    # IPアドレス入力
    ip = input("Stack-chanのIPアドレスを入力してください (デフォルト: 192.168.1.100): ")
    if not ip.strip():
        ip = "192.168.1.100"
    
    # クライアント作成
    stackchan = StackChanClient(ip)
    
    # 接続テスト
    if not stackchan.test_connection():
        print("接続に失敗しました。プログラムを終了します。")
        return
    
    # メニュー表示
    while True:
        print("\n" + "=" * 40)
        print("メニューを選択してください:")
        print("1: 表情デモ")
        print("2: 色テーマデモ") 
        print("3: 対話モード")
        print("4: 接続テスト")
        print("0: 終了")
        
        choice = input("選択 (0-4): ")
        
        if choice == "1":
            demo_expressions(stackchan)
        elif choice == "2":
            demo_colors(stackchan)
        elif choice == "3":
            interactive_mode(stackchan)
        elif choice == "4":
            stackchan.test_connection()
        elif choice == "0":
            print("プログラムを終了します。")
            break
        else:
            print("無効な選択です。")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nプログラムが中断されました。")
    except Exception as e:
        print(f"予期しないエラーが発生しました: {e}")
