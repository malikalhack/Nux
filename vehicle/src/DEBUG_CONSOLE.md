/**
 * @file    DEBUG_CONSOLE.md
 * @brief   Debug console command reference for Nux vehicle firmware.
 */

# Nux Vehicle — Debug Console

The debug console allows real-time control of the vehicle via UART at **115200 baud**.
It is completely independent from the radio link and is useful for testing motor,
steering, lighting, and fail-safe logic without involving the transmitter.

## Configuration

The debug console is controlled by:
- **BSP**: `BSP_DEBUG_CONSOLE` in `vehicle/bsp/inc/bsp.h` (default: 1)
- **Debug module**: `DEBUG_CONSOLE_ENABLED` in `vehicle/src/debug.h` (reads from BSP setting)

To **disable** the debug console entirely and remove it from the build:
1. Set `BSP_DEBUG_CONSOLE` to `0` in `vehicle/bsp/inc/bsp.h`
2. Rebuild; the debug code is completely removed by the preprocessor

## Command Syntax

All commands are case-sensitive, separated by spaces, and terminated with `Enter`.

### 1. Motor Command

```
motor <throttle> <direction> <brake>
```

- **throttle**: Signed integer in range `[-1000, +1000]`
  - Negative = reverse direction
  - 0 = idle
  - Positive = forward direction
- **direction**: Unsigned integer, `[0, 1, 2]`
  - `0` = idle (motor disabled)
  - `1` = forward
  - `2` = reverse
- **brake**: Unsigned integer, `[0, 1]`
  - `0` = no active braking
  - `1` = active braking (motor shorts through H-bridge, dissipates energy)

**Examples:**
```
motor 500 1 0     // 50% throttle forward, no braking
motor -750 2 0    // 75% throttle reverse, no braking
motor 0 0 1       // Idle with active brake
```

### 2. Steering Command

```
steer <angle>
```

- **angle**: Signed integer in range `[-1000, +1000]`
  - `-1000` = full left
  - `0` = center
  - `+1000` = full right

**Examples:**
```
steer 0           // Center
steer -500        // 50% left
steer 750         // 75% right
```

### 3. Lighting Command

```
light <headlights> <sidelights> <reverse> <left_turn> <right_turn>
```

All parameters are `[0, 1]` (off/on).

**Examples:**
```
light 1 1 0 0 0   // Headlights + sidelights on
light 0 0 1 0 0   // Reverse light on
light 0 0 0 1 0   // Left turn signal on
light 0 0 0 0 1   // Right turn signal on
```

### 4. Link Loss Simulation

```
loslink <enable>
```

- **enable**: `[0, 1]`
  - `0` = disable link loss simulation
  - `1` = enable link loss simulation (triggers fail-safe braking)

This command **does not affect** the radio link itself. Instead, it sets a flag
that the fail-safe logic can check to simulate the loss of 3 consecutive radio
frames. The vehicle should:
- Stop accepting radio commands
- Apply active braking
- Continue to do so until `loslink 0` is sent

**Examples:**
```
loslink 1         // Simulate link loss → vehicle should brake
loslink 0         // End simulation → vehicle should resume normal operation
```

### 5. State Dump

```
state
```

Prints the current value of all command structures (motor, steering, lights, link loss).

### 6. Help

```
help
```

Prints the command menu and usage examples.

## Communication Details

- **Baud rate**: 115200 8N1
- **Port**: USART1 on STM32F103C8 (PA9=TX, PA10=RX)
- **Echo**: Commands are echoed back as you type
- **Backspace**: Supported (`\x08` or `\x7F`)
- **Buffer size**: 64 characters per command

## Testing Workflow

### Test 1: Motor Control
```
motor 300 1 0     // Throttle forward
motor 0 0 0       // Idle
```
Expected: Motor spins forward, then stops.

### Test 2: Reverse + Brake
```
motor -400 2 0    // Throttle reverse
motor 0 0 1       // Stop with active braking
```
Expected: Motor spins in reverse, then active braking engages.

### Test 3: Steering
```
steer 500         // Turn right
steer 0           // Center
steer -500        // Turn left
```
Expected: Servo moves to commanded positions.

### Test 4: Fail-Safe
```
motor 500 1 0     // Throttle forward
loslink 1         // Simulate link loss
```
Expected: Motor should immediately begin active braking (or ramp to idle).

```
loslink 0         // Resume
motor 500 1 0     // Throttle forward again
```
Expected: Motor should respond normally again.

### Test 5: Lighting
```
light 1 1 1 1 1   // All lights on
light 0 0 0 0 0   // All lights off
state             // Verify state
```
Expected: Lights toggle, state reflects the changes.

## Implementation Notes

- The debug console is **non-blocking**: `debugPoll()` is called from the scheduler
  or main loop and processes one character per call.
- Commands are **independent** from radio input: both can work simultaneously,
  but the radio link takes precedence in the final implementation.
- **Future enhancements** could include:
  - Telemetry readback (battery voltage, current, temperature)
  - Parameter tuning (PID gains, rate limits, endpoints)
  - Oscilloscope-style data logging over UART

## No-op Mode

When `BSP_DEBUG_CONSOLE = 0`:
- All `debug*()` function calls become empty inlines
- No UART reception code is generated
- No command parsing code is generated
- Total code/RAM overhead: **~0 bytes**

---

Example: Building for production with debug removed
```bash
# Edit vehicle/bsp/inc/bsp.h
# Change: #define BSP_DEBUG_CONSOLE   1
# To:     #define BSP_DEBUG_CONSOLE   0

# Rebuild
cbuild vehicle.cproject.yml --configuration "vehicle.Release+STM32F103C8"
```
