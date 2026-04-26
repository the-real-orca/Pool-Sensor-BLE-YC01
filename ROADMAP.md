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
- [x] **Reboot Logic:** Verify the reboot timer and watchdog interaction.
- [x] **Security:** disable download, or read out of config.json unless in DEBUG is set (debug mode)
- [ ] **Web Connection:** fix WiFi connection, do not restart captive portal unless explizite HW reset
- [ ] **General Code Cleanliness:** Clean up `main.cpp` and other source files (remove duplicate lines, commented-out code).
- [ ] **Technical Specification (TSD):** Expand on the BLE decoding algorithm and the internal state machine, document status LED meaning.
- [ ] **API Documentation:** Document the `/cmd` and `/status` endpoints.
- [ ] **Config Cleanup:** Finalize `config_t` structure and remove legacy TODOs., rename config.address -> config.bleAddress

## Phase 2: Tests (Medium Priority)
- [ ] **Unit Tests:** Implement tests Captive Portal, WiFi AP connection and Web site.

## Phase 3: Feature Enhancement & Connectivity (Medium Priority)
- [ ] **Enhance MQTT Robustness:** Improve MQTT connection handling and error reporting in the web status.
- [ ] **Web UI Optimization:** Improve the file upload process and provide more system information.
- [ ] **BLE Reading Improvements:** Improve error reporting when connection or decoding fails, and address potential thread-safety issues in `decodeData`.
- [ ] **Advanced BLE Diagnostics:** Add more logging for BLE connection and decoding steps.

## Phase 4: Optimization & Hardware Support (Low Priority)
- [ ] **Memory Management:** Monitor heap usage during long-term operation, especially with `ESPAsyncWebServer` and `ArduinoJson`.
- [ ] **Support for Multiple Boards:** Ensure all environments in `platformio.ini` are up-to-date and working.
- [ ] **Dynamic JSON Buffers:** Review `ArduinoJson` usage to ensure efficient memory allocation.

## Phase 5: Long-term Maintenance & Validation (Low Priority)
- [ ] **Unit Tests:** Implement tests for the BLE decoding logic.
- [ ] **Automated CI:** Set up GitHub Actions for automated builds and linting.

