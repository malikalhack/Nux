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

[Unreleased]: https://example.com/nux/compare/HEAD
