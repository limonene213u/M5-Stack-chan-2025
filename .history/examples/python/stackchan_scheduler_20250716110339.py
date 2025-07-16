#!/usr/bin/env python3
"""
Stack-chan定期実行プログラム

このスクリプトは定期的にStack-chanの表情や色を変更します。
時間帯によって挨拶を変えたり、ランダムに色を変更したりします。

必要なライブラリ:
pip install requests schedule

使用方法:
1. Stack-chanのIPアドレスを設定
2. スクリプトを実行
3. Ctrl+Cで停止

作者: Based on Stack-chan API
ライセンス: MIT
"""

import schedule
import time
import random
from datetime import datetime
from stackchan_client import StackChanClient

class StackChanScheduler:
    """Stack-chan定期実行管理クラス"""
    
    def __init__(self, ip_address: str = "192.168.1.100"):
        self.stackchan = StackChanClient(ip_address)
        self.last_color = None
        
    def hourly_greeting(self):
        """毎時の挨拶"""
        now = datetime.now()
        hour = now.hour
        
        print(f"\n[{now.strftime('%H:%M:%S')}] 時間別挨拶実行")
        
        if 6 <= hour < 10:
            messages = [
                "おはようございます！",
                "今日も一日頑張りましょう！",
                "良い朝ですね！"
            ]
            expression = "happy"
        elif 10 <= hour < 12:
            messages = [
                "午前中ですね",
                "今日の調子はどうですか？",
                "頑張って！"
            ]
            expression = "neutral"
        elif 12 <= hour < 14:
            messages = [
                "お昼の時間です",
                "お昼ご飯の時間ですね",
                "午後も頑張りましょう"
            ]
            expression = "happy"
        elif 14 <= hour < 18:
            messages = [
                "午後の時間ですね",
                "今日も頑張ってますね",
                "もう少しで夕方です"
            ]
            expression = "neutral"
        elif 18 <= hour < 22:
            messages = [
                "夕方ですね",
                "お疲れ様です",
                "今日も一日お疲れ様でした"
            ]
            expression = "sleepy"
        else:
            messages = [
                "夜の時間ですね",
                "お疲れ様でした",
                "ゆっくり休んでください"
            ]
            expression = "sleepy"
        
        message = random.choice(messages)
        self.stackchan.set_expression_and_speech(expression, message)
    
    def random_color_change(self):
        """ランダム色変更"""
        colors = list(StackChanClient.COLORS.keys())
        
        # 同じ色の連続を避ける
        available_colors = [c for c in colors if c != self.last_color]
        selected_color = random.choice(available_colors)
        
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] ランダム色変更: {selected_color}")
        
        if self.stackchan.set_color(selected_color):
            self.last_color = selected_color
    
    def random_expression_change(self):
        """ランダム表情変更"""
        expressions = list(StackChanClient.EXPRESSIONS.keys())
        selected_expression = random.choice(expressions)
        
        messages = {
            'neutral': ['普通の気分です', 'いつも通りです', '平常心'],
            'happy': ['嬉しいです！', 'ハッピー！', '気分がいいです'],
            'sleepy': ['ちょっと眠い...', 'うとうと...', 'ねむねむ'],
            'doubt': ['うーん？', 'どうしよう...', '困ったな']
        }
        
        message = random.choice(messages[selected_expression])
        
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] ランダム表情変更: {selected_expression}")
        self.stackchan.set_expression_and_speech(selected_expression, message)
    
    def weather_check(self):
        """天気チェック風のメッセージ（実際の天気APIは使用せず、ランダム）"""
        weather_messages = [
            ("今日はいい天気ですね！", "happy"),
            ("曇りがちな天気です", "neutral"),
            ("雨が降りそうです", "doubt"),
            ("暖かい日ですね", "happy"),
            ("涼しくて過ごしやすいです", "neutral")
        ]
        
        message, expression = random.choice(weather_messages)
        
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] 天気チェック")
        self.stackchan.set_expression_and_speech(expression, message)
    
    def motivational_message(self):
        """励ましのメッセージ"""
        messages = [
            ("頑張って！", "happy"),
            ("きっと大丈夫！", "happy"),
            ("応援してます", "happy"),
            ("ファイト！", "happy"),
            ("今日もいい日にしよう", "neutral")
        ]
        
        message, expression = random.choice(messages)
        
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] 励ましメッセージ")
        self.stackchan.set_expression_and_speech(expression, message)
    
    def setup_schedule(self):
        """スケジュール設定"""
        print("スケジュール設定中...")
        
        # 毎時0分に挨拶
        schedule.every().hour.at(":00").do(self.hourly_greeting)
        
        # 30分ごとに色変更
        schedule.every(30).minutes.do(self.random_color_change)
        
        # 45分ごとに表情変更
        schedule.every(45).minutes.do(self.random_expression_change)
        
        # 平日の9時、13時、17時に励ましメッセージ
        schedule.every().monday.at("09:00").do(self.motivational_message)
        schedule.every().monday.at("13:00").do(self.motivational_message)
        schedule.every().monday.at("17:00").do(self.motivational_message)
        
        schedule.every().tuesday.at("09:00").do(self.motivational_message)
        schedule.every().tuesday.at("13:00").do(self.motivational_message)
        schedule.every().tuesday.at("17:00").do(self.motivational_message)
        
        schedule.every().wednesday.at("09:00").do(self.motivational_message)
        schedule.every().wednesday.at("13:00").do(self.motivational_message)
        schedule.every().wednesday.at("17:00").do(self.motivational_message)
        
        schedule.every().thursday.at("09:00").do(self.motivational_message)
        schedule.every().thursday.at("13:00").do(self.motivational_message)
        schedule.every().thursday.at("17:00").do(self.motivational_message)
        
        schedule.every().friday.at("09:00").do(self.motivational_message)
        schedule.every().friday.at("13:00").do(self.motivational_message)
        schedule.every().friday.at("17:00").do(self.motivational_message)
        
        # 12時と18時に天気チェック風メッセージ
        schedule.every().day.at("12:00").do(self.weather_check)
        schedule.every().day.at("18:00").do(self.weather_check)
        
        print("スケジュール設定完了")
        self.print_schedule()
    
    def print_schedule(self):
        """設定されたスケジュールを表示"""
        print("\n=== 設定されたスケジュール ===")
        print("- 毎時0分: 時間別挨拶")
        print("- 30分ごと: ランダム色変更")
        print("- 45分ごと: ランダム表情変更")
        print("- 平日 9時/13時/17時: 励ましメッセージ")
        print("- 毎日 12時/18時: 天気チェック風メッセージ")
        print("=" * 30)
    
    def run(self):
        """定期実行開始"""
        print("Stack-chan定期実行プログラム開始")
        print("Ctrl+Cで停止")
        
        # 接続テスト
        if not self.stackchan.test_connection():
            print("Stack-chanに接続できません。IPアドレスを確認してください。")
            return
        
        # 開始メッセージ
        self.stackchan.set_expression_and_speech("happy", "定期実行プログラムを開始します！")
        
        # スケジュール設定
        self.setup_schedule()
        
        # メインループ
        try:
            while True:
                schedule.run_pending()
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\n定期実行を停止します...")
            self.stackchan.set_expression_and_speech("neutral", "定期実行を停止しました")
            print("プログラムを終了しました。")

def main():
    """メイン関数"""
    print("Stack-chan定期実行プログラム")
    print("=" * 40)
    
    # IPアドレス入力
    ip = input("Stack-chanのIPアドレスを入力してください (デフォルト: 192.168.1.100): ")
    if not ip.strip():
        ip = "192.168.1.100"
    
    # スケジューラー作成・実行
    scheduler = StackChanScheduler(ip)
    scheduler.run()

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"エラーが発生しました: {e}")
