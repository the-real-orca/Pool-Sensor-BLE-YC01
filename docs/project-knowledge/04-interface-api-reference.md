# Interface and API Reference

> Purpose: document inbound APIs, outbound APIs, events, files, CLIs, and examples
> Status: draft generated from static inventory; verify and complete against source code.

## Inbound HTTP/API Route Candidates

| Method | Path | File | Line | Framework hint |
| --- | --- | --- | --- | --- |
| ROUTE |  | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | 39 | csharp-aspnet |
| GET |  | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/references/language-framework-patterns.md | 39 | csharp-aspnet |
## Endpoint Documentation Template

For every endpoint, add request examples, response examples, validation rules, auth, side effects, idempotency, and errors.

```bash
# Example shape; replace with a verified endpoint
curl -X GET 'http://localhost:<port>/<path>' \
  -H 'Accept: application/json'
```

## Outbound Interfaces

_To be completed from source analysis._
## Events / Queues / Jobs

TODO

## Hardware / Device Interfaces

Document GPIO, EXTI/interrupts, ADC/DAC, PWM/timers, I2C, SPI/QSPI, UART/Serial, USB, CAN/TWAI, BLE, Wi-Fi, Ethernet, LoRa/LoRaWAN, cellular modem, MQTT/HTTP/CoAP, OTA, storage, sensors, displays, motors, relays, LEDs, and actuators. Include pins/buses, data formats, timing, and failure modes.

_To be completed from source analysis._
