# Coding Guidelines - Pool Sensor BLE-YC01

## 1. Overview
Mandatory standards for the "Pool Sensor BLE-YC01" firmware (ESP32). Focus: Stability, performance in embedded environments, and seamless collaboration between humans and AI.

## 2. Architecture & Design
- **Separation of Concerns (SoC):** Decouple hardware abstraction (BLE/WiFi), logic (decoding/processing), and I/O (Web/Serial).
- **Encapsulation:** Functions required only within a specific module must be declared as `static`.
- **Memory Management:** 
    - Avoid heap allocation (`malloc`, `new`) at runtime.
    - Prefer static buffers.
    - Use `NimBLE` Arduino `String` class instead of `std::string`.
- **Non-blocking Design:** No `delay()` calls in the main loop or time-critical modules. Use `millis()`-based state machines.

## 3. Style Rules (Hardened)
### Naming Conventions (C++)
- **Classes/Structs:** `PascalCase` (e.g., `BLEHandler`)
- **Methods/Functions:** `camelCase` (e.g., `decodeData()`)
- **Local Variables:** `camelCase` (e.g., `rawData`)
- **Global/Static Variables:** `camelCase` (e.g., `targetName`)
- **Constants/Macros:** `UPPER_SNAKE_CASE` (e.g., `MAX_BLE_RETRY`)
- **Namespaces (optional):** `snake_case`

### Naming Conventions (Python)
- **Standard:** Strict PEP 8.

### Formatting
- **Indentation:** Strictly 4 spaces (no tabs).
- **Braces:** K&R style (opening brace on the same line).
- **Header Files:** Use only `#ifndef` include guards.

## 4. Error Handling & Robustness
- **Input Validation:** All external input (Serial, Web, BLE) must be validated for length and type before processing.
- **Return Values:** Functions should signal success/failure (e.g., `bool` or error enums).
- **Logging:** 
    - Use the `DEBUG_print` and `DEBUG_println` macros.
    - Do not use raw `Serial.print` calls in production code.

## 5. Documentation
- **Why, not What:** Comments explain the intention behind complex algorithms (e.g., XOR decoding in `BLE-YC01.cpp`).
- **Doxygen:** API documentation in header files using `/** ... */`.

## 6. Git & Workflow
- **Surgical Commits:** Only one logical topic per commit.
- **Types:** `feature:`, `fix:`, `docs:`, `refactor:`, `test:`, `chore:`.
- **Review:** Check `git diff` before every commit.

## 7. AI Agent & Automation (Mandatory)
AI agents must follow these operative rules:
1. **Source-First:** Read existing code before proposing changes. Consistency takes precedence over "modern" approaches.
2. **No "Ghost Code" Deletions:** Ensure no relevant side effects or logging statements are lost when replacing code blocks.
3. **Surgical Updates:** Only change what is absolutely necessary. Avoid unnecessary refactoring of working code.
4. **Validation Requirement:** Every change must (theoretically or practically) be covered by a test case in `pytest/`.
5. **Zero-Trust Secrets:** Agents must never write or log API keys, WiFi passwords, or other sensitive data.
6. **Agent Idempotency:** Repeated executions of the same task must lead to the same stable result.
7. **No Auto-Commits:** Commits occur only after explicit user confirmation for each individual step.
