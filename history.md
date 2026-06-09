# CreatiVemu2 Project History & Status

## Project Overview
CreatiVemu2 is a C++ emulator for the **CreatiVision** and **Dick Smith Wizard** 8-bit consoles, utilizing **SDL2** for video, audio, and input handling.

### Core Architecture
- **CPU**: 6502 core (`cpu/cpu6502.cpp`).
- **Video**: TMS9918 VDP implementation (`video/tms9918.cpp`).
- **I/O**: Motorola 6821 PIA handling keyboard, joysticks, and sound chip control (`pia/6821pia.cpp`).
- **Audio**: SN76496 sound generator emulation (`audio/sn76496.cpp`).
- **Memory**: Custom mapping and ROM loading logic (`mem/cvmemory.cpp`).

---

## Technical Issues & Resolved Bugs

### 1. Memory Mirroring & ROM Corruption
- **Problem**: The CPU memory write logic incorrectly mirrored the entire lower 16KB address space, causing RAM writes to overwrite and corrupt game ROM data starting at `0x4000`.
- **Fix**: Restricted mirroring to the base 1KB RAM region (`0x0000-0x03FF`) while preserving access to other I/O registers (VDP/PIA).
- **File**: `cpu/cpu6502.cpp`

### 2. ROM Loading Buffer Overflow
- **Problem**: 18KB ROM cartridges caused a massive buffer overflow during the mirroring/copying phase in the memory manager, reading 14KB past the source buffer end.
- **Fix**: Corrected the loop boundaries and mirroring logic for 18KB ROMs.
- **File**: `mem/cvmemory.cpp`

### 3. Audio Ring Buffer & Thread Safety
- **Problem**: The audio system used a flawed "read backwards" logic and lacked thread safety, leading to buffer overflows, memory corruption, and "clicks" or "garbage sound."
- **Fix**: 
    - Implemented a proper **producer-consumer ring buffer**.
    - Wrapped shared buffer access in `SDL_LockAudio()` / `SDL_UnlockAudio()`.
    - Added underrun handling (outputting silence) to prevent popping.
- **File**: `main.cpp`, `main.h`

### 4. Noise Generator Deadlock
- **Problem**: The white noise generator in `sn76496.cpp` could enter an infinite loop if the time-step became too small or zero, stalling the entire emulator.
- **Fix**: Added a safety break and enforced a minimum period (maximum frequency) for the noise channel.
- **File**: `audio/sn76496.cpp`

### 5. Audio Timing & Decoupling
- **Problem**: Audio generation was tied to VDP scanlines (`Line == 0`), which didn't trigger reliably, causing silent audio.
- **Fix**: Decoupled audio generation from video state; it now runs at a steady 50Hz using a high-resolution timer.
- **File**: `main.cpp`

---

## Current Status & Remaining Challenges

### Missing Tones / Musical Output
As of the end of this session, the emulator produces **noise and bursts** but lacks clear musical tones (square waves).

#### Observations:
- **Register Writes confirmed**: Terminal logs show the game is successfully writing valid frequency data (e.g., `0x07f`, `0x1df`) and volume data (`0x00` for max, `0x0f` for silence).
- **Frequency Formula**: The period calculation was updated to `Period = UpdateStep * RegisterValue * 32`.
- **Volume Table**: Base gain was increased to ensure indexes 1-14 are audible.
- **Inconsistency**: A secondary frequency update path in `sn76496Write` was corrected, but tones remain elusive.

#### Recommendations for Next Session:
1. **Verify `sn76496Update` Loop**: Inspect the square wave generation logic. Check if `R->Output[i]` is toggling correctly and if the `while (R->Count[i] <= 0)` loop is correctly accumulating volume.
2. **Clock Alignment**: Verify if `UpdateStep` (calculated from `SAMPLE_RATE` and chip clock) correctly aligns with the expected frequencies of the 6502-driven sound writes.
3. **DC Offset**: Re-evaluate the sample mapping `(out / STEP) - MAX_OUTPUT` to ensure the waveform is perfectly centered for signed 16-bit output.
4. **Volume Scaling**: Check if `R->Volume[c]` (derived from `VolTable`) is being applied correctly in the final `out` summation.
