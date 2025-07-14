// カスタム日本語フォント描画実装
#include "custom_japanese_font.h"
#include <M5Unified.h>
#include <string.h>

// フォント文字検索
const FontChar* findFontChar(uint16_t unicode) {
    for (uint16_t i = 0; i < custom_japanese_font_count; i++) {
        if (pgm_read_word(&custom_japanese_font[i].unicode) == unicode) {
            return &custom_japanese_font[i];
        }
    }
    return nullptr; // 文字が見つからない場合
}

// カスタム文字描画
void drawCustomChar(int x, int y, uint16_t unicode, uint16_t color) {
    const FontChar* fontChar = findFontChar(unicode);
    if (!fontChar) {
        // 文字が見つからない場合は四角を描画
        M5.Lcd.drawRect(x, y, CUSTOM_FONT_WIDTH, CUSTOM_FONT_HEIGHT, color);
        return;
    }
    
    // ビットマップデータを描画
    for (int row = 0; row < CUSTOM_FONT_HEIGHT; row++) {
        uint8_t line = pgm_read_byte(&fontChar->bitmap[row]);
        for (int col = 0; col < CUSTOM_FONT_WIDTH; col++) {
            if (line & (0x80 >> col)) {
                M5.Lcd.drawPixel(x + col, y + row, color);
            }
        }
    }
}

// UTF-8文字列を解析してUnicodeコードポイントを取得
uint16_t utf8ToUnicode(const char* utf8, int& byteCount) {
    uint8_t first = utf8[0];
    byteCount = 1;
    
    if (first < 0x80) {
        // ASCII文字 (1バイト)
        return first;
    } else if ((first & 0xE0) == 0xC0) {
        // 2バイト文字
        byteCount = 2;
        return ((first & 0x1F) << 6) | (utf8[1] & 0x3F);
    } else if ((first & 0xF0) == 0xE0) {
        // 3バイト文字（日本語のほとんど）
        byteCount = 3;
        return ((first & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
    } else if ((first & 0xF8) == 0xF0) {
        // 4バイト文字
        byteCount = 4;
        return ((first & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | 
               ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
    }
    
    // 不正なUTF-8
    return 0;
}

// カスタム文字列描画
void drawCustomString(int x, int y, const char* str, uint16_t color) {
    int currentX = x;
    int currentY = y;
    int strLen = strlen(str);
    int i = 0;
    
    while (i < strLen) {
        int byteCount;
        uint16_t unicode = utf8ToUnicode(&str[i], byteCount);
        
        if (unicode == 0) {
            i++;
            continue; // 不正な文字をスキップ
        }
        
        // 改行処理
        if (unicode == 0x000A) { // LF
            currentX = x;
            currentY += CUSTOM_FONT_HEIGHT + 2;
            i += byteCount;
            continue;
        }
        
        // 画面幅チェック（自動改行）
        if (currentX + CUSTOM_FONT_WIDTH > M5.Lcd.width()) {
            currentX = x;
            currentY += CUSTOM_FONT_HEIGHT + 2;
        }
        
        // 文字描画
        drawCustomChar(currentX, currentY, unicode, color);
        
        currentX += CUSTOM_FONT_WIDTH + 1; // 文字間隔
        i += byteCount;
    }
}

// デフォルトフォント代替描画（フォールバック用）
void drawFallbackString(int x, int y, const char* str, uint16_t color) {
    M5.Lcd.setTextColor(color);
    M5.Lcd.setCursor(x, y);
    M5.Lcd.setTextSize(1);
    M5.Lcd.print(str);
}

// 混合モード描画（カスタムフォント＋フォールバック）
void drawMixedString(int x, int y, const char* str, uint16_t color) {
    int currentX = x;
    int currentY = y;
    int strLen = strlen(str);
    int i = 0;
    String fallbackBuffer = "";
    
    while (i < strLen) {
        int byteCount;
        uint16_t unicode = utf8ToUnicode(&str[i], byteCount);
        
        // カスタムフォントで対応可能な文字かチェック
        const FontChar* fontChar = findFontChar(unicode);
        
        if (fontChar) {
            // カスタムフォントで描画可能
            // 蓄積されたフォールバック文字があれば先に描画
            if (fallbackBuffer.length() > 0) {
                M5.Lcd.setTextColor(color);
                M5.Lcd.setCursor(currentX, currentY);
                M5.Lcd.setTextSize(1);
                M5.Lcd.print(fallbackBuffer);
                currentX += fallbackBuffer.length() * 6; // 概算幅
                fallbackBuffer = "";
            }
            
            // カスタム文字描画
            drawCustomChar(currentX, currentY, unicode, color);
            currentX += CUSTOM_FONT_WIDTH + 1;
        } else {
            // フォールバック文字として蓄積
            for (int j = 0; j < byteCount; j++) {
                fallbackBuffer += str[i + j];
            }
        }
        
        i += byteCount;
    }
    
    // 残ったフォールバック文字を描画
    if (fallbackBuffer.length() > 0) {
        M5.Lcd.setTextColor(color);
        M5.Lcd.setCursor(currentX, currentY);
        M5.Lcd.setTextSize(1);
        M5.Lcd.print(fallbackBuffer);
    }
}
