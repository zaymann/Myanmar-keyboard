# Myanmar Romanized IME — macOS

A native macOS input method (InputMethodKit). Type romanized Burmese in **any**
app; the live conversion shows as underlined marked text and commits on
space / return / punctuation.

## Build

Requires **Xcode Command Line Tools** (full Xcode not needed):
```bash
xcode-select --install   # if not already installed
./build.sh
```
This compiles `build/MyanmarIME.app`.

## Install

```bash
./install.sh
```
Then enable it once:
1. **System Settings → Keyboard → Text Input → Input Sources → Edit…**
2. Click **+**, search **“Myanmar Romanized”** (under Burmese), **Add**.
3. Switch input sources with the menu-bar icon or **Ctrl+Space**.

If it doesn't appear immediately, log out and back in once.

## Files

| file | purpose |
|------|---------|
| `MyanmarIME/Romanizer.swift`       | conversion engine (port of `core/romanizer.py`) |
| `MyanmarIME/InputController.swift` | IMKInputController — buffers input, shows preedit, commits |
| `MyanmarIME/main.swift`            | starts the IMKServer |
| `MyanmarIME/Info.plist`            | input-method registration |
| `build.sh` / `install.sh`          | build & install scripts |

## Notes

- The app is ad-hoc code-signed by `build.sh`. macOS allows ad-hoc-signed input
  methods from your own machine.
- Uninstall: remove `~/Library/Input Methods/MyanmarIME.app` and remove the
  input source in System Settings.
