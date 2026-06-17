#!/bin/bash
# Install MyanmarIME.app into the user's Input Methods folder.
set -euo pipefail
cd "$(dirname "$0")"

APP="build/MyanmarIME.app"
DEST="$HOME/Library/Input Methods"

if [ ! -d "$APP" ]; then
    echo "Not built yet. Run ./build.sh first."
    exit 1
fi

echo "==> Installing to $DEST"
mkdir -p "$DEST"
rm -rf "$DEST/MyanmarIME.app"
cp -R "$APP" "$DEST/"

echo "==> Registering"
# Nudge the system to pick up the new input method.
"$DEST/MyanmarIME.app/Contents/MacOS/MyanmarIME" &>/dev/null &
sleep 1

cat <<'EOF'

Installed.  Next steps (one time):
  1. Open  System Settings > Keyboard > Text Input > Input Sources > Edit…
  2. Click  +  (bottom-left).
  3. Search "Myanmar Romanized" (or look under Burmese), Add it.
  4. Switch to it with the input-source menu (top-right) or Ctrl+Space.

If it does not appear, log out and back in once.
EOF
