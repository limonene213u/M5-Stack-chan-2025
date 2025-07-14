#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
シンプルな日本語フォント生成スクリプト（デバッグ用）
問題のある自動生成フォントを手動作成の安全なフォントに置換
"""

def generate_simple_font_header():
    """手動で作成したシンプルで安全な日本語フォントを生成"""
    
    # シンプルなテスト用フォントデータ（手動作成）
    simple_fonts = [
        # スペース
        ('0x0020', ' ', [
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # ! (感嘆符)
        ('0x0021', '!', [
            0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
            0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # A
        ('0x0041', 'A', [
            0x00, 0x00, 0x18, 0x24, 0x24, 0x42, 0x7E, 0x42,
            0x42, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # あ (ひらがな)
        ('0x3042', 'あ', [
            0x00, 0x00, 0x1C, 0x20, 0x18, 0x24, 0x24, 0x24,
            0x24, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # い (ひらがな)
        ('0x3044', 'い', [
            0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # う (ひらがな)
        ('0x3046', 'う', [
            0x00, 0x00, 0x1C, 0x20, 0x20, 0x1C, 0x02, 0x02,
            0x22, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # か (ひらがな)
        ('0x304B', 'か', [
            0x00, 0x00, 0x20, 0x20, 0x3E, 0x20, 0x20, 0x22,
            0x22, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # こ (ひらがな)
        ('0x3053', 'こ', [
            0x00, 0x00, 0x3E, 0x02, 0x02, 0x02, 0x02, 0x02,
            0x22, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # ん (ひらがな)
        ('0x3093', 'ん', [
            0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x32,
            0x2A, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # に (ひらがな)
        ('0x306B', 'に', [
            0x00, 0x00, 0x3E, 0x02, 0x02, 0x1E, 0x02, 0x02,
            0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # ち (ひらがな)
        ('0x3061', 'ち', [
            0x00, 0x00, 0x04, 0x04, 0x04, 0x3E, 0x04, 0x04,
            0x0C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # は (ひらがな)
        ('0x306F', 'は', [
            0x00, 0x00, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22,
            0x22, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # 日 (漢字)
        ('0x65E5', '日', [
            0x00, 0x00, 0x1E, 0x12, 0x12, 0x1E, 0x12, 0x12,
            0x12, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # 本 (漢字)
        ('0x672C', '本', [
            0x00, 0x00, 0x08, 0x08, 0x08, 0x3E, 0x08, 0x08,
            0x14, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
        
        # 語 (漢字)
        ('0x8A9E', '語', [
            0x00, 0x00, 0x3E, 0x24, 0x24, 0x3E, 0x24, 0x24,
            0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]),
    ]
    
    # ヘッダーファイル生成
    header_content = '''// カスタム日本語フォント - デバッグ版
// 手動作成の安全なフォントデータ
// 8x16ピクセル ビットマップフォント
#pragma once
#include <stdint.h>
#include <Arduino.h>  // PROGMEM定義のため

// フォント設定
#define CUSTOM_FONT_WIDTH  8
#define CUSTOM_FONT_HEIGHT 16
#define CUSTOM_FONT_COUNT  ''' + str(len(simple_fonts)) + '''

// フォントビットマップデータ構造体
struct FontChar {
    uint16_t unicode;      // Unicode文字コード
    uint8_t bitmap[16];    // 8x16ビットマップデータ
};

// シンプルな日本語文字セット（デバッグ用）
const FontChar custom_japanese_font[] PROGMEM = {
'''
    
    # 各文字のビットマップデータを生成
    for i, (unicode_hex, char, bitmap) in enumerate(simple_fonts):
        unicode_val = int(unicode_hex, 16)
        
        header_content += f'    // {char} ({unicode_hex})\n'
        header_content += f'    {{{unicode_hex}, {{\n'
        
        # 16バイトのビットマップデータを4行に分けて出力
        for row in range(0, 16, 4):
            header_content += '        '
            for col in range(4):
                if row + col < 16:
                    header_content += f'0x{bitmap[row + col]:02X}'
                    if row + col < 15:
                        header_content += ', '
            header_content += '\n'
        
        header_content += '    }}'
        if i < len(simple_fonts) - 1:
            header_content += ','
        header_content += '\n\n'
    
    header_content += '''};

const uint16_t custom_japanese_font_count = sizeof(custom_japanese_font) / sizeof(FontChar);

// フォント検索・描画関数の宣言
const FontChar* findFontChar(uint16_t unicode);
void drawCustomChar(int x, int y, uint16_t unicode, uint16_t color);
void drawCustomString(int x, int y, const char* str, uint16_t color);
void drawFallbackString(int x, int y, const char* str, uint16_t color);
void displayFontStats();

// UTF-8解析関数
uint16_t utf8ToUnicode(const char* utf8, int& byteCount);
'''
    
    return header_content

if __name__ == "__main__":
    print("シンプルなデバッグ用日本語フォント生成中...")
    
    # フォントヘッダー生成
    header_content = generate_simple_font_header()
    
    # ファイルに保存
    with open('../src/custom_japanese_font_debug.h', 'w', encoding='utf-8') as f:
        f.write(header_content)
    
    print("デバッグ用フォントヘッダーファイルを生成しました: src/custom_japanese_font_debug.h")
    print("15文字の手動作成フォントでテスト可能です")
