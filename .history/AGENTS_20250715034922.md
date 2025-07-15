# Stack-chan Communication System - Build Environment

## PlatformIO Setup

### Python Virtual Environment

- **Location**: `/Users/limonene/MyProject/01_dev/M5/stack-chan/.venv/`
- **Python Executable**: `/Users/limonene/MyProject/01_dev/M5/stack-chan/.venv/bin/python`
- **PlatformIO Installation**: Installed via `install_python_packages` tool

### Build Commands

```bash
# ビルドコマンド（仮想環境使用）
source .venv/bin/activate && pio run -e m5stack-grey

# M5Stack Basic (Grey) 用環境を使用 - 実機対応 ⭐
# platformio.ini の default_envs = m5stack-grey が適用される
```

### Platform Configuration ⭐

- **Target Environment**: `m5stack-grey` (M5Stack Basic 16MB Flash) **← 実機仕様**
- **Platform**: ESP32 (espressif32 @ 6.5.0)
- **Framework**: Arduino
- **Board**: M5Stack Grey (ESP32 240MHz, 520KB RAM, 16MB Flash)
- **Hardware**: M5Stack Basic (Grey) - 実機環境

### Build Results (Latest Success) ⭐

- **Flash Usage**: 35.2% (2,308,573 / 6,553,600 bytes)
- **RAM Usage**: 11.5% (61,388 / 532,480 bytes)
- **Build Time**: 25.07 seconds
- **Status**: SUCCESS ✅ **M5Stack BASIC対応完了**

## Font System - Japanese Support

### **CRITICAL: Japanese Font Display Solution** ⭐

**Problem**: Japanese text appears as squares (□) on M5Stack display

**Solution** (verified working):

```cpp
#include "M5Unified.h"

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // CORRECT API: Use M5.Lcd.setTextFont() NOT M5.Display.setFont()
  M5.Lcd.setTextFont(&fonts::efontJA_16);
  M5.Display.setTextSize(1);
  M5.Lcd.println("こんにちは");  // Will display correctly
}
```

**Key Points**:

- ✅ **CORRECT**: `M5.Lcd.setTextFont(&fonts::efontJA_16)`
- ❌ **WRONG**: `M5.Display.setFont(&fonts::efontJA_16)` (causes squares)
- Available fonts: `fonts::efontJA_16`, `fonts::efontJA_24`
- Fallback: `fonts::Font0` for English text
- **Font Size**: Use `M5.Display.setTextSize(0.5)` for smaller text

### **CRITICAL: StackchanSystemConfig Dependency Issue** ⭐

**Problem**: FreeRTOS `xQueueGenericSend assertion failed` crashes during Avatar operations

**Root Cause**: Complex StackchanSystemConfig dependency chain causing task conflicts:

- SD card file reading
- YAML parsing
- File system access competing with Avatar FreeRTOS tasks

**Solution** (Codex-suggested):

```cpp
// Remove StackchanSystemConfig dependency
// #include <Stackchan_system_config.h>  // Remove this

// Use simple default configuration instead
comm_config.webserver_port = 80;
comm_config.bluetooth_device_name = "M5Stack-StackChan";
comm_config.bluetooth_starting_state = true;
comm_config.lyrics = {"こんにちは", "元気です", "よろしく"};
```

**Benefits**:

- Eliminates SD card dependency conflicts
- Reduces memory usage and complexity
- Improves Avatar stability
- Faster boot time

**Trade-off**: Complete StackchanSystemConfig removal may disable Avatar display functionality.

### **BALANCED: Avatar Display Recovery** ⭐

**Problem**: Removing StackchanSystemConfig eliminates Avatar visual display entirely

**Balanced Solution**:

```cpp
// Avatar initialization with minimal tasks for stability
avatar.init();
avatar.setColorPalette(*cps[0]);
avatar.setSpeechFont(&fonts::efontJA_16);
avatar.setExpression(Expression::Neutral);

// Add only essential task for display
avatar.addTask(face, "face");      // Required for visual display
// avatar.addTask(lipSync, "lipSync");  // Skip for stability

// Reduce animation frequency
static unsigned long last_animation = 0;
if (avatar_initialized && (millis() - last_animation) > 10000) {
  avatar.setMouthOpenRatio(0.2);  // Minimal movement
  delay(100);
  avatar.setMouthOpenRatio(0.0);
  last_animation = millis();
}
```

**Result**: Maintains Avatar visual display while preserving system stability

### Current Implementation

- **Font Library**: M5GFX internal fonts
- **Japanese Font**: `fonts::efontJA_16` (内蔵日本語フォント)
- **API Usage**: `M5.Display.setFont(&fonts::efontJA_16)`

### Resolved Issues

1. **Custom Font Problems**:
   - Removed custom bitmap font implementation (caused green lines and garbled display)
   - Migrated from custom UTF-8 parsing to M5GFX internal font system

2. **Build Dependencies**:
   - Cleaned up orphaned custom font files:
     - `custom_japanese_font.cpp` → `.bak2`
     - `custom_japanese_font.h` → `.bak`
     - `custom_japanese_font_debug.h` → `.bak`

3. **Font Initialization Hang Issues** (2025年7月15日):
   - **Problem**: System hanging at "日本語フォント初期化中" with green vertical lines
   - **Root Cause**: M5GFX efontJA_16 internal font causing initialization deadlock
   - **Solution**: Replaced all Japanese font calls with basic Font0 (English font)
   - **Status**: Stable build achieved, system no longer hangs during startup

### Font Files Status

- **Active**: Internal M5GFX fonts only
- **Backup**: All custom font implementations moved to `.bak` files
- **Dependencies**: No external font file dependencies

## Libraries & Dependencies

### Core Libraries

- **M5GFX**: 0.2.9 (graphics and font rendering)
- **M5Unified**: 0.2.7 (M5Stack hardware abstraction)
- **M5Stack-Avatar**: 0.10.0 (avatar display)
- **stackchan-arduino**: 0.0.4 (Stack-chan core functionality)

### Communication Libraries

- **ArduinoJson**: 7.4.2 (JSON handling)
- **BluetoothSerial**: 2.0.0 (Bluetooth communication)
- **WiFi**: 2.0.0 (WiFi connectivity)
- **WebServer**: 2.0.0 (HTTP server)

## Troubleshooting Notes

### Common Build Issues

1. **PlatformIO Not Found**: Install via Python virtual environment
2. **Wrong Target Board**: Use `m5stack-grey` not `m5stack-core2` for Basic
3. **Font Display Issues**: Use internal M5GFX fonts, avoid custom implementations
4. **Dependency Conflicts**: Clean `.pio/libdeps/` if needed

### Warning Handling

- **DynamicJsonDocument Deprecation**: Warnings present but not blocking
- **LGFX_USE_V1 Redefinition**: Expected warnings from M5GFX integration
- **Return Type Warnings**: Non-critical stackchan-arduino library warnings

## Development Environment

- **IDE**: VS Code with PlatformIO IDE extension
- **OS**: macOS
- **Shell**: zsh
- **Build System**: PlatformIO Core in Python virtual environment

## Last Updated

2025年7月15日 - Added M5Unified_StackChan_ChatGPT analysis and extension strategy documentation

## **Extension Strategy & Reference Implementations** ⭐

### **M5Unified_StackChan_ChatGPT Analysis**

**Reference Implementation**: `/Users/limonene/MyProject/01_dev/M5/M5Unified_StackChan_ChatGPT/`

**Key Architecture Features**:

1. **WiFi Connection Strategy**:

   ```cpp
   // Multi-layered connection approach
   WiFi.begin();  // Try previous connection
   WiFi.beginSmartConfig();  // Fallback to SmartConfig
   
   // SD file-based configuration
   SD.open("/wifi.txt", FILE_READ);  // SSID\nPassword format
   SD.open("/apikey.txt", FILE_READ);  // OpenAI\nVoiceText format
   ```

2. **Successful Avatar + FreeRTOS Integration**:

   ```cpp
   avatar.init();
   avatar.addTask(lipSync, "lipSync");
   avatar.addTask(servo, "servo");  // Critical: servo task works
   avatar.setSpeechFont(&fonts::efontJA_16);
   ```

3. **WebServer API Implementation**:

   - Uses **ESP32WebServer** (not WebServer)
   - REST API endpoints: `/chat`, `/role`, `/apikey`
   - HTML forms for configuration
   - SPIFFS for persistent settings

**Stability Analysis**:

- ✅ **Avatar with tasks works**: lipSync + servo tasks successful
- ✅ **WiFi + WebServer**: Stable concurrent operation
- ✅ **Audio + TTS**: VoiceText API integration
- ✅ **ChatGPT API**: Full conversational AI

### **Current Implementation vs Reference**

| Component | Current (stack-chan) | Reference (ChatGPT) | Status |
|-----------|---------------------|-------------------|---------|
| Avatar Base | ✅ Works | ✅ Works | Same |
| Avatar Tasks | ❌ **FreeRTOS crash** | ✅ **lipSync+servo** | **Critical difference** |
| WiFi Strategy | ✅ Multi-network | ✅ SmartConfig+SD | Different approach |
| WebServer | ✅ WebServer class | ✅ ESP32WebServer | Library difference |
| Japanese Font | ✅ efontJA_16 | ✅ efontJA_16 | Same |
| SD Configuration | ✅ StackchanSystemConfig | ✅ Simple txt files | Different complexity |

### **Critical Finding: Avatar Task Success Pattern**

**Reference implementation successfully uses**:

```cpp
avatar.addTask(lipSync, "lipSync");
avatar.addTask(servo, "servo");
```

**Key Differences**:

1. **servo task instead of face task**
2. **Different task implementation** in reference
3. **Successful FreeRTOS task management**

### **🚨 Communication Edition Design Principles** ⭐

**Core Philosophy**: Avatar display + communication focus, **NO servo control**

**What to Include**:
- ✅ Avatar visual display (init, expressions, speech text)
- ✅ Communication systems (WiFi, Bluetooth, WebServer)
- ✅ Japanese font support
- ✅ Memory optimization techniques from servo implementation
- ✅ FreeRTOS task management patterns

**What to EXCLUDE**:
- ❌ **Servo motor control** (ServoEasing, servo tasks)
- ❌ **Physical movement** (neck, head rotation)
- ❌ **Hardware servo libraries**
- ❌ **Servo GPIO configuration**

**Reference Implementation Lessons** (servo-free adaptation):

```cpp
// APPLY: Memory management patterns
preallocateBuffer = (uint8_t *)malloc(preallocateBufferSize);

// APPLY: Task management approach (without servo)
avatar.addTask(lipSync, "lipSync");  // Audio sync only
// avatar.addTask(servo, "servo");   // SKIP: No servo control

// APPLY: Configuration strategies
SD.open("/wifi.txt", FILE_READ);     // Simple config files
SD.open("/apikey.txt", FILE_READ);   // Future AI integration

// APPLY: WebServer patterns
ESP32WebServer server(80);           // Alternative server library
```

**Benefits of Servo-Free Design**:
- Reduced complexity and memory usage
- Fewer FreeRTOS task conflicts
- Focus on communication reliability
- Easy deployment without hardware assembly

### **Extension Roadmap**

**Phase 1: Stability (Current)**
- ✅ StackchanSystemConfig integration
- ✅ Japanese font display
- ⏳ Avatar FreeRTOS stability

**Phase 2: ChatGPT Integration**
- ESP32WebServer migration
- ChatGPT API integration
- VoiceText TTS integration
- Role-based character settings

**Phase 3: Advanced Features**
- Voice recognition input
- Emotional expression system
- Multi-device communication
- SPIFFS configuration storage

**Phase 4: AI Enhancement**
- Conversation history
- Context awareness
- Learning capabilities
- Custom model integration

### **Immediate Action Items**

1. **Investigate servo task implementation** in reference
2. **Compare FreeRTOS task management** patterns
3. **Test ESP32WebServer** as alternative to WebServer
4. **Implement simple SD configuration** files (wifi.txt, apikey.txt)

### **Long-term Vision**

Transform current communication-focused stack-chan into full **conversational AI robot** following the proven ChatGPT implementation pattern while maintaining stability improvements.