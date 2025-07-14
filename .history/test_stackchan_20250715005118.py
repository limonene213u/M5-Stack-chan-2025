#!/usr/bin/env python3
"""
Stack-chan Communication Test Script
WiFi API とBluetooth通信のテスト用スクリプト
"""

import requests
import json
import time

class StackChanTester:
    def __init__(self, ip_address="192.168.1.100", port=80):
        self.base_url = f"http://{ip_address}:{port}"
        self.api_base = f"{self.base_url}/api"
    
    def send_message(self, message):
        """メッセージを送信"""
        url = f"{self.api_base}/message"
        data = {"message": message}
        
        try:
            response = requests.post(url, json=data, timeout=5)
            print(f"Message sent: {message}")
            print(f"Response: {response.json()}")
            return response.json()
        except Exception as e:
            print(f"Error sending message: {e}")
            return None
    
    def change_expression(self, expression):
        """表情を変更"""
        url = f"{self.api_base}/expression"
        data = {"expression": expression}
        
        try:
            response = requests.post(url, json=data, timeout=5)
            print(f"Expression changed: {expression}")
            print(f"Response: {response.json()}")
            return response.json()
        except Exception as e:
            print(f"Error changing expression: {e}")
            return None
    
    def get_status(self):
        """ステータスを取得"""
        url = f"{self.api_base}/status"
        
        try:
            response = requests.get(url, timeout=5)
            status = response.json()
            print("=== Stack-chan Status ===")
            print(f"Current Message: {status['current_message']}")
            print(f"Last Received: {status['last_received']}")
            print(f"Bluetooth: {'Connected' if status['bluetooth_connected'] else 'Disconnected'}")
            print(f"WiFi: {'Connected' if status['wifi_connected'] else 'Disconnected'}")
            print(f"IP Address: {status['ip_address']}")
            print(f"Mode: {status['mode']}")
            print("========================")
            return status
        except Exception as e:
            print(f"Error getting status: {e}")
            return None
    
    def demo_sequence(self):
        """デモンストレーション"""
        print("Starting Stack-chan Demo Sequence...")
        
        # ステータス確認
        self.get_status()
        time.sleep(2)
        
        # 挨拶
        self.send_message("こんにちは！テストを開始します")
        self.change_expression("happy")
        time.sleep(3)
        
        # 表情テスト
        expressions = ["normal", "happy", "sleepy", "doubt"]
        for expr in expressions:
            self.send_message(f"表情: {expr}")
            self.change_expression(expr)
            time.sleep(2)
        
        # 終了
        self.send_message("テスト完了しました！")
        self.change_expression("happy")
        
        print("Demo sequence completed!")

def main():
    # Stack-chanのIPアドレスを設定
    ip_address = input("Stack-chanのIPアドレスを入力してください (例: 192.168.1.100): ")
    if not ip_address:
        ip_address = "192.168.1.100"
    
    tester = StackChanTester(ip_address)
    
    while True:
        print("\n=== Stack-chan Communication Tester ===")
        print("1. メッセージ送信")
        print("2. 表情変更")  
        print("3. ステータス確認")
        print("4. デモ実行")
        print("5. 終了")
        
        choice = input("選択してください (1-5): ")
        
        if choice == "1":
            message = input("送信するメッセージを入力: ")
            tester.send_message(message)
            
        elif choice == "2":
            print("表情: normal, happy, sleepy, doubt")
            expression = input("表情を選択: ")
            tester.change_expression(expression)
            
        elif choice == "3":
            tester.get_status()
            
        elif choice == "4":
            tester.demo_sequence()
            
        elif choice == "5":
            print("終了します")
            break
            
        else:
            print("無効な選択です")

if __name__ == "__main__":
    main()
