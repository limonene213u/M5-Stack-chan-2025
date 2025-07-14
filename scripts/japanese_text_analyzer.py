#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
日本語テキスト解析スクリプト
使用される文字の抽出と必要な文字セットの分析
"""

import re
import unicodedata

def analyze_japanese_text():
    """設定ファイルとコードから必要な日本語文字を分析"""
    
    # 設定ファイルの日本語テキスト
    config_texts = [
        "こんにちは",
        "今日もいい天気ですね",
        "何かお手伝いできることはありますか？",
        "スタックチャンです",
        "WiFi接続を確認中...",
        "Bluetooth待機中...",
        "準備完了です！",
        "メッセージをお待ちしています",
        "表情を変更しました",
        "システム正常です"
    ]
    
    # コード内の日本語テキスト
    code_texts = [
        "WiFi受信:",
        "BT受信:",
        "カスタム日本語フォント",
        "初期化中...",
        "システム初期化中...",
        "Bluetoothモード",
        "両方モード",
        "WiFiモード",
        "表情を変更しました！",
        "接続成功",
        "接続失敗",
        "接続中",
        "未接続",
        "接続済み",
        "待機中...",
        "Bluetooth接続しました",
        "Bluetooth切断されました"
    ]
    
    # 追加でよく使われる文字
    common_texts = [
        "はい", "いいえ", "ありがとう", "すみません", "おはよう", "こんばんは",
        "お疲れ様", "よろしく", "頑張って", "大丈夫", "問題ありません",
        "エラー", "警告", "注意", "確認", "設定", "変更", "保存", "読み込み",
        "開始", "停止", "終了", "再起動", "リセット", "クリア", "削除",
        "追加", "更新", "送信", "受信", "接続", "切断", "認証", "ログイン",
        "時間", "分", "秒", "年", "月", "日", "曜日", "午前", "午後",
        "バッテリー", "電源", "充電", "容量", "温度", "湿度", "気圧"
    ]
    
    all_texts = config_texts + code_texts + common_texts
    
    # 文字を分類
    hiragana_chars = set()
    katakana_chars = set()
    kanji_chars = set()
    other_chars = set()
    
    for text in all_texts:
        for char in text:
            if '\u3040' <= char <= '\u309F':  # ひらがな
                hiragana_chars.add(char)
            elif '\u30A0' <= char <= '\u30FF':  # カタカナ
                katakana_chars.add(char)
            elif '\u4E00' <= char <= '\u9FAF':  # 漢字
                kanji_chars.add(char)
            elif char not in ' \n\t.,!?()[]{}":;/\\-_=+*&%$#@~`|<>':
                other_chars.add(char)
    
    print("=== 日本語文字解析結果 ===\n")
    
    print(f"ひらがな ({len(hiragana_chars)}文字):")
    print(''.join(sorted(hiragana_chars)))
    print()
    
    print(f"カタカナ ({len(katakana_chars)}文字):")
    print(''.join(sorted(katakana_chars)))
    print()
    
    print(f"漢字 ({len(kanji_chars)}文字):")
    print(''.join(sorted(kanji_chars)))
    print()
    
    if other_chars:
        print(f"その他の文字 ({len(other_chars)}文字):")
        print(''.join(sorted(other_chars)))
        print()
    
    return hiragana_chars, katakana_chars, kanji_chars, other_chars

def generate_complete_kana_sets():
    """完全なひらがな・カタカナセットを生成"""
    
    # 完全なひらがなセット（小文字含む）
    hiragana_complete = [
        'あ', 'い', 'う', 'え', 'お',
        'か', 'き', 'く', 'け', 'こ', 'が', 'ぎ', 'ぐ', 'げ', 'ご',
        'さ', 'し', 'す', 'せ', 'そ', 'ざ', 'じ', 'ず', 'ぜ', 'ぞ',
        'た', 'ち', 'つ', 'て', 'と', 'だ', 'ぢ', 'づ', 'で', 'ど',
        'な', 'に', 'ぬ', 'ね', 'の',
        'は', 'ひ', 'ふ', 'へ', 'ほ', 'ば', 'び', 'ぶ', 'べ', 'ぼ', 'ぱ', 'ぴ', 'ぷ', 'ぺ', 'ぽ',
        'ま', 'み', 'む', 'め', 'も',
        'や', 'ゆ', 'よ',
        'ら', 'り', 'る', 'れ', 'ろ',
        'わ', 'ゐ', 'ゑ', 'を', 'ん',
        'ゃ', 'ゅ', 'ょ', 'っ'
    ]
    
    # 完全なカタカナセット（小文字含む）
    katakana_complete = [
        'ア', 'イ', 'ウ', 'エ', 'オ',
        'カ', 'キ', 'ク', 'ケ', 'コ', 'ガ', 'ギ', 'グ', 'ゲ', 'ゴ',
        'サ', 'シ', 'ス', 'セ', 'ソ', 'ザ', 'ジ', 'ズ', 'ゼ', 'ゾ',
        'タ', 'チ', 'ツ', 'テ', 'ト', 'ダ', 'ヂ', 'ヅ', 'デ', 'ド',
        'ナ', 'ニ', 'ヌ', 'ネ', 'ノ',
        'ハ', 'ヒ', 'フ', 'ヘ', 'ホ', 'バ', 'ビ', 'ブ', 'ベ', 'ボ', 'パ', 'ピ', 'プ', 'ペ', 'ポ',
        'マ', 'ミ', 'ム', 'メ', 'モ',
        'ヤ', 'ユ', 'ヨ',
        'ラ', 'リ', 'ル', 'レ', 'ロ',
        'ワ', 'ヰ', 'ヱ', 'ヲ', 'ン',
        'ャ', 'ュ', 'ョ', 'ッ', 'ー'
    ]
    
    print("=== 完全なかなセット ===\n")
    print(f"ひらがな完全セット ({len(hiragana_complete)}文字):")
    print(''.join(hiragana_complete))
    print()
    
    print(f"カタカナ完全セット ({len(katakana_complete)}文字):")
    print(''.join(katakana_complete))
    print()
    
    return hiragana_complete, katakana_complete

def print_unicode_info(chars, title):
    """文字のUnicode情報を出力"""
    print(f"=== {title} Unicode情報 ===")
    for char in sorted(chars):
        unicode_val = ord(char)
        print(f"'{char}' : U+{unicode_val:04X} ({unicode_val})")
    print()

if __name__ == "__main__":
    print("Stack-chan 日本語文字解析\n")
    
    # 実際に使用される文字を解析
    used_hiragana, used_katakana, used_kanji, used_other = analyze_japanese_text()
    
    # 完全なかなセットを生成
    complete_hiragana, complete_katakana = generate_complete_kana_sets()
    
    # Unicode情報を出力
    print_unicode_info(used_kanji, "使用される漢字")
    
    # フォント実装の優先度を提案
    print("=== フォント実装優先度 ===")
    print("1. 完全ひらがなセット (74文字)")
    print("2. 完全カタカナセット (75文字)")
    print(f"3. 実際に使用される漢字 ({len(used_kanji)}文字)")
    print("4. 基本的な記号・数字")
    print(f"\n合計推定文字数: {74 + 75 + len(used_kanji) + 20}文字")
    print(f"推定メモリ使用量: {(74 + 75 + len(used_kanji) + 20) * 18}バイト ({(74 + 75 + len(used_kanji) + 20) * 18 / 1024:.1f}KB)")
