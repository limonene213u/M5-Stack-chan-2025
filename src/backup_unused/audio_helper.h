/*
 * Audio Helper for Stack-chan
 * Future audio playback functionality (WAV/MP3)
 */

#ifndef _AUDIO_HELPER_H
#define _AUDIO_HELPER_H

#include <Arduino.h>
#include <SD.h>
#include "communication_config.h"

// 将来の音声再生機能用のクラス
class AudioHelper {
private:
  AudioConfig* config;
  bool initialized;
  uint32_t last_auto_speech;
  
public:
  AudioHelper(AudioConfig* cfg) : config(cfg) {
    initialized = false;
    last_auto_speech = 0;
  }

  bool begin() {
    if (!config->enable) {
      Serial.println("Audio功能が無効です");
      return false;
    }
    
    // 将来的に音声ライブラリの初期化を行う
    // 例: ESP32-AudioI2S, ESP32-A2DP などのライブラリを使用
    
    Serial.println("Audio機能は将来のアップデートで実装予定です");
    Serial.printf("設定: フォーマット=%s, 音量=%d, サンプリングレート=%d\n", 
                  config->format.c_str(), config->volume, config->sample_rate);
    
    initialized = true;
    return true;
  }

  bool playFile(const String& filename) {
    if (!initialized || !config->enable) {
      return false;
    }
    
    String filepath = config->voice_files_path + filename;
    
    // 将来的にファイル再生機能を実装
    Serial.printf("音声ファイル再生予定: %s\n", filepath.c_str());
    
    return true;
  }

  bool playWelcomeMessage() {
    return playFile("welcome.wav");
  }

  bool playNotificationSound() {
    return playFile("notification.wav");
  }

  bool playErrorSound() {
    return playFile("error.wav");
  }

  void checkAutoSpeech() {
    if (!config->auto_speech || !initialized) {
      return;
    }
    
    if (millis() - last_auto_speech > config->speech_interval) {
      // 自動音声再生（ひとりごと機能）
      playFile("monologue.wav");
      last_auto_speech = millis();
    }
  }

  void setVolume(int volume) {
    if (volume >= 0 && volume <= 100) {
      config->volume = volume;
      // 実際の音量調整処理を将来実装
      Serial.printf("音量設定: %d\n", volume);
    }
  }

  bool isEnabled() const {
    return config->enable && initialized;
  }
  
  // 推奨音声ファイル一覧
  std::vector<String> getRecommendedFiles() {
    return {
      "welcome.wav",      // 起動時の挨拶
      "notification.wav", // 通知音
      "error.wav",        // エラー音
      "monologue.wav",    // ひとりごと
      "timer.wav",        // タイマー音
      "hello.wav",        // こんにちは
      "goodbye.wav",      // さようなら
      "thankyou.wav",     // ありがとう
      "ok.wav",           // はい
      "no.wav"            // いえ
    };
  }
  
  void printAudioInfo() {
    Serial.println("=== Audio設定情報 ===");
    Serial.printf("有効: %s\n", config->enable ? "はい" : "いいえ");
    Serial.printf("音量: %d\n", config->volume);
    Serial.printf("フォーマット: %s\n", config->format.c_str());
    Serial.printf("サンプリングレート: %d Hz\n", config->sample_rate);
    Serial.printf("音声ファイルパス: %s\n", config->voice_files_path.c_str());
    Serial.printf("自動音声: %s\n", config->auto_speech ? "有効" : "無効");
    if (config->auto_speech) {
      Serial.printf("自動音声間隔: %d秒\n", config->speech_interval / 1000);
    }
    Serial.println("==================");
  }
};

#endif
