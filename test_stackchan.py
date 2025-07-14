#!/usr/bin/env python3
"""
スタックチャン 通信テストスクリプト
WiFi API、Bluetooth通信、WiFi管理機能のテスト用スクリプト
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
            print(f"メッセージ送信: {message}")
            print(f"応答: {response.json()}")
            return response.json()
        except Exception as e:
            print(f"メッセージ送信エラー: {e}")
            return None
    
    def change_expression(self, expression):
        """表情を変更"""
        url = f"{self.api_base}/expression"
        data = {"expression": expression}
        
        try:
            response = requests.post(url, json=data, timeout=5)
            print(f"表情変更: {expression}")
            print(f"応答: {response.json()}")
            return response.json()
        except Exception as e:
            print(f"表情変更エラー: {e}")
            return None
    
    def get_status(self):
        """ステータスを取得"""
        url = f"{self.api_base}/status"
        
        try:
            response = requests.get(url, timeout=5)
            status = response.json()
            print("=== スタックチャン ステータス ===")
            print(f"現在のメッセージ: {status['current_message']}")
            print(f"最後の受信: {status['last_received']}")
            print(f"Bluetooth: {'接続済み' if status['bluetooth_connected'] else '未接続'}")
            print(f"WiFi状態: {status['wifi_status']}")
            print(f"IPアドレス: {status['ip_address']}")
            print(f"接続クライアント数: {status['connected_clients']}")
            print(f"APモード: {'はい' if status['is_ap_mode'] else 'いいえ'}")
            print("==============================")
            return status
        except Exception as e:
            print(f"ステータス取得エラー: {e}")
            return None
    
    def scan_wifi_networks(self):
        """WiFiネットワークをスキャン"""
        url = f"{self.api_base}/wifi/scan"
        
        try:
            response = requests.get(url, timeout=10)
            networks = response.json()
            print("=== WiFiネットワーク ===")
            for network in networks['networks']:
                status = ""
                if network['connected']:
                    status += " (接続中)"
                if network['configured']:
                    status += f" [設定済み:優先度{network.get('priority', '?')}]"
                if not network['available']:
                    status += " (圏外)"
                    
                print(f"- {network['ssid']}{status}")
            print("======================")
            return networks
        except Exception as e:
            print(f"WiFiスキャンエラー: {e}")
            return None
    
    def toggle_wifi_mode(self):
        """WiFiモードを切り替え"""
        url = f"{self.api_base}/wifi/toggle"
        
        try:
            response = requests.post(url, timeout=5)
            result = response.json()
            print(f"WiFiモード切り替え: {result['mode']}")
            print(f"IPアドレス: {result['ip']}")
            return result
        except Exception as e:
            print(f"WiFiモード切り替えエラー: {e}")
            return None
    
    def add_wifi_network(self, ssid, password, priority=5):
        """WiFiネットワークを追加"""
        url = f"{self.api_base}/wifi/add"
        data = {"ssid": ssid, "password": password, "priority": priority}
        
        try:
            response = requests.post(url, json=data, timeout=5)
            result = response.json()
            print(f"WiFiネットワーク追加: {ssid} (優先度: {priority})")
            print(f"応答: {result}")
            return result
        except Exception as e:
            print(f"WiFiネットワーク追加エラー: {e}")
            return None
    
    def connect_to_network(self, ssid):
        """指定されたWiFiネットワークに接続"""
        url = f"{self.api_base}/wifi/connect"
        data = {"ssid": ssid}
        
        try:
            response = requests.post(url, json=data, timeout=10)
            result = response.json()
            print(f"WiFi接続試行: {ssid}")
            print(f"結果: {result['message']}")
            return result
        except Exception as e:
            print(f"WiFi接続エラー: {e}")
            return None
    
    def demo_sequence(self):
        """デモンストレーション"""
        print("スタックチャン デモシーケンスを開始します...")
        
        # ステータス確認
        self.get_status()
        time.sleep(2)
        
        # WiFiネットワークスキャン
        self.scan_wifi_networks()
        time.sleep(2)
        
        # 挨拶
        self.send_message("こんにちは！テストを開始します")
        self.change_expression("happy")
        time.sleep(3)
        
        # 表情テスト
        expressions = [
            ("normal", "普通の表情です"),
            ("happy", "嬉しい表情です"), 
            ("sleepy", "眠い表情です"),
            ("doubt", "疑問の表情です")
        ]
        
        for expr, msg in expressions:
            self.send_message(msg)
            self.change_expression(expr)
            time.sleep(2)
        
        # WiFi機能テスト
        self.send_message("WiFi機能をテストします")
        time.sleep(2)
        
        # 終了
        self.send_message("テスト完了しました！ありがとうございました")
        self.change_expression("happy")
        
        print("デモシーケンス完了！")

def main():
    print("=== スタックチャン 通信テスター ===")
    
    # スタックチャンのIPアドレスを設定
    ip_address = input("スタックチャンのIPアドレスを入力してください (例: 192.168.1.100): ")
    if not ip_address:
        ip_address = "192.168.1.100"
    
    tester = StackChanTester(ip_address)
    
    while True:
        print("\n=== スタックチャン 通信テスター ===")
        print("1. メッセージ送信")
        print("2. 表情変更")  
        print("3. ステータス確認")
        print("4. WiFiネットワークスキャン")
        print("5. WiFiモード切り替え")
        print("6. WiFiネットワーク追加")
        print("7. WiFiネットワーク接続")
        print("8. デモ実行")
        print("9. 終了")
        
        choice = input("選択してください (1-9): ")
        
        if choice == "1":
            message = input("送信するメッセージを入力: ")
            tester.send_message(message)
            
        elif choice == "2":
            print("表情: normal(普通), happy(嬉しい), sleepy(眠い), doubt(疑問)")
            expression = input("表情を選択: ")
            tester.change_expression(expression)
            
        elif choice == "3":
            tester.get_status()
            
        elif choice == "4":
            tester.scan_wifi_networks()
            
        elif choice == "5":
            tester.toggle_wifi_mode()
            
        elif choice == "6":
            ssid = input("追加するWiFi SSID: ")
            password = input("パスワード: ")
            priority = input("優先度 (1-10, デフォルト:5): ")
            priority = int(priority) if priority.isdigit() else 5
            tester.add_wifi_network(ssid, password, priority)
            
        elif choice == "7":
            ssid = input("接続するWiFi SSID: ")
            tester.connect_to_network(ssid)
            
        elif choice == "8":
            tester.demo_sequence()
            
        elif choice == "9":
            print("終了します")
            break
            
        else:
            print("無効な選択です")

if __name__ == "__main__":
    main()
