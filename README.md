# Tetris (Command-Line Version)

[![GCC](https://img.shields.io/badge/GCC-11.5.0-blue?style=flat-square)](https://gcc.gnu.org/)
[![Windows](https://img.shields.io/badge/Windows-10%2F11-green?style=flat-square)](https://www.microsoft.com/windows/)
[![C Standard](https://img.shields.io/badge/C-C99-orange?style=flat-square)](https://en.cppreference.com/w/c/99)

A command-line Tetris game written in plain C. It runs in the Windows console.

## About

This is a complete Tetris implementation for the console. It uses incremental drawing: only changed cells are redrawn instead of the whole screen.

## Features

- **7 tetrominoes** — I, O, T, S, Z, L, J
- **4 rotations per piece**
- **Scoring**:
  - 1 line: 100
  - 2 lines: 300
  - 3 lines: 500
  - 4 lines: 800
- **Levels** — one level every 500 points; speed increases as the level goes up
- **Next piece preview** — shown in the right panel
- **Two control schemes** — arrow keys or WASD
- **Pause** — pause and resume at any time

## Configuration

| Setting | Default | Description |
| ------------ | ------ | ------------------------ |
| `BOARD_WIDTH` | 15 | Board width in cells |
| `BOARD_HEIGHT` | 30 | Board height in cells |
| `BLOCK_SIZE` | 2 | Character width per cell |
| `INIT_SPEED` | 300 | Initial drop interval (ms) |

## Controls

### Movement

| Key | Action |
| ----- | -------- |
| ← / A | Move left |
| → / D | Move right |
| ↑ / W | Rotate |
| ↓ / S | Soft drop |

### Other

| Key | Action |
| ------ | ---------------------- |
| Space | Hard drop |
| P | Pause/resume |
| X | Quit |
| R | Restart (after game over) |

## Build and Run

### Requirements

- Windows
- GCC (or a compatible C compiler)

### Build

```bash
gcc 俄罗斯方块（命令行版）.cpp -o 俄罗斯方块.exe
```

### Run

```bash
俄罗斯方块.exe
```

## Implementation

- Plain C, no external libraries
- Windows console APIs for cursor control, colors, and window size
- Incremental drawing to avoid full redraws
- A `Game` struct holds the game state

## Dependencies

Standard headers used:

- `stdio.h` — input/output
- `stdlib.h` — standard library
- `conio.h` — console input/output
- `windows.h` — Windows API
- `time.h` — timing
- `stdbool.h` — boolean type

---

## Author

**Dr. Ailan Anjuxi (谙弆悕)**

- Email: anjuxi.ME@outlook.com
- SIP: sip:anjuxi@sip.linphone.org
