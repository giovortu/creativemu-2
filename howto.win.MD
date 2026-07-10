# Windows build how-to

This project now builds on Windows with **CMake + Visual Studio 2022 + vcpkg**.

## 1. Prerequisites

- Visual Studio 2022 with the **Desktop development with C++** workload
- CMake 3.21+
- Git

## 2. Bootstrap vcpkg

Run these commands in PowerShell:

```powershell
git clone https://github.com/microsoft/vcpkg "$env:USERPROFILE\vcpkg"
& "$env:USERPROFILE\vcpkg\bootstrap-vcpkg.bat" -disableMetrics
```

## 3. Install dependencies from the manifest

From the repository root:

```powershell
& "$env:USERPROFILE\vcpkg\vcpkg.exe" install --triplet x64-windows-static
```

The manifest installs:

- SDL2
- zlib

## 4. Configure the project

Use a fresh build directory the first time you configure, or any time you previously configured without vcpkg:

```powershell
Remove-Item -Recurse -Force .\build -ErrorAction SilentlyContinue

cmake --preset windows-vs2022
```

The project also auto-detects vcpkg at:

- `$env:VCPKG_ROOT`
- `$env:USERPROFILE\vcpkg`

So a manual configure also works:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

## 5. Build

```powershell
cmake --build --preset windows-release
```

The executable is generated here:

```text
build\Release\CreatiVemu2.exe
```

During the build, the `bios`, `roms`, and `snapshots` folders are prepared next to the executable so the default relative paths keep working.

## 6. Run

Run from the executable directory:

```powershell
Set-Location .\build\Release
.\CreatiVemu2.exe roms\autochas.rom
```

To force an explicit BIOS file:

```powershell
.\CreatiVemu2.exe roms\autochas.rom bios\Biosdsw.rom
```

## Notes

- The root CMake entry file is `CMakeLists.txt`.
- If you see `Could not find SDL2Config.cmake`, delete `build` and configure again so CMake picks up the vcpkg toolchain on the first configure.
- The current codebase still emits a few legacy compiler warnings on MSVC, but the Release build completes successfully.
