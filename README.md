# Myanmar Romanized Keyboard / မြန်မာ Romanized လက်ကွက်

Type Burmese **Unicode** by typing the sounds in Latin letters — e.g. `kya naw`
→ ကျ နော, `mingalarpar` → မင်္ဂလာ… A modern, cross-platform input method inspired
by [WaitZar](https://github.com/yathit/waitzar), but producing clean **Unicode**
(not Zawgyi) so it works with every current app, website and phone.

မြန်မာစာကို အသံထွက်အတိုင်း အင်္ဂလိပ်စာလုံးနဲ့ ရိုက်ပြီး **Unicode** မြန်မာစာ ရိုက်နိုင်ပါတယ်။
WaitZar ကို အခြေခံပေမဲ့ ခုခေတ် system တွေနဲ့ ကိုက်ညီအောင် Unicode ထွက်အောင် ရေးထားပါတယ်။

---

## What's here / ပါဝင်တာ

```
core/
  romanizer.py     reference engine + tests   (python3 core/romanizer.py)
  romanizer.h      C++ engine (used by Windows)
  ROMANIZATION.md  how to type — the full romanization table
macos/             macOS input method (Swift + InputMethodKit)
windows/           Windows helper app (C++ keyboard hook, single .exe)
```

All three engines (Python, Swift, C++) implement the **same** algorithm and pass
the same tests, so typing behaves identically everywhere.

## How to type / ရိုက်နည်း

See **[core/ROMANIZATION.md](core/ROMANIZATION.md)**. Quick examples:

| type | result |
|------|--------|
| `ka`     | က |
| `kaa:`   | ကား |
| `kya`    | ကျ |
| `ko`     | ကို |
| `kaung`  | ကောင် |
| `kaN`    | ကံ |
| `mng_galar` | မင်္ဂလာ |
| `bud_dha`   | ဗုဒ္ဓ |

Type one syllable, then **space** to commit. **Backspace** edits before committing.

---

## Install

### 🍎 macOS

```bash
cd macos
./build.sh      # compiles MyanmarIME.app (needs Xcode Command Line Tools)
./install.sh    # copies it to ~/Library/Input Methods
```
Then: **System Settings → Keyboard → Text Input → Input Sources → Edit → +**,
add **“Myanmar Romanized”**. Switch to it with Ctrl+Space.

### 🪟 Windows

```bat
cd windows
build.bat       :: compiles MyanmarIME.exe with MinGW-w64 (see build.bat header)
```
Double-click **MyanmarIME.exe**. Toggle Myanmar mode with **Ctrl+Alt+M** or the
tray icon. No installer, no admin rights needed.

See [windows/README.md](windows/README.md) and [macos/README.md](macos/README.md)
for details.

---

## Status

- ✅ Core conversion engine — tested, identical on all 3 platforms
- ✅ macOS input method — builds & installs on this machine
- ✅ Windows helper app — complete source (build on a Windows PC with MinGW)
- ✅ Stacked consonants / kinzi (e.g. မင်္ဂလာ, ဗုဒ္ဓ) via the `_` stacker

Credit: romanization approach inspired by the **WaitZar** project (yathit/waitzar).
