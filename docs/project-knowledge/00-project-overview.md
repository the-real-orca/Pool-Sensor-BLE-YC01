# Project Overview

> Purpose: fast onboarding for new developers
> Status: draft generated from static inventory; verify and complete against source code.

## Identity

- Project root: `/workspace`
- Files analyzed: 92 text files / 123 total files
- Probable source roots: src, lib

## Technology Snapshot

| Language | Files |
| --- | --- |
| markdown | 42 |
| json | 15 |
| python | 12 |
| c-cpp | 7 |
| toml | 4 |
| cpp | 4 |
| html | 3 |
| yaml | 2 |
| css | 1 |
## Embedded / Firmware Snapshot

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
## Existing Documentation

| Path | Lines |
| --- | --- |
| GEMINI.md | 43 |
| README.md | 28 |
| .gemini/extensions/codebase-documentation-analyzer/GEMINI.md | 8 |
| .gemini/extensions/codebase-documentation-analyzer/README.md | 23 |
| docs/Analysis.md | 30 |
| docs/Coding Guidelines.md | 57 |
| docs/Functional Specification - Pool Sensor BLE-YC01.md | 184 |
| docs/README.md | 22 |
| docs/Technical Specification Document - Pool Sensor BLE-YC01.md | 256 |
| docs/examlpes/FSD layout description with examples.md | 309 |
| docs/examlpes/FSD layout description.md | 129 |
| docs/project-knowledge/00-project-overview.md | 119 |
| docs/project-knowledge/01-functional-specification.md | 31 |
| docs/project-knowledge/02-technical-system-design.md | 135 |
| docs/project-knowledge/03-configuration-reference.md | 158 |
| docs/project-knowledge/04-interface-api-reference.md | 51 |
| docs/project-knowledge/05-runtime-states-and-workflows.md | 322 |
| docs/project-knowledge/06-data-model-and-persistence.md | 21 |
| docs/project-knowledge/07-operations-and-deployment.md | 132 |
| docs/project-knowledge/08-testing-and-quality-map.md | 17 |
| docs/project-knowledge/09-extension-refactoring-backlog.md | 21 |
| docs/project-knowledge/10-embedded-platforms.md | 189 |
| docs/project-knowledge/11-gemini-cli-agent-setup.md | 68 |
| docs/project-knowledge/ai-agent-knowledge-base.md | 46 |
| docs/project-knowledge/README.md | 23 |
| docs/project-knowledge/traceability-matrix.md | 11 |
| docs/project-knowledge/_analysis/inventory.md | 436 |
| docs/Workbench/REST_API.md | 1716 |
| docs/Workbench/Test Dokumentation.md | 658 |
| docs/Workbench/testing-skills.md | 280 |
| docs/Workbench/User Manual.md | 442 |
## Gemini CLI / Agent Tooling Snapshot

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
## Onboarding Map

- How to run locally: TODO
- Main entrypoints: TODO
- Primary domain concepts: TODO
- First files to read: TODO

