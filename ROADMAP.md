# Roadmap - Pool Sensor BLE-YC01

This document consolidates the project's tasks and roadmap, providing a prioritized view of planned development and maintenance efforts.

## Phase 1: Documentation, Stability & Core Refinements (High Priority)
- [x] **Function Documentation:** Complete headers and descriptions for all functions to improve maintainability.
        -> documentation generated, see /docs
- [x] **Flash Size Optimization:** Address the high flash usage (currently >99%) by adjusting build flags and analyzing source code and FS content.
        -> flash usage 65% (disable BLE Peripheral and Broadcast functions, switch from SPIFFS towith LittleFS)
- [x] **Test Framework:** setup initial test framework with Universal Workbench and pytest
        -> pytest environment configured and startup_test.py functional
- [x] **WiFi Stability:** Review the `while(true)` loop in `captivePortalSetup()` to ensure it cannot hang indefinitely.
- [x] **Captive Portal:** rework captive portal implementation, deactivate after timeout, retry to connect to Wifi, do not start captive portal unless reset
- [x] **Serial API:** add serial command API
        -> serial commands RESET, SCAN, READ, STATUS, SET_CONFIG, GET_CONFIG implemented
        -> refactored to non-blocking with internal buffer and direct JSON argument
- [x] **BLE Scan:** converted to asynchronous, non-blocking state machine
- [x] **Technical Specification (TSD):** Expand on the BLE decoding algorithm and the internal state machine, document status LED meaning.
- [x] **API Documentation:** Document the `/cmd` and `/status` endpoints.
- [x] **Config Cleanup:** Finalize `config_t` structure and remove legacy TODOs., rename config.address -> config.bleAddress
- [x] **Network Refactoring:** Implement Standby Mode, remove permanent offline mode, and update Serial API.

## Phase 2: Tests (Medium Priority)
- [x] **Unit Tests:** Implement tests Captive Portal, WiFi AP connection and Web site.
- [x] **MQTT Tests:** Implement tests for MQTT connection, status send, etc.
- [ ] **MQTT online:** deactivte MQTT when WiFi is not in station mode (offline or captive portal)
- [ ] **Offline mode:** stay in standby mode when activated manually (via OFFLINE command)

## Phase 3: Feature Enhancement & Connectivity (Medium Priority)
- [ ] **Enhance MQTT Robustness:** Improve MQTT connection handling and error reporting in the web status.
- [ ] **Web UI Optimization:** Improve the file upload process and provide more system information.
- [ ] **BLE Reading Improvements:** Improve error reporting when connection or decoding fails, and address potential thread-safety issues in `decodeData`.
- [ ] **Advanced BLE Diagnostics:** Add more logging for BLE connection and decoding steps.
- [ ] **Threshhold detection:** Implement min / max thresholds for PH, Chlorine, ORP, Temperature and battery voltage
        - plan implementation and update Roadmap, FSD, TSD and tests accordingly
        - add limits to config and web ui
        - allert on web ui, serial if value outside threshold
        - add multiple testcases for each value (min, max, normal,recovery from min/max)
        - send alert via mqtt if value outside threshold and on recovery
             {
                alert: true,
                type: "temp",
                value: 5,
                min: 10,
                max: 35,
                text: "Alert:Temperature too low"
             }
        - draft concept for email alerts if value outside threshold and on recovery or missing heartbeat (status message missing for X minutes)
             Send daily status summary via email (default 9am local time)
              - include min / max values for all values since last summary
              - trend since last summary
              - alert on deviation from threshold or missing heartbeat
              - status: normal, warning, critical, offline



## Phase 4: Optimization & Hardware Support (Low Priority)
- [ ] **Memory Management:** Monitor heap usage during long-term operation, especially with `ESPAsyncWebServer` and `ArduinoJson`.
- [ ] **Support for Multiple Boards:** Ensure all environments in `platformio.ini` are up-to-date and working.
- [ ] **Dynamic JSON Buffers:** Review `ArduinoJson` usage to ensure efficient memory allocation.

## Phase 5: Long-term Maintenance & Validation (Low Priority)
- [ ] **Unit Tests:** Implement tests for the BLE decoding logic.
- [ ] **Automated CI:** Set up GitHub Actions for automated builds and linting.

