# Changelog

All notable changes to Nux are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added

- Initial project documentation: README, requirements, development history,
  changelog and wiki scaffold.
- Project scope and architecture decisions for the 0.1.0 milestone.
- Shared radio protocol implementation: `common/protocol/nux_protocol.c` (frame
  finalise/validate API) and a standalone CRC-16/CCITT-FALSE module
  (`common/protocol/nux_crc.{h,c}`).
- Thin scheduler adapter `common/sched/nux_sched.{h,c}` over the frozen
  AcroSched library, plus the Cortex-M3 port header.
- Board support package for both firmwares (`vehicle/bsp/`, `transmitter/bsp/`):
  STM32F103C8 clock/Flash/UART bring-up, IWDG, SysTick driving the scheduler
  adapter, and ARMv7-M fault handlers.
- CMSIS-Toolbox build system: `nux.csolution.yml`, the shared
  `common/nux_common.clayer.yml`, and the `vehicle`/`transmitter` projects with
  per-project STM32F103C8 memory map and linker scripts. Builds clean on both
  AC6 and GCC.
- GCC-only newlib syscall stubs (`bsp/src/syscalls.c`) that silence the
  `_close`/`_lseek`/`_read`/`_write` link-time warnings; excluded from AC6.

[Unreleased]: https://example.com/nux/compare/HEAD
