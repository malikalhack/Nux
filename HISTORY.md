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

### Scheduler adapter
- Vendored the frozen AcroSched 2.1.0 library into `common/lib/`: public headers
  in `inc/` plus the pre-built archives `ac6/acrosched.lib` and
  `gcc/libacrosched.a`. Confirmed the vendored `acrosched_config.h`
  matches the agreed configuration (cooperative, `MAX_TASKS = 8`, watchdog on,
  IPC on, timers off, idle hook on).
- Added the Cortex-M3 port header `common/lib/inc/acrosched_port.h` (32-bit tick,
  PRIMASK critical sections, `__WFI` wait-for-event); it is required because
  `acrosched_config.h` includes it and it is not part of the frozen library.
- Wrote the thin scheduler adapter `common/sched/nux_sched.{h,c}` (REQ-NUX-072):
  a self-contained `Nux*` interface that hides the `acro*` API, owns the 1 kHz
  tick counter (`nuxSchedTick` / `nuxSchedNow`, bound to the scheduler in
  `nuxSchedInit`), forwards task management, and re-exports the IPC event flags
  as `SNuxEventGroup_t` / `nuxEventSet`/`nuxEventGet`/`nuxEventClear`.
  `_Static_assert` guards keep the mode/status encodings and the tick width in
  lock-step with the pre-built library ABI.

## 2026-08-04

### Board support package
- Cleaned the imported BSP skeletons for both firmwares
  (`vehicle/bsp/`, `transmitter/bsp/`) down to a shared, board-agnostic base;
  the two copies are kept identical for now (per-peripheral split comes later).
- Retargeted from the STM32VLDISCOVERY template (STM32F100 Value Line, 24 MHz)
  to **STM32F103C8**: 72 MHz SYSCLK from the 8 MHz HSE crystal (PLL ×9), Flash
  2 wait states + prefetch, APB1 /2 (36 MHz), APB2 72 MHz; debug USART1 on
  PA9/PA10 re-tuned to `BRR = 625` for 115200 baud.
- Bound SysTick to the scheduler adapter: `SysTick_Handler` now calls
  `nuxSchedTick()`; dropped the BSP-owned `sys_tick`, the `acrosched.h` include
  and the pre-emption tick (cooperative kernel only).
- Kept the fault handlers (`handlers.c`) as a naked MSP/PSP trampoline into a
  common C handler that captures the stacked frame and fault-status registers.

