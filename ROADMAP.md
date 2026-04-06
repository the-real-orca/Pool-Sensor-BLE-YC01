# Roadmap - Pool Sensor BLE-YC01

This document consolidates the project's tasks and roadmap, providing a prioritized view of planned development and maintenance efforts.

## Phase 1: Documentation, Stability & Core Refinements (High Priority)
- [x] **Function Documentation:** Complete headers and descriptions for all functions to improve maintainability.
        -> documentation generated, see /docs
- [x] **Flash Size Optimization:** Address the high flash usage (currently >99%) by adjusting build flags and analyzing source code and FS content.
        -> flash usage 65% (disable BLE Peripheral and Broadcast functions, switch from SPIFFS towith LittleFS)
- [x] **Test Framework:** setup initial test framework with Universal Workbench and pytest
        -> pytest environment configured and startup_test.py functional
- [ ] **WiFi Stability:** Review the `while(true)` loop in `captivePortalSetup()` to ensure it cannot hang indefinitely.
- [ ] **Reboot Logic:** Verify the reboot timer and watchdog interaction.
- [ ] **General Code Cleanliness:** Clean up `main.cpp` and other source files (remove duplicate lines, commented-out code).
- [ ] **Technical Specification (TSD):** Expand on the BLE decoding algorithm and the internal state machine.
- [ ] **API Documentation:** Document the `/cmd` and `/status` endpoints.
- [ ] **Config Cleanup:** Finalize `config_t` structure and remove legacy TODOs.

## Phase 2: Feature Enhancement & Connectivity (Medium Priority)
- [ ] **Enhance MQTT Robustness:** Improve MQTT connection handling and error reporting in the web status.
- [ ] **Web UI Optimization:** Improve the file upload process and provide more system information.
- [ ] **Configurable LED:** Allow the status LED pin to be changed via the web interface.
- [ ] **BLE Reading Improvements:** Improve error reporting when connection or decoding fails, and address potential thread-safety issues in `decodeData`.
- [ ] **Advanced BLE Diagnostics:** Add more logging for BLE connection and decoding steps.

## Phase 3: Optimization & Hardware Support (Low Priority)
- [ ] **Memory Management:** Monitor heap usage during long-term operation, especially with `ESPAsyncWebServer` and `ArduinoJson`.
- [ ] **Support for Multiple Boards:** Ensure all environments in `platformio.ini` are up-to-date and working.
- [ ] **Dynamic JSON Buffers:** Review `ArduinoJson` usage to ensure efficient memory allocation.

## Phase 4: Long-term Maintenance & Validation
- [ ] **Unit Tests:** Implement tests for the BLE decoding logic.
- [ ] **Automated CI:** Set up GitHub Actions for automated builds and linting.
