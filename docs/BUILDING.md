# Building

## Requirements

- Windows 10 or 11 x64
- Visual Studio 2022 or newer with Desktop development with C++
- CMake 3.25 or newer
- Ninja (the Visual Studio bundled copy is sufficient)

## Build and test

From an x64 Visual Studio developer shell:

```powershell
cmake -S . -B build-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ninja
ctest --test-dir build-ninja --output-on-failure
```

Or run:

```powershell
./scripts/build.ps1
```

Expected outputs:

- `build-ninja/TechmaniaNfcHook.dll`
- `build-ninja/TechmaniaNfcProbe.exe`
- `build-ninja/techmania_nfc_tests.exe`

## Verify the artifact

Run native tests, then probe real hardware:

```powershell
./build-ninja/techmania_nfc_tests.exe
./build-ninja/TechmaniaNfcProbe.exe --seconds 30
```

The probe intentionally redacts card IDs. A healthy connected reader reports
`reader connected`; tapping and removing a card reports only its kind and
presence transition.

