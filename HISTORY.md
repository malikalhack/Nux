# Nux — Development History

A running log of project work in chronological order (newest entries at the
bottom). Release-facing changes are summarised separately in
[CHANGELOG.md](CHANGELOG.md).

---

## 2026-08-02

### Project inception
- Chose the project name **Nux** (the "war boy" from *Mad Max: Fury Road*),
  matching the **Devilboy — Born to race** body livery.

### Architecture decisions
- Hardware direction: STM32F103C8T6, MOSFET H-bridge drive (target DRV8251A),
  SG90 servo steering, SX1276 GFSK link at 868 MHz, split 2S traction / 1S logic
  18650 power, on/off scale lighting.
- Software architecture: bare-metal C11 (no HAL), cooperative AcroSched
  scheduler linked as a frozen pre-built library, shared radio protocol
  compiled into both firmwares.
- Monorepo layout: `common/` (protocol + scheduler libs + adapter), `vehicle/`
  and `transmitter/` firmwares.

### Documentation
- Recorded the initial project requirements in [requirements.md](requirements.md).
- Created the initial documentation set (README, requirements, history,
  changelog, wiki).

### Protocol
- Drafted the shared radio contract `common/protocol/nux_protocol.h` (little-
  endian packed frames, CRC-16/CCITT-FALSE): `SNuxCommand_t` (10 B) and
  `SNuxTelemetry_t` (9 B), light/status flag enums, `_Static_assert` size guards.

## 2026-08-03

### Architecture decisions
- Protocol draft resolved: axis resolution stays `int16` (±1000); telemetry
  keeps both `battery_mv` and `battery_pct`; the software in-payload CRC-16 is
  kept (end-to-end, independent of the SX1276 hardware CRC); the frame `seq` is
  widened `uint8` → `uint16`; the checksum moves into its own `nux_crc` module.
- AcroSched build configuration agreed: cooperative kernel,
  `ACROSCHED_MAX_TASKS = 8`, 32-bit tick (`uint32_t`) at 1 kHz; watchdog hook on
  (IWDG), idle / low-power `__WFI` hook on, software timers off (periodic task
  modes cover turn-signal blink, fail-safe and telemetry cadence). Radio RX uses
  IPC **event flags** (`RX_DONE` set by the DIO0 ISR), not the mailbox. Both
  firmwares link the same library configuration.

### Requirements
- Clarified REQ-NUX-074 (radio RX via IPC event flags). Added REQ-NUX-076
  (AcroSched build configuration), REQ-NUX-077 (hardware watchdog) and
  REQ-NUX-078 (idle / low-power hook).

### Protocol
- Widened the frame `seq` to `uint16`; command frame is now **11 B**, telemetry
  **10 B** (`_Static_assert` size guards updated).
- Extracted `nuxCrc16` (CRC-16/CCITT-FALSE) into `common/protocol/nux_crc.{h,c}`.
- Implemented the frame API in `common/protocol/nux_protocol.c`:
  `nuxCommandFinalize`/`nuxCommandValid`, `nuxTelemetryFinalize`/`nuxTelemetryValid`.
