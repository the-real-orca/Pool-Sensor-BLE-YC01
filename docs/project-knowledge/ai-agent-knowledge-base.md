# AI Agent Knowledge Base

> Purpose: compact, source-grounded facts for future AI agents
> Status: draft generated from static inventory; verify and complete against source code.

## Project Facts

- Project name: TODO
- Primary language/framework: TODO
- Main runtime: TODO
- Main entrypoint: TODO
- Data stores: TODO
- External services: TODO

## Embedded / Firmware Facts

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
## Source Map

- Start here: TODO
- Domain logic: TODO
- API layer: TODO
- Config: TODO
- Tests: TODO

## High-Confidence Facts

Use bullet points with source file references.

## Open Questions

TODO
