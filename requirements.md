# Nux — Requirements

Tracked requirements for the Nux RC car project. Each requirement has a stable
`REQ-NUX-xxx` identifier so it can be referenced from commits, history entries
and code comments.

**Version:** 0.1.0 (Unreleased)

Status legend: **Draft** (agreed, not implemented) · **WIP** · **Done** ·
**Deferred**.

---

## 1. Platform

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-001 | The firmware targets the **STM32F103C8T6** (Cortex-M3, Blue Pill) for prototyping. | Draft |
| REQ-NUX-002 | Source language is **C11**. | Draft |
| REQ-NUX-003 | **No HAL**: hardware is accessed by direct register manipulation over CMSIS only. | Draft |
| REQ-NUX-004 | The project is a **monorepo** with two firmwares (`vehicle`, `transmitter`) and a shared protocol. | Draft |

## 2. Drive

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-010 | The traction motor is driven by a **MOSFET H-bridge** (efficiency-first; target part DRV8251A). | Draft |
| REQ-NUX-011 | The drive must support **reverse**. | Draft |
| REQ-NUX-012 | Throttle is **open-loop**: PWM duty maps to throttle (no tachometer/encoder available). | Draft |
| REQ-NUX-013 | Motor current is sensed (driver IPROPI or shunt) for coarse load feedback. | Deferred |

## 3. Steering

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-020 | Steering uses an **SG90** servo driven at **50 Hz** (1–2 ms pulse). | Draft |
| REQ-NUX-021 | Steering angle is proportional to the commanded value. | Draft |
| REQ-NUX-022 | Configurable end-point limits and centre trim. | Deferred |

## 4. Radio link

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-030 | The link uses an **SX1276** transceiver, **point-to-point**, **half-duplex**. | Draft |
| REQ-NUX-031 | Modulation is **GFSK** (not LoRa) to minimise latency. | Draft |
| REQ-NUX-032 | Carrier frequency is **868 MHz**. | Draft |
| REQ-NUX-033 | The transmitter sends a command frame every **20 ms (50 Hz)**. | Draft |
| REQ-NUX-034 | The vehicle returns **telemetry** (traction battery charge) in the ACK window. | Draft |
| REQ-NUX-035 | **Fail-safe:** after **3 missed frames (~60 ms)** the vehicle ramps throttle to zero (active braking, not coasting). | Draft |
| REQ-NUX-036 | End-to-end control latency (stick → wheels) target **≤ 30 ms**. | Draft |

## 5. Power

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-040 | **Separate power domains**: traction and logic. | Draft |
| REQ-NUX-041 | Traction pack is **2S 18650** (~7.4 V) with a 2S BMS. | Draft |
| REQ-NUX-042 | Logic is an **independent 1S 18650** (TP4056 + protection). | Draft |
| REQ-NUX-043 | Batteries are **chargeable in place** (no removal); method not yet finalised. | Draft |
| REQ-NUX-044 | Battery voltage is measured on **ADC** (divider + RC filter). | Draft |
| REQ-NUX-045 | Reverse-polarity and over-discharge protection on the traction line. | Draft |

## 6. Lighting

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-050 | Lighting groups: **head-lights, side-lights, reverse light, turn signals (L/R)**. | Draft |
| REQ-NUX-051 | Lighting is **on/off only** (no brightness PWM). | Draft |
| REQ-NUX-052 | Turn signals **blink** (~1.5–2 Hz). | Draft |
| REQ-NUX-053 | Reverse light follows the drive direction automatically. | Draft |
| REQ-NUX-054 | All lighting can be disabled from the transmitter. | Draft |

## 7. Controller (transmitter)

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-060 | The transmitter is STM32 + SX1276, powered from a **1S 18650**. | Draft |
| REQ-NUX-061 | Input is **two single-axis joysticks** (steering, throttle); Hall-effect preferred. | Draft |
| REQ-NUX-062 | Push-buttons toggle the lighting groups. | Draft |
| REQ-NUX-063 | The transmitter displays the vehicle's reported battery charge. | Deferred |

## 8. Software architecture

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-070 | Task scheduling uses **AcroSched** (cooperative kernel). | Draft |
| REQ-NUX-071 | The scheduler is linked as a **frozen pre-built library**; its source is **not** vendored into this public repo. | Draft |
| REQ-NUX-072 | Application code accesses the scheduler through a **thin adapter** (`common/sched/`), not directly. | Draft |
| REQ-NUX-073 | The system tick source is **SysTick at 1 kHz**; the control loop runs at **50 Hz**. | Draft |
| REQ-NUX-074 | Radio RX signals a task via scheduler **IPC event flags**: a DIO0 ISR sets an `RX_DONE` bit; the radio task reads the FIFO and validates the CRC outside the ISR. | Draft |
| REQ-NUX-075 | The shared **protocol** (frames, version, CRC) lives in `common/protocol/` and compiles into both firmwares. | Draft |
| REQ-NUX-076 | The AcroSched library is built with the **cooperative kernel**, `ACROSCHED_MAX_TASKS = 8`, a **32-bit tick** (`uint32_t`) at 1 kHz, and **no software timers** (periodic task modes cover blink / fail-safe / telemetry cadence). Both firmwares link the same configuration. | Draft |
| REQ-NUX-077 | An independent **hardware watchdog (IWDG)** is refreshed from the scheduler loop via the `acroWatchdogRefresh()` hook (`ACROSCHED_USE_WATCHDOG = 1`). | Draft |
| REQ-NUX-078 | The scheduler **idle / low-power hook** (`ACROSCHED_USE_IDLE_HOOK = 1`) enters `__WFI()` when no task runs in an iteration, to save the 1S logic battery. | Draft |

## 9. Toolchain

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-080 | Build system is **CMSIS-Toolbox 2.13** (`csolution` / `cbuild`). | Draft |
| REQ-NUX-081 | Two compilers are supported: **AC6 6.20.1** and **arm-none-eabi GCC 15.2.1**. | Draft |
| REQ-NUX-082 | Flashing and debugging use **ST-Link (SWD)**. | Draft |

## 10. Optional / future

| ID | Requirement | Status |
|---|---|---|
| REQ-NUX-090 | On-board camera (separate subsystem; F103 cannot process video). | Deferred |
