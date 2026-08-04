# Nux

> **Devilboy — Born to race**

Nux is a radio-controlled off-road car built around an **STM32F103C8T6** micro-
controller and an **SX1276** sub-GHz radio link. The project is a monorepo that
contains two firmwares — the car (`vehicle`) and the hand-held controller
(`transmitter`) — plus the shared communication protocol they both compile
against.

The name comes from *Mad Max: Fury Road* — Nux, the reckless "war boy",
mirroring the **Devilboy** livery on the body shell.

---

## Status

Early development. See [CHANGELOG.md](CHANGELOG.md) for released changes,
[HISTORY.md](HISTORY.md) for the running work log, and
[requirements.md](requirements.md) for the tracked requirements.

**Version:** 0.1.0 (Unreleased)

---

## Features

- Bare-metal C11 firmware — **no HAL**, direct register access over CMSIS only.
- Cooperative task scheduling via **AcroSched** (linked as a frozen pre-built
  library; see [Scheduler](#scheduler)).
- Sub-GHz **SX1276** point-to-point half-duplex link at **868 MHz**, low-latency
  **GFSK** modulation.
- Open-loop drive: MOSFET H-bridge, reversible, PWM throttle.
- Servo steering (SG90), angle proportional to the commanded duty.
- Separate power domains: 2S 18650 traction pack, independent 1S 18650 logic
  cell, with battery-voltage telemetry back to the controller.
- Scale lighting: head-lights, side-lights, reverse light and blinking turn
  signals, switchable from the controller.

---

## Hardware overview

| Subsystem | Choice |
|---|---|
| MCU | STM32F103C8T6 (Blue Pill, prototyping) — Cortex-M3 |
| Drive | MOSFET H-bridge (target: DRV8251A, current-sense via IPROPI) |
| Steering | SG90 servo, 50 Hz / 1–2 ms pulse |
| Radio | SX1276, GFSK, 868 MHz, half-duplex point-to-point |
| Traction power | 2S 18650 (~7.4 V) with 2S BMS |
| Logic power | Independent 1S 18650, TP4056 + protection |
| Sensing | Battery voltage divider on ADC; motor current via IPROPI |
| Lighting | Head / side / reverse / turn signals (on-off, no PWM) |

There are **no tachometers or encoders**: throttle is open-loop (duty = throttle).

---

## Repository layout

```
Nux/
├─ common/                # shared by both firmwares
│  ├─ protocol/           # radio contract compiled into both sides
│  ├─ lib/                # frozen AcroSched: public headers + prebuilt libs
│  │  ├─ *.h              # public scheduler headers + acrosched_port.h
│  │  ├─ stm32f1-ac6/     # AC6 library (acrosched.lib)
│  │  └─ stm32f1-gcc/     # GCC library (libacrosched.a)
│  └─ sched/              # thin scheduler adapter (SysTick tick source)
├─ vehicle/               # car firmware (Nux)
│  ├─ inc/
│  └─ src/                # bsp, radio, motor, servo, power, lights
├─ transmitter/           # hand-held controller firmware
│  ├─ inc/
│  └─ src/                # bsp, radio, input (dual single-axis joysticks)
└─ wiki/                  # long-form documentation
```

---

## Communication protocol

Point-to-point, half-duplex, one command frame every **20 ms (50 Hz)**:

1. The transmitter sends a **command frame** (steering, throttle, light flags,
   a rolling counter and a CRC).
2. The transmitter switches to receive; the vehicle replies with a short
   **telemetry frame** (traction battery charge) inside the ACK window.

**Fail-safe:** if the vehicle misses **3 consecutive command frames (~60 ms)**
it ramps the throttle smoothly to zero (active braking, not coasting).

The exact frame layout, versioning and CRC live in
`common/protocol/` and are compiled into both firmwares from a single source, so
the contract can never drift between the two sides.

---

## Controls

- Two **single-axis joysticks** — one for steering, one for throttle
  (Hall-effect preferred over resistive).
- Push-buttons to toggle the lighting groups.

---

## Toolchain & build

- **CMSIS-Toolbox 2.13** (`csolution` / `cbuild`).
- Two supported compilers, selectable per build context:
  - **Arm Compiler 6 (AC6) 6.20.1**
  - **Arm GNU Toolchain (`arm-none-eabi`) 15.2.1**
- Flashing / debugging over **ST-Link (SWD)**.

---

## Scheduler

Nux runs on **AcroSched**, a lightweight static task scheduler. Its source lives
in a **private** repository and is **not** distributed here. Only the frozen,
pre-built libraries (`common/lib/ac6/`, `common/lib/gcc/`) and the public headers
(`common/lib/inc/`) are vendored into this public repo — enough to build and link
Nux, without exposing the scheduler's source. Application code talks to the
scheduler through the thin adapter in `common/sched/` rather than calling the
library directly.

---

## License

See [LICENSE](LICENSE).
