@echo off
REM ============================================================
REM  Build MyanmarIME.exe  (Windows)
REM ============================================================
REM
REM  Option A — MinGW-w64 (free, recommended for non-developers):
REM    1. Install MSYS2 from https://www.msys2.org
REM    2. In the MSYS2 UCRT64 terminal run:
REM         pacman -S mingw-w64-ucrt-x86_64-gcc
REM    3. Add  C:\msys64\ucrt64\bin  to your PATH.
REM    4. Double-click this build.bat (or run it in a terminal).
REM
REM  Produces a single standalone MyanmarIME.exe — no install needed.
REM ============================================================

g++ main.cpp -o MyanmarIME.exe -municode -mwindows -O2 -static ^
    -lshell32 -lgdi32 -luser32

if %errorlevel%==0 (
    echo.
    echo Built MyanmarIME.exe — double-click it to run.
    echo Toggle Myanmar mode with Ctrl+Alt+M, or via the tray icon.
) else (
    echo.
    echo Build failed. Make sure g++ ^(MinGW-w64^) is on your PATH.
)
pause
