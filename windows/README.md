# Myanmar Romanized IME — Windows

A single-executable helper app (like the original WaitZar). It installs a
system-wide keyboard hook: when Myanmar mode is ON, your Latin typing is
captured, shown in a small popup, and the converted Burmese Unicode is typed
into whatever app has focus.

- **No installer, no admin rights, no COM registration** — just run the `.exe`.
- Toggle Myanmar mode: **Ctrl+Alt+M** (or the tray-icon menu).

## Build

You need a C++ compiler. The easiest free option is **MinGW-w64** via MSYS2:

1. Install [MSYS2](https://www.msys2.org).
2. In the **UCRT64** terminal:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```
3. Add `C:\msys64\ucrt64\bin` to your PATH.
4. Run **`build.bat`** (double-click or from a terminal).

This produces a standalone **`MyanmarIME.exe`**.

> Alternative (MSVC / Visual Studio Developer Command Prompt):
> ```bat
> cl /EHsc /utf-8 /DUNICODE /D_UNICODE main.cpp /link user32.lib gdi32.lib shell32.lib /SUBSYSTEM:WINDOWS
> ```

## Use

1. Double-click `MyanmarIME.exe`. A tray icon appears (Myanmar IME: off).
2. Press **Ctrl+Alt+M** to turn Myanmar mode ON.
3. Type romanized Burmese in any app (Word, browser, chat…). A popup shows the
   live conversion. Press **space / enter** to commit.
4. Press **Ctrl+Alt+M** again to turn it off.

See [../core/ROMANIZATION.md](../core/ROMANIZATION.md) for how to type.

## Requirements

- Windows 10 or later (ships with the **Myanmar Text** Unicode font).
- On older Windows, install a Myanmar Unicode font for correct display.

## Files

| file | purpose |
|------|---------|
| `main.cpp`           | keyboard hook, popup, tray, Unicode injection |
| `../core/romanizer.h`| conversion engine (shared with all platforms) |
| `build.bat`          | one-command MinGW build |

## How it differs from a TSF service

A full TSF (Text Services Framework) IME would integrate slightly more deeply,
but requires COM registration and admin rights. The hook approach matches
WaitZar's original "single executable" design and is far simpler to install —
exactly the trade-off WaitZar made.
