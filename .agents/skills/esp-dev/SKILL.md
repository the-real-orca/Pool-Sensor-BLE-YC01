---
name: esp-dev
description: ESP32/ESP8266 firmware development standards using Arduino Core, PlatformIO, and ESP-IDF. Use when writing, reviewing, debugging, refactoring, hardening, or setting up ESP firmware, platformio.ini, Arduino sketches, ESP-IDF components, FreeRTOS tasks, Wi-Fi/MQTT/BLE code, OTA updates, Secure Boot, Flash Encryption, NVS, provisioning, partitions, Unity tests, pytest-embedded, or HIL test rigs.
---

# ESP Firmware Development Skill

Use this skill to make professional, reliable, testable, updateable, and secure firmware changes for ESP32/ESP8266-based IoT devices. Prefer existing project conventions over generic defaults; use the defaults below when starting a new project or when a repository has no clear convention.

## Trigger Signals

Use this skill for tasks mentioning:

- `esp32`, `esp8266`, `esp32-s2`, `esp32-s3`, `esp32-c3`, `esp32-c6`, `esp32-h2`, Espressif
- `platformio.ini`, `pio run`, `pio test`, `pio check`, `pio device monitor`
- `arduino-cli`, `.ino`, Arduino Core, `arduino-lint`, FQBN
- `idf.py`, `sdkconfig`, `sdkconfig.defaults`, `partitions.csv`, ESP-IDF components, FreeRTOS
- `WiFi.h`, `PubSubClient`, MQTT, BLE, provisioning, SoftAP, SNTP, TLS on device
- OTA, `esp_ota_*`, `esp_https_ota`, rollback, Secure Boot, Flash Encryption, eFuse, NVS
- `Unity`, `pytest-embedded`, HIL, Wokwi, Mosquitto for tests
- Watchdog resets, brownout, heap fragmentation, stack overflow, panics, backtraces
- `esp_err_t`, `ESP_ERROR_CHECK`, error handling, retry/backoff, reboot loops, safe mode, core dumps

Do not apply this skill to non-firmware work, unrelated host tooling, or generic embedded targets (STM32, nRF, RP2040) unless the user explicitly asks for ESP guidance.

## Agent Workflow

1. Inspect the existing project first: `platformio.ini` envs, `sdkconfig*`, partition table, framework (Arduino/ESP-IDF), target chips, libraries, tests, CI, and pin map.
2. Preserve existing conventions unless they are unsafe, broken, or the user asks to modernize them.
3. Make the smallest correct change; avoid broad refactors, opportunistic library bumps, or unrelated formatting.
4. Keep firmware behavior non-blocking, observable through logs, and testable on the host where possible.
5. Add or update tests for changed behavior, especially state machines, parsers, retry/backoff, OTA logic, and security-sensitive paths.
6. Run the relevant quality gates: build for all target boards, native tests, embedded tests if hardware is available, `pio check`, and formatting.
7. Report what changed, why it changed, which boards were built, and how it was validated.

## Core Principles

| Principle | Practical rule |
|---|---|
| Reproducible builds | Commit `platformio.ini`, `sdkconfig.defaults`, partition tables, lockfiles; pin lib versions for production |
| Non-blocking by default | Use `millis()`/state machines instead of `delay()`; never block forever on Wi-Fi/MQTT |
| Resource-aware | Bound buffers, JSON sizes, reconnect attempts; watch heap and stack |
| Testable logic | Separate pure logic from `Serial`, `WiFi`, `digitalRead`, MQTT so it can run as native tests |
| Updateable | OTA with rollback and self-test from day one |
| Secure by default | TLS with verification, per-device credentials, no hardcoded secrets, validated inputs |
| Power-loss resilient | Validate config, keep last-known-good, avoid frequent flash writes |
| Fail loud, recover quietly | Log errors with context, then degrade gracefully — never crash silently or spin in tight retry loops |
| Layered testing | Static checks, native unit tests, embedded unit tests, HIL for end-to-end |

## Toolchain Defaults

For new projects, prefer **PlatformIO**:

- PlatformIO for project structure, build, upload, tests, dependencies, CI.
- Arduino Core for fast development and the library ecosystem.
- ESP-IDF for production-grade Secure Boot, Flash Encryption, robust OTA, FreeRTOS, low-level peripherals.
- Unity for embedded and native unit tests.
- pytest-embedded for HIL and serial-driven device tests.
- `pio check` (cppcheck, clang-tidy) for static analysis.
- `clang-format` (Google base, 2-space indent, 100 col) for formatting.

Install:

```bash
python3 -m pip install --upgrade platformio
pio --version
```

Common commands:

```bash
pio run                    # Build all default envs
pio run -e esp32dev        # Build a single env
pio run -t upload          # Flash firmware
pio device monitor -b 115200
pio test                   # Run tests
pio test -e native         # Native (host) tests only
pio check                  # Static analysis
pio pkg update             # Deliberately update deps
```

## Project Layout (PlatformIO + Arduino)

```text
project/
├── platformio.ini
├── include/                # public headers, config.hpp
├── lib/                    # local libraries with library.json
├── src/                    # main.cpp + .cpp/.hpp modules
├── test/
│   ├── test_logic/         # native unit tests
│   ├── test_config/
│   └── test_embedded_*/    # on-device tests
└── scripts/                # pre/post build hooks
```

Avoid putting all logic in `main.cpp` or a giant `.ino`. Move logic into `.h`/`.cpp` modules so it can be tested and reused.

## `platformio.ini` Defaults

Recommended starting point with versioned deps, debug/release envs, and static analysis:

```ini
[platformio]
default_envs = esp32dev

[env]
framework = arduino
monitor_speed = 115200
test_framework = unity
build_flags =
  -D APP_NAME=\"esp-device\"
  -D FW_VERSION=\"0.1.0\"
lib_deps =
  bblanchon/ArduinoJson@^7
check_tool = cppcheck, clangtidy
check_flags =
  cppcheck: --enable=warning,style,performance,portability
  clangtidy: --checks=-*,bugprone-*,performance-*,readability-*

[env:esp32dev]
platform = espressif32
board = esp32dev

[env:esp32_debug]
extends = env:esp32dev
build_type = debug
build_flags = ${env.build_flags} -D DEBUG=1 -D CORE_DEBUG_LEVEL=5

[env:native]
platform = native
test_framework = unity
```

Pin libraries for production (`@7.4.1`), not floating ranges.

## Arduino Essentials

Keep `setup()`/`loop()` small. Drive everything from a state machine.

```cpp
unsigned long lastRun = 0;
const unsigned long intervalMs = 10000;

void loop() {
  unsigned long now = millis();
  if (now - lastRun >= intervalMs) {
    lastRun = now;
    readSensor();
  }
  mqttClient.loop();
}
```

Forbidden in production loops: `delay()` longer than a few ms, `while (WiFi.status() != WL_CONNECTED)` without a timeout, blocking HTTP/MQTT calls without bounded retries.

Always log the firmware version on boot and on state transitions.

## State Machines

IoT firmware is a state machine. Make states explicit; do not scatter Boolean flags.

```cpp
enum class DeviceState {
  Boot, Provisioning, ConnectingWiFi, ConnectingMqtt, Online, Updating, Error
};
```

Recommended module split:

```text
AppController
├── NetworkManager
├── MqttService
├── SensorService
├── StorageService
├── OtaService
├── ProvisioningService
└── DiagnosticsService
```

Rules: encapsulate hardware access; separate parsing from execution; keep pure logic free of `Arduino.h` so it can run native tests.

## Networking

Wi-Fi: always use a bounded timeout, exponential backoff for reconnects, distinct error states, a provisioning fallback, and never log passwords.

MQTT in production:

- TLS with verified server certificates.
- Per-device credentials and unique client IDs.
- ACLs scoped to the device's topic prefix.
- Bounded payload sizes; reject unknown commands.
- Last Will and Testament for online/offline state.
- Topic convention: `devices/<device_id>/{telemetry,state,cmd,ota}`.

## Memory, Timing, Reliability

- Prefer `StaticJsonDocument<N>` with a fixed bound; avoid `DynamicJsonDocument` for untrusted payloads.
- Avoid heap allocations in hot loops; reuse buffers.
- Limit `String` concatenation; prefer `snprintf` into fixed buffers.
- Log `ESP.getFreeHeap()` periodically during development.
- Do not disable the watchdog. Split long work, yield, and use timeouts.
- Expect resets and power loss: validate config, keep last-known-good, avoid frequent flash writes.

## Error Handling

Error handling is a first-class part of firmware design, not an afterthought. The device is unattended, hard to debug remotely, and must survive transient failures without bricking itself or burning flash in reboot loops.

### Error Categories

Classify every failure path explicitly. The class drives the response.

| Class | Examples | Response |
|---|---|---|
| Transient | Wi-Fi drop, MQTT disconnect, DNS, NTP, sensor timeout | Retry with bounded backoff, stay in current state |
| Recoverable | Bad command payload, missing optional config, single bad sensor read | Log, reject input, continue running |
| Degraded | One subsystem dead (e.g. sensor) but device still useful | Mark subsystem unhealthy, publish state, continue core function |
| Fatal-soft | Repeated subsystem failure, OTA self-test fail, config corruption | Enter Safe Mode / Provisioning state, await intervention |
| Fatal-hard | Stack overflow, heap exhausted, hardware fault, invalid pointer | Controlled reset with reason recorded |

Never treat all errors the same. Do not reboot on a malformed MQTT payload, and do not silently ignore a watchdog reset.

### Return Value Checking

Check every return value of API calls that can fail. Ignored returns are the single most common bug source in ESP firmware.

Bad:

```cpp
mqttClient.publish(topic, payload);
preferences.begin("app", false);
WiFi.begin(ssid, pass);
```

Better:

```cpp
if (!mqttClient.publish(topic, payload)) {
  LOG_WARN("mqtt publish failed topic=%s state=%d", topic, mqttClient.state());
  metrics.mqttPublishFailures++;
}
```

For ESP-IDF, use `esp_err_t` consistently and handle it explicitly:

```c
esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return err;
}
```

`ESP_ERROR_CHECK(...)` aborts on failure — use it only for initialization paths where continuing makes no sense (e.g. NVS partition init at boot). For runtime paths, handle the error and return it up the stack. Prefer `ESP_ERROR_CHECK_WITHOUT_ABORT` plus an explicit decision when the caller can recover.

Propagate errors with context, do not swallow them:

```c
esp_err_t app_storage_load(app_config_t* out) {
    esp_err_t err = nvs_open(NS, NVS_READONLY, &h);
    if (err != ESP_OK) return err;            // propagate
    err = nvs_get_blob(h, "cfg", out, &len);
    nvs_close(h);
    return err;
}
```

### Exponential Backoff with Jitter

Tight retry loops are an anti-pattern: they DoS the broker, drain power, and starve other tasks. Always cap retries with exponential backoff plus jitter.

```cpp
class Backoff {
 public:
  Backoff(unsigned long minMs, unsigned long maxMs)
      : min_(minMs), max_(maxMs), current_(minMs) {}

  unsigned long nextDelay() {
    unsigned long jitter = random(0, current_ / 4 + 1);
    unsigned long delayMs = current_ + jitter;
    current_ = min(current_ * 2, max_);
    return delayMs;
  }

  void reset() { current_ = min_; }

 private:
  unsigned long min_, max_, current_;
};

// Usage: Backoff wifiBackoff(1000, 60000);
// Reset on success, advance on failure.
```

Reset the backoff on every successful operation. Never use `delay(backoff)` inside `loop()` — store `nextAttemptAt = millis() + delay` and check non-blocking.

### Timeouts

Every external interaction must have a bounded timeout: Wi-Fi connect, DNS, TLS handshake, HTTP request, MQTT connect, sensor read, mutex acquisition, queue receive. A missing timeout is a latent deadlock.

```cpp
constexpr unsigned long WIFI_TIMEOUT_MS = 15000;
constexpr unsigned long MQTT_TIMEOUT_MS = 10000;
constexpr unsigned long HTTP_TIMEOUT_MS = 10000;
constexpr unsigned long SENSOR_TIMEOUT_MS = 500;
```

Centralize timeouts as named constants — never use magic numbers scattered across modules.

### Reboot Loop Protection

A device that crashes immediately after boot will burn through flash writes, drain batteries, and spam OTA servers. Detect and break the loop.

Pattern:

1. On boot, increment a `boot_counter` in NVS.
2. After the device has been "stable" (e.g. 5 minutes online, OTA self-test passed), reset the counter.
3. If `boot_counter` exceeds a threshold (e.g. 5), enter **Safe Mode**: disable optional subsystems, force provisioning/diagnostics, await OTA recovery.

```cpp
constexpr uint8_t BOOT_LOOP_THRESHOLD = 5;
constexpr unsigned long STABLE_AFTER_MS = 5 * 60 * 1000;

void onBoot() {
  uint8_t count = prefs.getUChar("boot_count", 0) + 1;
  prefs.putUChar("boot_count", count);
  if (count >= BOOT_LOOP_THRESHOLD) enterSafeMode();
}

void onStable() {
  prefs.putUChar("boot_count", 0);
}
```

Always combine this with the ESP reset reason:

```cpp
esp_reset_reason_t reason = esp_reset_reason();
LOG_INFO("boot reason=%d count=%u version=%s", reason, count, FW_VERSION);
```

Treat `ESP_RST_PANIC`, `ESP_RST_INT_WDT`, `ESP_RST_TASK_WDT`, `ESP_RST_BROWNOUT` differently from `ESP_RST_POWERON` and `ESP_RST_SW`.

### Safe Mode

Safe Mode is a minimal, guaranteed-bootable state used for recovery. It must:

- Skip optional features (sensors, scheduled jobs, application logic).
- Enable provisioning or a diagnostic endpoint.
- Allow OTA recovery from a known good image.
- Publish its state so operators can see the device is alive but degraded.

Safe Mode is the destination for `Fatal-soft` errors — never try to "fix" corrupted state from inside the failing subsystem.

### Watchdog Strategy

Do not disable the Task Watchdog (TWDT) or Interrupt Watchdog (IWDT). If a task triggers it, the bug is in the task — fix the bug.

Rules:

- Long operations must yield: `vTaskDelay(1)` or `taskYIELD()` periodically.
- Subscribe long-running custom tasks to the TWDT explicitly (`esp_task_wdt_add`) so silent hangs are caught.
- Never call blocking network APIs from an ISR or a high-priority task without a queue.
- Avoid `delay()` longer than the watchdog timeout in `loop()`.

### Brownout & Power Loss

- Brownout detector enabled (default on ESP32). Treat `ESP_RST_BROWNOUT` as a signal of bad power supply, not a software bug.
- Never write to NVS or flash during a known unstable supply state (e.g. boot before regulator settles).
- Make multi-step flash writes atomic where possible: write to a shadow key, then commit a "version" or "valid" flag last. On boot, only accept config marked valid; otherwise fall back to last-known-good.

### Crash Diagnostics

For non-trivial projects, enable ESP-IDF **Core Dump** to flash or UART so post-mortem backtraces survive a reset. In development, decode panics with `addr2line` or `idf.py monitor` (which symbolizes automatically).

```ini
; sdkconfig.defaults
CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH=y
CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF=y
CONFIG_ESP_COREDUMP_CHECKSUM_CRC32=y
```

On the next successful boot, upload the core dump to a backend or expose it via a diagnostic endpoint, then erase the partition. Never leak core dumps in plaintext logs in production — they may contain sensitive memory.

### Error Reporting

Once the device is online, surface errors centrally:

- Publish a periodic `state` topic with: firmware version, uptime, free heap, last error code, last error timestamp, subsystem health flags, boot counter, reset reason.
- Publish one-shot error events on a dedicated `events` topic with rate limiting (e.g. max 1/sec, drop-with-counter on overflow).
- Define a small, stable error code space (enum) — do not ship freeform strings as the primary identifier.

```cpp
enum class ErrorCode : uint16_t {
  Ok               = 0,
  WifiTimeout      = 1001,
  MqttConnectFail  = 1002,
  TlsHandshake     = 1003,
  ConfigInvalid    = 2001,
  SensorTimeout    = 3001,
  OtaVerifyFail    = 4001,
  // ...
};
```

### Logging Errors

Log every error with: code, subsystem, what was attempted, what value was seen, and what will happen next.

```cpp
LOG_ERROR("mqtt connect failed code=%d state=%d host=%s next_retry_ms=%lu",
          (int)ErrorCode::MqttConnectFail, mqttClient.state(),
          cfg.mqttHost, backoff.peek());
```

Bad: `Serial.println("error");` — useless in field debugging.

Never log secrets, tokens, certificates, or full request bodies on error paths.

### Input Validation as Error Handling

Most "errors" are actually invalid input. Validate at the boundary, reject early, and never let bad input propagate into state or flash.

```cpp
StaticJsonDocument<512> doc;
DeserializationError jerr = deserializeJson(doc, payload, len);
if (jerr) {
  reportError(ErrorCode::CmdParseFail, jerr.c_str());
  return;
}
const char* cmd = doc["cmd"] | "";
if (!isAllowedCommand(cmd)) {
  reportError(ErrorCode::CmdNotAllowed, cmd);
  return;
}
```

Allow-list, never deny-list. Bound every string and array length explicitly.

### C++ Exceptions

Exceptions are disabled by default in Arduino-ESP32 and ESP-IDF builds and add code size and stack overhead. Do **not** introduce `try`/`throw`/`catch` in firmware. Use:

- `bool` return + out-parameter, or
- `std::optional<T>` / `tl::expected<T, E>` patterns, or
- ESP-IDF `esp_err_t` end-to-end.

### OTA Error Handling

OTA failures must never brick the device. Required flow:

1. Validate image size, target, and version before writing.
2. Verify signature/hash after download, before activating.
3. Reboot into the new partition.
4. Run a self-test within a bounded window.
5. On self-test fail or panic before the marker, the bootloader auto-rolls back via `otadata`.
6. Report the OTA outcome (success or rollback reason) on next online.

Never set `esp_ota_mark_app_valid_cancel_rollback()` immediately at boot — defeat-the-rollback is a recurring real-world bug.

### Error Handling Anti-Patterns

- Silent `catch (...)` or ignored return value → data loss, undebuggable failures.
- `delay(60000)` inside an error path → blocks the entire system.
- Rebooting on every error → reboot loop, flash wear, denial of service to itself.
- `ESP_ERROR_CHECK` in runtime hot paths that have a recoverable alternative.
- Using `assert()` in production builds for input validation.
- One global `last_error` int with no context.
- Logging only `"failed"` without error code, subsystem, or attempted operation.
- Retrying forever with no backoff and no max attempts.
- Hiding errors from the cloud / operator — devices that look "fine" but are degraded.

### Error Handling Review Checklist

- [ ] Every fallible API call's return value is checked.
- [ ] Every external operation has a bounded timeout.
- [ ] Retries use exponential backoff with jitter and a max delay.
- [ ] Errors are classified (transient / recoverable / degraded / fatal-soft / fatal-hard).
- [ ] Reboot loop protection exists with a Safe Mode fallback.
- [ ] Reset reason is logged on every boot.
- [ ] No `client.setInsecure()`, no disabled watchdog, no `delay()` in error paths.
- [ ] Error codes are defined as a stable enum, not freeform strings.
- [ ] Errors are surfaced to the operator via state/events topics.
- [ ] OTA has self-test + rollback wired up and tested.
- [ ] Logs contain context but no secrets.
- [ ] Tests exist for at least the parser, state-transition, and backoff error paths.

## Configuration & Secrets

| Type | Examples | Storage |
|---|---|---|
| Compile-time | feature flags, board variant, pin map | `build_flags`, headers |
| Runtime | Wi-Fi, MQTT host, device name | NVS / Preferences |
| Secret material | tokens, keys, certificates | encrypted NVS or manufacturing flow |

Never commit: Wi-Fi/MQTT passwords, API tokens, private keys, production certificates, OTA signing keys.

Provide `secrets.example.h` and add the real `secrets.h` to `.gitignore` along with `*.pem`, `*.key`, `*.p12`, `.env`.

Always validate config before use:

```cpp
bool isValidMqttHost(const String& host) {
  return host.length() > 0 && host.length() < 128 && host.indexOf(' ') < 0;
}
```

## Security Rules

Treat security as a release blocker, not a follow-up.

1. **TLS**: never call `client.setInsecure()` in production. Use `client.setCACert(root_ca)` or the ESP certificate bundle. Set device time via SNTP before TLS handshakes.
2. **Secure Boot v2**: recommended for production devices with tamper risk; signing keys stay out of the repo; eFuse steps are often irreversible — validate on test devices first.
3. **Flash Encryption**: development mode is not production. Production mode can permanently restrict debug/update flows. Test OTA and recovery before enabling.
4. **NVS**: do not store tokens/credentials in plaintext Preferences for production; evaluate encrypted NVS.
5. **Provisioning**: prefer BLE/SoftAP/Unified Provisioning with proof-of-possession over hardcoded credentials. Disable provisioning after setup; re-enable only via timeout or physical action. Factory reset must wipe credentials cleanly.
6. **OTA**: HTTPS and signed firmware (ideally both); check version, target hardware, and size; rollback with self-test; downgrade protection; limit update attempts.
7. **MQTT/HTTP APIs**: per-device credentials, ACLs, payload-size limits, schema validation, allow-lists for commands, no unauthenticated OTA endpoints.
8. **Debug interfaces**: disable or secure JTAG in production; reduce debug logs; never emit secrets in panics, backtraces, or logs.
9. **Input validation**: every network payload is untrusted. Bound JSON, check types, reject unknown fields.

```cpp
StaticJsonDocument<512> doc;
if (deserializeJson(doc, payload)) return false;
if (!doc["cmd"].is<const char*>()) return false;
```

## OTA Update Pattern

Partitions: `factory` or initial app, `ota_0`, `ota_1`, `otadata`, NVS/config.

After booting new firmware:

1. Detect pending verification.
2. Run minimal diagnostics (config loads, tasks start, peripherals init, network stack up, no immediate reset).
3. Mark firmware valid if diagnostics pass.
4. Trigger rollback if they fail.

The OTA server must reject wrong hardware target, downgrades without explicit approval, incompatible partitioning, and unsigned/invalid images. Embed `FW_VERSION` and a `FW_BUILD_ID` in firmware.

## Testing Strategy

| Layer | Where | Purpose | Tools |
|---|---|---|---|
| Static checks | host/CI | catch errors pre-runtime | `pio check`, cppcheck, clang-tidy, `arduino-lint` |
| Native unit tests | host | pure logic, fastest | Unity native, GoogleTest, Doctest |
| Embedded unit tests | ESP board | hardware-adjacent code | PlatformIO Unity, ESP-IDF Unity |
| Integration tests | ESP + services | Wi-Fi/MQTT/API | pytest, pytest-embedded, Docker Mosquitto |
| HIL tests | real hardware + rig | end-to-end behavior | pytest-embedded, relays, serial |

Test priority for changes: config parsing, command parsing, state transitions, debounce logic, retry/backoff, sensor conversion, JSON payload generation, MQTT topic construction, OTA version comparison. Do not start with live Wi-Fi tests — they are slow and flaky.

Native test env makes pure logic CI-friendly:

```ini
[env:native]
platform = native
test_framework = unity
```

pytest-embedded for device-level tests:

```python
def test_boot_message(dut):
    dut.expect("Booting")
    dut.expect("WiFi init")
```

## CI/CD Minimums

Per pull request:

- Build all target boards.
- Run native tests.
- Run embedded tests if a board is available in CI.
- `pio check` static analysis.
- Formatting check.
- `arduino-lint` for Arduino libraries/sketches.

Release artifacts: app `.bin`, bootloader, partition table, manifest with version and hash, changelog, signature metadata. Never publish private signing keys.

## Anti-Patterns to Reject

When reviewing or generating code, flag and rewrite:

- `while (WiFi.status() != WL_CONNECTED) delay(1000);` → bounded timeout + state machine.
- `client.setInsecure();` in production → CA cert / cert bundle / pinning.
- Hardcoded passwords/tokens → provisioning + per-device credentials.
- Hundreds-of-lines `loop()` mixing networking, sensors, storage, commands → service classes + state machine.
- OTA without rollback or self-test → dual partitions + diagnostics + rollback.
- `preferences.putUInt(...)` inside `loop()` every iteration → write only on real changes; debounce; keep counters in RAM.
- Ignoring return values of `mqttClient.publish`, `WiFi.begin`, `client.connect` → check and log failures.
- `DynamicJsonDocument(10000)` for untrusted input → `StaticJsonDocument<N>` with bound + schema check.
- One global mutable config touched everywhere → config service with snapshots and explicit update methods.
- Reusing ESP8266 assumptions on ESP32 (or vice versa) → always test on the real target.
- Rebooting on any error (including bad MQTT payloads) → classify the error first; only `Fatal-hard` warrants reset.
- `ESP_ERROR_CHECK` on runtime/recoverable paths → propagate `esp_err_t` and decide explicitly.
- Tight retry loops with no backoff or max attempts → exponential backoff with jitter and a cap.
- Disabling the watchdog to "fix" a hang → split the work and yield.

## Logging Rules

Good logs answer: current state, which operation failed, error code, when it will retry, firmware version. They never contain passwords, tokens, private keys, full provisioning payloads, or personal data.

Use a leveled macro rather than scattered `Serial.println`:

```cpp
#define LOG_INFO(msg) Serial.printf("[INFO] %lu %s\n", millis(), msg)
```

## Naming & Style

| Element | Style | Example |
|---|---|---|
| Classes | PascalCase | `NetworkManager` |
| Functions | camelCase | `connectWiFi()` |
| Constants | `UPPER_CASE` or `kCamelCase` | `WIFI_TIMEOUT_MS`, `kTimeoutMs` |
| Private fields | trailing underscore | `client_` |
| Files | snake_case | `network_manager.cpp` |

Consistency within a project beats the exact convention.

## Pre-Change Checklist

Before proposing a change:

- [ ] Builds for all target environments.
- [ ] Tests added or updated for the new behavior.
- [ ] No secrets in the diff.
- [ ] Logs are useful and contain no credentials.
- [ ] Error paths exist for failures (Wi-Fi, MQTT, OTA, parse) with classification, timeout, and bounded backoff.
- [ ] Reset reason and reboot-loop protection considered for any change touching boot or main control flow.
- [ ] Buffer/JSON sizes are bounded.
- [ ] Memory impact considered (heap, stack, flash writes).
- [ ] OTA compatibility considered (partition layout, version, signing).
- [ ] Documentation, pin map, and MQTT API doc updated when relevant.

## Debugging Cheat Sheet

| Symptom | First checks |
|---|---|
| Upload fails | `pio device list`, cable, BOOT button, `dialout` group |
| Random reset | `esp_reset_reason()`, brownout, watchdog, panic backtrace, power supply, core dump partition |
| Reboot loop | check NVS boot counter, reset reason, OTA self-test marker, last logged error code |
| Wi-Fi unstable | RSSI, blocking calls, power supply, reconnect logic |
| MQTT disconnects | broker logs, client ID collisions, keepalive, TLS time/cert |
| TLS error | SNTP time set, CA bundle, server chain, expiry |
| Tests hang | `test_speed`, monitor baud, missing `UNITY_END()` |
| Native tests do not compile | hardware dependency leaking — abstract or mock |
| OTA bricked device | partition table, target match, rollback logic, signing |
| Config disappears | NVS namespace changed, erase history, key migration |

```bash
pio device list
pio device monitor -b 115200
pio run -v
pio run -t clean
pio run -t erase
```

## When to Reach for ESP-IDF Instead of Arduino

Move to ESP-IDF (or coexist via Arduino-as-component) when the project needs Secure Boot v2, Flash Encryption in production mode, complex partition tables, robust OTA with rollback APIs, fine-grained FreeRTOS task control, low-level networking/peripherals, or production-grade certification.

For new security-relevant products, prefer ESP32 family over ESP8266.

## Authoritative References

- Espressif ESP-IDF Security Guides: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/
- ESP-IDF Secure Boot v2: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/security/secure-boot-v2.html
- ESP-IDF OTA & Rollback: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/ota.html
- ESP-IDF Unified Provisioning: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/provisioning/provisioning.html
- ESP x509 Certificate Bundle: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_crt_bundle.html
- Arduino-ESP32 Core: https://docs.espressif.com/projects/arduino-esp32/en/latest/
- ESP8266 Arduino Core: https://github.com/esp8266/Arduino
- Arduino CLI: https://docs.arduino.cc/arduino-cli/
- Arduino Lint: https://arduino.github.io/arduino-lint/1.2/
- PlatformIO Project Configuration: https://docs.platformio.org/en/latest/projectconf/
- PlatformIO Unit Testing: https://docs.platformio.org/en/stable/advanced/unit-testing/index.html
- PlatformIO Static Code Analysis: https://docs.platformio.org/en/latest/advanced/static-code-analysis/index.html
- ESP-IDF Unit Testing with Unity: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/unit-tests.html
- pytest-embedded: https://docs.espressif.com/projects/pytest-embedded/en/latest/

When uncertain about an Espressif security/OTA/provisioning detail, fetch the relevant doc above rather than guessing — incorrect eFuse, partition, or signing steps are often irreversible.
