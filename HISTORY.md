# Nux — Development History

A running log of project work, in reverse-chronological order. Release-facing
changes are summarised separately in [CHANGELOG.md](CHANGELOG.md).

---

## 2026-08-02

- Chose the project name **Nux** (the "war boy" from *Mad Max: Fury Road*),
  matching the **Devilboy — Born to race** body livery.
- Agreed the initial hardware direction: STM32F103C8T6, MOSFET H-bridge drive
  (target DRV8251A), SG90 servo steering, SX1276 GFSK link at 868 MHz, split 2S
  traction / 1S logic 18650 power, on/off scale lighting.
- Agreed the software architecture: bare-metal C11 (no HAL), cooperative
  AcroSched scheduler linked as a frozen pre-built library, shared radio
  protocol compiled into both firmwares.
- Fixed the monorepo layout: `common/` (protocol + scheduler libs + adapter),
  `vehicle/` and `transmitter/` firmwares.
- Recorded the initial project requirements in [requirements.md](requirements.md).
- Created the initial documentation set (README, requirements, history,
  changelog, wiki).
- Drafted the shared radio contract `common/protocol/nux_protocol.h` (little-
  endian packed frames, CRC-16/CCITT-FALSE): `SNuxCommand_t` (10 B) and
  `SNuxTelemetry_t` (9 B), light/status flag enums, `_Static_assert` size guards.

