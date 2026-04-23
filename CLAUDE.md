# CLAUDE.md - esp32-utilities

This is the core library for the [Celestial Wayfinder](https://github.com/Blake-Ballew/Celestial-Wayfinder) project — a hardware navigation/communication device built on ESP32. It provides high-level managers, UI primitives, LED control, LoRa mesh networking, GPS navigation, and more.

## Build System

- **Framework:** PlatformIO with Arduino framework
- **Target:** ESP32 (espressif32, esp32dev board)
- **Build:** `pio run`
- **Clean build:** `pio run --target clean`
- **Library dependency mode:** `deep` (resolves transitive dependencies)

Include paths are explicitly set in `platformio.ini` build_flags — all `include/` subdirectories are on the path.

## Project Structure

```
esp32-utilities/
├── include/                    # Public headers (mirrors src/)
│   ├── Interfaces/             # Abstract interfaces (LED_Pattern_Interface, DrawCommandInterface, etc.)
│   ├── ModuleManagers/         # Top-level system managers
│   ├── HelperClasses/
│   │   ├── DrawCommands/       # UI drawing primitives
│   │   ├── LED_Patterns/       # WS2812B animation patterns
│   │   ├── Message_Types/      # LoRa message type definitions
│   │   ├── Network/            # Network communication classes
│   │   ├── OLED_Content/       # Screen content renderers
│   │   ├── OLED_Window/        # Window management classes
│   │   ├── Rpc/                # Remote procedure call framework
│   │   └── Window_States/      # State machine definitions
│   └── Utilities/              # Utility headers
└── src/                        # Implementations (mirrors include/)
```

## Architecture

### Layered Design

```
Application (Celestial Wayfinder)
    ↓
ModuleManagers (Display, LED, LoRa, GPS, Settings, Filesystem, EspNow, Rpc)
    ↓
HelperClasses (Windows, States, Content, Patterns, Messages)
    ↓
Utilities + Interfaces
```

### Module Managers (Singletons)

All managers use static methods — no instance needed.

| Manager | Responsibility |
|---|---|
| `Display_Manager` | OLED orchestration, window/state stack, input routing via FreeRTOS queue |
| `LED_Manager` | WS2812B animation, compass ring rendering, pattern registration |
| `Settings_Manager` | JSON settings persistence (SPIFFS) |
| `LoraManager` | LoRa mesh networking, MessagePack serialization |
| `NavigationManager` | GPS + compass integration, heading/distance calculation |
| `FilesystemManager` | SPIFFS file I/O |
| `EspNowManager` | ESP-NOW protocol |
| `RpcManager` | Remote procedure call infrastructure |

### Key Patterns

- **Singleton:** All managers use static class members (FreeRTOS-safe)
- **State Machine:** `Window_State` + `OLED_Window` implement a state stack for display navigation
- **Queue-Driven:** Display commands flow through FreeRTOS queues; inputs mapped to callbacks via `std::map<uint32_t, callbackPointer>`
- **Interface/Plugin:** `LED_Pattern_Interface` and `DrawCommandInterface` allow registering new behaviors without modifying managers
- **Factory:** `MessageBase::MessageFactory` creates polymorphic LoRa messages
- **Template Method:** `OLED_Content` defines virtual `render()`/`update()` lifecycle

## Coding Conventions

- **Classes:** PascalCase with underscores for multi-word (`LED_Manager`, `OLED_Window`, `Window_State`)
- **Constants/Defines:** `UPPER_CASE` (`DEBOUNCE_DELAY`, `NUM_COMPASS_LEDS`, `BUTTON_1_PIN`)
- **Member variables:** camelCase; static members for class-level state
- **Logging:** `ESP_LOG` macros (ESP-IDF style) — not `Serial.print`
- **C++ standard:** C++17 — use `std::unordered_map`, `std::vector`, `std::string`, lambdas, move semantics
- **JSON:** `ArduinoJson` v6 (`StaticJsonDocument` / `DynamicJsonDocument`)
- **Serialization:** MessagePack via ArduinoJson for LoRa messages

## Hardware

Two hardware versions are supported via conditional compilation:

| Define | v1 | v2 |
|---|---|---|
| Display | 128×32 SSD1306 | 128×64 SSD1306 |
| LEDs | 30 WS2812B | 31 WS2812B |
| Compass ring | 16 LEDs | 16 LEDs |

Other hardware: 8 debounced buttons, UART GPS, SPI LoRa, I2C compass (HMC5883 or QMC5883L), SPIFFS, EEPROM.

Relevant compile-time flags:
- `HARDWARE_VERSION`
- `USE_FAKE_GPS_COORDS`

## Key Dependencies

| Library | Purpose |
|---|---|
| FastLED 3.6.0 | WS2812B LED control |
| Adafruit SSD1306 + GFX | OLED display |
| ArduinoJson 6.x | Settings, MessagePack serialization |
| TinyGPSPlus | GPS NMEA parsing |
| NimBLE-Arduino | Bluetooth LE |
| ESPAsyncWebServer | WiFi web interface |
| tinyfsm | State machine support |
| RadioHead (custom fork) | LoRa radio driver |
| ESP32Encoder | Rotary encoder input |
| HMC5883 / QMC5883LCompass | Magnetometer/compass |

## Adding New Features

### New LED Pattern
1. Create header in `include/HelperClasses/LED_Patterns/` inheriting `LED_Pattern_Interface`
2. Implement in `src/HelperClasses/LED_Patterns/`
3. Register with `LED_Manager`

### New Window
1. Create header in `include/HelperClasses/OLED_Window/` inheriting `OLED_Window`
2. Implement states in `include/HelperClasses/Window_States/`
3. Register window with `Display_Manager`

### New LoRa Message Type
1. Inherit from `MessageBase` in `include/HelperClasses/Message_Types/`
2. Implement MessagePack serialization/deserialization
3. Register with `MessageBase::MessageFactory`

### New Draw Command
1. Inherit from `DrawCommandInterface` in `include/HelperClasses/DrawCommands/`
2. Implement `draw()` method

## Related Repository

The [Celestial Wayfinder](https://github.com/Blake-Ballew/Celestial-Wayfinder) repo contains the application layer that consumes this library. Changes to manager APIs or message formats here may require corresponding updates there.
