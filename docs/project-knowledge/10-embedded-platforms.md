# Embedded and Firmware Platform Notes

> Purpose: document embedded targets, boards, hardware interfaces, firmware lifecycle, and verification gaps
> Status: draft generated from static inventory; verify and complete against source code.

## Detected Platform Evidence

| Platform | Evidence type | Source | Detail |
| --- | --- | --- | --- |
| arduino | platformio_framework | platformio.ini | framework=arduino |
| arduino | source_marker | src/BLE-YC01.cpp | Arduino setup/loop or Arduino.h |
| arduino | source_marker | src/BLE-YC01.h | Arduino setup/loop or Arduino.h |
| arduino | source_marker | src/captivePortal.cpp | Arduino setup/loop or Arduino.h |
| arduino | source_marker | src/config.h | Arduino setup/loop or Arduino.h |
| arduino | source_marker | src/main.cpp | Arduino setup/loop or Arduino.h |
| arduino | source_marker | src/webUtils.cpp | Arduino setup/loop or Arduino.h |
| esp | source_marker | docker_setup.md | ESP-family API/header marker |
| esp | source_marker | README.md | ESP-family API/header marker |
| esp | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | ESP-family API/header marker |
| esp | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | ESP-family API/header marker |
| esp | source_marker | .gemini/skills/pool-sensor-guidelines/SKILL.md | ESP-family API/header marker |
| esp | source_marker | docs/Analysis.md | ESP-family API/header marker |
| esp | source_marker | docs/Coding Guidelines.md | ESP-family API/header marker |
| esp | source_marker | docs/Functional Specification - Pool Sensor BLE-YC01.md | ESP-family API/header marker |
| esp | source_marker | docs/Technical Specification Document - Pool Sensor BLE-YC01.md | ESP-family API/header marker |
| esp | source_marker | docs/examlpes/FSD layout description with examples.md | ESP-family API/header marker |
| esp | source_marker | docs/project-knowledge/05-runtime-states-and-workflows.md | ESP-family API/header marker |
| esp | source_marker | docs/project-knowledge/ai-agent-knowledge-base.md | ESP-family API/header marker |
| esp | source_marker | docs/project-knowledge/_analysis/inventory.md | ESP-family API/header marker |
| esp | source_marker | docs/Workbench/REST_API.md | ESP-family API/header marker |
| esp | source_marker | docs/Workbench/Test Dokumentation.md | ESP-family API/header marker |
| esp | source_marker | docs/Workbench/User Manual.md | ESP-family API/header marker |
| esp | source_marker | pytest/workbench_driver.py | ESP-family API/header marker |
| esp | source_marker | pytest/tests/01_startup_test.py | ESP-family API/header marker |
| esp | source_marker | pytest/tests/06_wifi_portal_test.py | ESP-family API/header marker |
| esp | source_marker | skills/pool-sensor-guidelines/SKILL.md | ESP-family API/header marker |
| esp | source_marker | src/captivePortal.cpp | ESP-family API/header marker |
| esp | source_marker | src/main.cpp | ESP-family API/header marker |
| esp | source_marker | src/update.html.h | ESP-family API/header marker |
| esp | source_marker | src/webUtils.cpp | ESP-family API/header marker |
| esp | source_marker | src/data/config.json | ESP-family API/header marker |
| esp | source_marker | src/data/config_example.json | ESP-family API/header marker |
| esp | source_marker | src/data/update.html | ESP-family API/header marker |
| esp32 | platformio_platform | platformio.ini | platform=espressif32 |
| esphome | yaml_config | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | ESPHome device config |
| esphome | yaml_config | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | ESPHome device config |
| esphome | yaml_config | docs/project-knowledge/05-runtime-states-and-workflows.md | ESPHome device config |
| esphome | yaml_config | docs/project-knowledge/_analysis/inventory.md | ESPHome device config |
| freertos | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | FreeRTOS API marker |
| freertos | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | FreeRTOS API marker |
| freertos | source_marker | docs/project-knowledge/_analysis/inventory.md | FreeRTOS API marker |
| mbed-os | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | Mbed OS API marker |
| mbed-os | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | Mbed OS API marker |
| mbed-os | source_marker | docs/project-knowledge/_analysis/inventory.md | Mbed OS API marker |
| nuttx | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | NuttX marker |
| platformio | manifest | platformio.ini | PlatformIO project configuration |
| platformio | setting | platformio.ini | board=esp32doit-devkit-v1 |
| platformio | setting | platformio.ini | board_build.partitions=min_spiffs.csv |
| platformio | setting | platformio.ini | board_build.filesystem=littlefs |
| platformio | setting | platformio.ini | platform=espressif32 |
| platformio | setting | platformio.ini | framework=arduino |
| platformio | setting | platformio.ini | monitor_speed=115200 |
| platformio | setting | platformio.ini | lib_deps=bblanchon/ArduinoJson @ ^7.4.1 |
| platformio | setting | platformio.ini | build_flags=-D CORE_DEBUG_LEVEL=0 |
| raspberry-pi-pico-sdk | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/analysis-methodology.md | Pico SDK API marker |
| raspberry-pi-pico-sdk | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | Pico SDK API marker |
| raspberry-pi-pico-sdk | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | Pico SDK API marker |
| raspberry-pi-pico-sdk | source_marker | docs/project-knowledge/_analysis/inventory.md | Pico SDK API marker |
| riot-os | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | RIOT OS marker |
| riot-os | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | RIOT OS marker |
| tinygo | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/diagram-guidelines.md | TinyGo machine package marker |
| tinygo | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | TinyGo machine package marker |
| tinygo | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | TinyGo machine package marker |
| tinygo | source_marker | docs/project-knowledge/02-technical-system-design.md | TinyGo machine package marker |
| tinygo | source_marker | docs/project-knowledge/03-configuration-reference.md | TinyGo machine package marker |
| tinygo | source_marker | docs/project-knowledge/05-runtime-states-and-workflows.md | TinyGo machine package marker |
| tinygo | source_marker | docs/project-knowledge/07-operations-and-deployment.md | TinyGo machine package marker |
| tinygo | source_marker | docs/project-knowledge/10-embedded-platforms.md | TinyGo machine package marker |
| tinygo | source_marker | docs/project-knowledge/_analysis/inventory.md | TinyGo machine package marker |
| zephyr | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | Zephyr API/devicetree/thread marker |
| zephyr | source_marker | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | Zephyr API/devicetree/thread marker |
| zephyr | source_marker | docs/project-knowledge/_analysis/inventory.md | Zephyr API/devicetree/thread marker |
| arduino | platform_candidate |  | inferred from embedded project evidence |
| esp/esp-idf | platform_candidate |  | inferred from embedded project evidence |
| espressif32 | platform_candidate |  | inferred from embedded project evidence |
| freertos | platform_candidate |  | inferred from embedded project evidence |
| mbed-os | platform_candidate |  | inferred from embedded project evidence |
| nuttx | platform_candidate |  | inferred from embedded project evidence |
| platformio | platform_candidate |  | inferred from embedded project evidence |
| raspberry-pi-pico-sdk | platform_candidate |  | inferred from embedded project evidence |
| riot-os | platform_candidate |  | inferred from embedded project evidence |
| stm32cube/stm32-hal | platform_candidate |  | inferred from embedded project evidence |
| zephyr | platform_candidate |  | inferred from embedded project evidence |
| hardware-interface-source | embedded_file | docker_setup.md | 107 lines |
| hardware-interface-source | embedded_file | GEMINI.md | 43 lines |
| hardware-interface-source, platformio-project | embedded_file | platformio.ini | 41 lines |
| hardware-interface-source | embedded_file | README.md | 28 lines |
| hardware-interface-source | embedded_file | ROADMAP.md | 50 lines |
| hardware-interface-source | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/SKILL.md | 154 lines |
| esp-idf-source, hardware-interface-source, pico-sdk-project | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/analysis-methodology.md | 106 lines |
| hardware-interface-source | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/diagram-guidelines.md | 157 lines |
| hardware-interface-source | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/documentation-templates.md | 222 lines |
| hardware-interface-source | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/gemini-cli-usage.md | 158 lines |
| arduino-source, esp-idf-source, freertos-project, hardware-interface-source, mbed-os-project, pico-sdk-project, riot-os-project, zephyr-project | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | 140 lines |
| arduino-source, esp-idf-source, freertos-project, hardware-interface-source, mbed-os-project, nuttx-project, pico-sdk-project, riot-os-project, stm32cube-hal-project, zephyr-project | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py | 1487 lines |
| hardware-interface-source | embedded_file | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/write_doc_scaffold.py | 534 lines |
| hardware-interface-source | embedded_file | .gemini/skills/pool-sensor-guidelines/SKILL.md | 44 lines |
| hardware-interface-source | embedded_file | docs/Analysis.md | 30 lines |
| hardware-interface-source | embedded_file | docs/Coding Guidelines.md | 57 lines |
| hardware-interface-source | embedded_file | docs/Functional Specification - Pool Sensor BLE-YC01.md | 184 lines |
| hardware-interface-source | embedded_file | docs/Technical Specification Document - Pool Sensor BLE-YC01.md | 256 lines |
| hardware-interface-source | embedded_file | docs/examlpes/FSD layout description with examples.md | 309 lines |
| hardware-interface-source | embedded_file | docs/examlpes/FSD layout description.md | 129 lines |
| arduino-source, hardware-interface-source | embedded_file | docs/project-knowledge/00-project-overview.md | 119 lines |
| arduino-source, hardware-interface-source | embedded_file | docs/project-knowledge/02-technical-system-design.md | 135 lines |
| arduino-source, hardware-interface-source | embedded_file | docs/project-knowledge/03-configuration-reference.md | 158 lines |
| esp-idf-source, hardware-interface-source | embedded_file | docs/project-knowledge/05-runtime-states-and-workflows.md | 322 lines |
| arduino-source, hardware-interface-source | embedded_file | docs/project-knowledge/07-operations-and-deployment.md | 132 lines |
| arduino-source, hardware-interface-source | embedded_file | docs/project-knowledge/10-embedded-platforms.md | 189 lines |
| hardware-interface-source | embedded_file | docs/project-knowledge/ai-agent-knowledge-base.md | 46 lines |
| arduino-source, esp-idf-source, freertos-project, hardware-interface-source, mbed-os-project, pico-sdk-project | embedded_file | docs/project-knowledge/_analysis/inventory.md | 436 lines |
| hardware-interface-source | embedded_file | docs/Workbench/REST_API.md | 1716 lines |
| hardware-interface-source | embedded_file | docs/Workbench/Test Dokumentation.md | 658 lines |
| hardware-interface-source | embedded_file | docs/Workbench/testing-skills.md | 280 lines |
| hardware-interface-source | embedded_file | docs/Workbench/User Manual.md | 442 lines |
| hardware-interface-source | embedded_file | pytest/conftest.py | 92 lines |
| hardware-interface-source | embedded_file | pytest/workbench_driver.py | 687 lines |
| hardware-interface-source | embedded_file | pytest/tests/06_wifi_portal_test.py | 218 lines |
| hardware-interface-source | embedded_file | pytest/tests/07_mqtt_test.py | 187 lines |
| hardware-interface-source | embedded_file | skills/pool-sensor-guidelines/SKILL.md | 44 lines |
| arduino-source, hardware-interface-source | embedded_file | src/BLE-YC01.cpp | 225 lines |
| arduino-source | embedded_file | src/BLE-YC01.h | 112 lines |
| arduino-source, hardware-interface-source | embedded_file | src/captivePortal.cpp | 136 lines |
| arduino-source, hardware-interface-source | embedded_file | src/config.h | 99 lines |
| arduino-source, hardware-interface-source | embedded_file | src/main.cpp | 643 lines |
| arduino-source, hardware-interface-source | embedded_file | src/webUtils.cpp | 402 lines |
| hardware-interface-source | embedded_file | src/webUtils.h | 36 lines |
| platformio | environment | platformio.ini | env=esp32doit-devkit-v1, platform=espressif32, board=esp32doit-devkit-v1, framework=arduino |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/analysis-methodology.md:24 | pico-sdk-init |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md:110 | timer-callback |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md:98 | pico-sdk-init |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md:100 | mbed-main |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py:188 | timer-callback |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py:217 | timer-callback |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py:277 | mbed-main |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py:988 | mbed-main |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py:279 | micropython-entry |
| firmware | firmware_entrypoint | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/scripts/project_inventory.py:990 | micropython-entry |
| firmware | firmware_entrypoint | docs/project-knowledge/04-interface-api-reference.md:45 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:117 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:120 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:121 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:128 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:129 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:130 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/10-embedded-platforms.md:131 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:168 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:171 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:172 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:173 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:174 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:175 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:176 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:183 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:184 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:185 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:186 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:187 | timer-callback |
| firmware | firmware_entrypoint | docs/project-knowledge/_analysis/inventory.md:188 | timer-callback |
## Build Targets / Profiles

Document PlatformIO environments, Arduino FQBNs, ESP-IDF targets, Zephyr board targets/overlays, STM32Cube `.ioc` settings, FreeRTOS config, Pico SDK targets, Nordic nRF Connect SDK/west settings, Mbed targets, MicroPython/CircuitPython entry files, board/chip family, upload and monitor settings, debug configuration, and CI build matrix.

## Hardware Interfaces

Document GPIO pins, EXTI/interrupts, ADC/DAC, PWM/timers, I2C, SPI/QSPI, UART/Serial, USB, CAN/TWAI, LIN, RS-485, BLE, Wi-Fi, Ethernet, LoRa/LoRaWAN, cellular modem, MQTT/HTTP/CoAP, OTA, sensors, actuators, displays, relays, motors, LEDs, buttons, and storage peripherals.

## Firmware Lifecycle

Document boot, clock/peripheral initialization, setup/main/app entry, main loop or RTOS tasks/threads, interrupts/callbacks, connectivity, provisioning, OTA/rollback, watchdog/brownout/reset, error handling, safe mode, calibration, and sleep/wake behavior.

## Memory / Storage / Partitions

Document flash size, partition table, linker scripts, memory map, NVS/preferences, EEPROM/emulated EEPROM, SPIFFS/LittleFS/FATFS, SD card files, retained RTC memory, external FRAM/EEPROM, and calibration constants.

## Verification Gaps

_To be completed from source analysis._
