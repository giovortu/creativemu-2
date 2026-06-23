# Building CreatiVemu2

This guide provides instructions for building the CreatiVemu2 emulator on Linux and Windows.

## 🐧 Linux

### 1. Install Dependencies
You need a C++ compiler, `make`, and the SDL2 development libraries.

**On Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev
```

**On Fedora:**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install SDL2-devel
```

**On Arch Linux:**
```bash
sudo pacman -S base-devel sdl2
```

### 2. Build
From the root directory of the project, simply run:
```bash
make
```

### 3. Install (Optional)
To install the binary to `/usr/local/bin`:
```bash
sudo make install
```

---

## 🪟 Windows

### 1. Install Toolchain
You need a MinGW-w64 environment. We recommend using **MSYS2** or a standalone **MinGW-w64** distribution.

1.  Install **MinGW-w64**.
2.  Ensure `g++` and `mingw32-make` are in your system PATH.

### 2. Setup SDL2
1.  Download the **SDL2 Development Libraries** (MinGW version) from the [official SDL website](https://www.libsdl.org/download-2.0.php).
2.  Extract the files to a directory on your computer (e.g., `C:/SDL2`).

### 3. Configure Makefile.win
Open `Makefile.win` in a text editor and update the `SDL_PATH` variable to point to your SDL2 installation folder:

```makefile
# Change this to the actual path where you extracted SDL2
SDL_PATH = C:/SDL2
```

### 4. Build
Open a terminal (CMD or PowerShell) in the project root and run:
```cmd
mingw32-make
```
*Note: If your make command is named `make` instead of `mingw32-make`, use `make`.*

---

## 🚀 Running the Emulator
Once built, you can run the emulator by providing a ROM file:

**Linux:**
```bash
./bin/cvemu2 roms/tennis.rom
```

**Windows:**
```cmd
CvEmu2.exe roms/tennis.rom
```
*Note: Ensure the `SDL2.dll` is in the same folder as the executable on Windows.*
