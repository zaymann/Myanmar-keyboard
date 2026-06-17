#!/bin/bash
# Build MyanmarIME.app from source using the Swift command-line compiler.
# Requires: Xcode Command Line Tools (xcode-select --install). Full Xcode NOT required.
set -euo pipefail

cd "$(dirname "$0")"
SRC="MyanmarIME"
APP="build/MyanmarIME.app"
BIN="$APP/Contents/MacOS/MyanmarIME"

echo "==> Cleaning"
rm -rf build
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"

echo "==> Compiling Swift sources"
swiftc -O \
    "$SRC/Romanizer.swift" \
    "$SRC/InputController.swift" \
    "$SRC/main.swift" \
    -framework Cocoa \
    -framework InputMethodKit \
    -o "$BIN"

echo "==> Assembling bundle"
cp "$SRC/Info.plist" "$APP/Contents/Info.plist"

echo "==> Ad-hoc code signing"
codesign --force --deep --sign - "$APP"

echo ""
echo "Built: $APP"
echo "Install with:  ./install.sh"
