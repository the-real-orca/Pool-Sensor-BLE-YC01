# Configuration Reference

> Purpose: document every internal and external parameter without exposing secrets
> Status: draft generated from static inventory; verify and complete against source code.

## Rules

Do not publish real secret values. Document names, sources, expected format, default behavior, and operational impact.

## Environment Parameters Defined in Files

_To be completed from source analysis._
## Environment Parameters Used in Code

_To be completed from source analysis._
## Config Files and Key Paths

| File | Key sample |
| --- | --- |
| .geminiignore |  |
| GEMINI.md |  |
| platformio.ini | platformio, platformio.data_dir, platformio.default_envs, platformio.build_cache_dir, env:esp32doit-devkit-v1, env:esp32doit-devkit-v1.board, env:esp32doit-devkit-v1.board_build.partitions, env:esp32doit-devkit-v1.board_build.filesystem, env:esp32doit-devkit-v1.platform, env:esp32doit-devkit-v1.framework, env:esp32doit-devkit-v1.upload_port, env:esp32doit-devkit-v1.monitor_port, env:esp32doit-devkit-v1.monitor_speed, env:esp32doit-devkit-v1.lib_deps, env:esp32doit-devkit-v1.build_flags |
| .gemini/settings.json | security, security.auth, security.auth.selectedType, general, general.defaultApprovalMode, context, context.fileFiltering, context.fileFiltering.respectGitIgnore, context.systemInstruction, model, model.name, ui, ui.footer, ui.footer.items, hooks, hooks.BeforeTool, hooks.BeforeTool[0].matcher, hooks.BeforeTool[0].hooks, hooks.BeforeTool[0].hooks[0].type, hooks.BeforeTool[0].hooks[0].command |
| .gemini/extensions/codebase-documentation-analyzer/gemini-extension.json | name, version, description, contextFileName |
| .gemini/extensions/codebase-documentation-analyzer/GEMINI.md |  |
| .gemini/skills/pool-sensor-guidelines/SKILL.md |  |
| src/data/config.json | interval, name, addr, wifiSSID, wifiPassword, wifiConnectionTimeout, portalSSID, portalPassword, portalTimeout, mqttServer, mqttPort, mqttTLS, mqttTopic, mqttUser, mqttPassword |
| src/data/config_example.json | interval, name, bleAddress, wifiSSID, wifiPassword, wifiTimeout, portalSSID, portalPassword, portalTimeout, mqttServer, mqttPort, mqttTLS, mqttTopic, mqttUser, mqttPassword |
## Embedded / Firmware Configuration Evidence

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
## External Service Candidates

| Service | Evidence count |
| --- | --- |
| github | 4 |
## Gemini CLI / Agent Configuration

| Kind | Path | Detail |
| --- | --- | --- |
| ignore | .geminiignore | Gemini file access ignore rules |
| context | GEMINI.md | Gemini context file |
| settings | .gemini/settings.json | Gemini settings |
| extension_manifest | .gemini/extensions/codebase-documentation-analyzer/gemini-extension.json | name=codebase-documentation-analyzer; version=1.0.0; context=GEMINI.md |
| context | .gemini/extensions/codebase-documentation-analyzer/GEMINI.md | Gemini context file |
| custom_command | .gemini/extensions/codebase-documentation-analyzer/commands/docs/analyze-codebase.toml | command=/docs:analyze-codebase |
| custom_command | .gemini/extensions/codebase-documentation-analyzer/commands/docs/gap-review.toml | command=/docs:gap-review |
| custom_command | .gemini/extensions/codebase-documentation-analyzer/commands/docs/update-codebase-docs.toml | command=/docs:update-codebase-docs |
| agent_skill | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/SKILL.md | skill=codebase-documentation-analyzer |
| agent_skill | .gemini/skills/pool-sensor-guidelines/SKILL.md | skill=pool-sensor-guidelines |
| agent_skill | skills/pool-sensor-guidelines/SKILL.md | skill=pool-sensor-guidelines |
Document `GEMINI.md`, `.gemini/settings.json`, MCP server configuration, disabled/enabled skills, context file names, hooks, and `.geminiignore` effects when present. Redact sensitive setting values.

## Configuration Matrix

_To be completed from source analysis._
