#!/bin/zsh
# Build the two double-clickable .app bundles, with the game's own icon.
#
#   ./build-apps.sh "$HOME/Games/Revenant Elegy"
#
# Icon: the only icon in either binary is 48x48, so it is upscaled into a full
# .iconset. macOS would otherwise show a generic document icon.
set -e
GAME="${1:-$HOME/Games/Revenant Elegy}"
[ -d "$GAME" ] || { echo "game dir not found: $GAME"; exit 1; }
command -v wrestool >/dev/null || { echo "need: brew install icoutils"; exit 1; }

WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT
wrestool -x -t 14 -o "$WORK" "$GAME/Revenant Elegy.exe" 2>/dev/null || true
ICO="$(ls "$WORK"/*.ico 2>/dev/null | head -1)"
[ -n "$ICO" ] || { echo "no icon found in Revenant Elegy.exe"; exit 1; }
icotool -x -o "$WORK" "$ICO"
PNG="$(ls "$WORK"/*.png | head -1)"

mkdir -p "$WORK/RE.iconset"
for s in 16 32 128 256 512; do
  sips -z $s $s "$PNG" --out "$WORK/RE.iconset/icon_${s}x${s}.png" >/dev/null
done
for pair in "16 32" "32 64" "128 256" "256 512" "512 1024"; do
  set -- $pair
  sips -z $2 $2 "$PNG" --out "$WORK/RE.iconset/icon_$1x$1@2x.png" >/dev/null
done
iconutil -c icns "$WORK/RE.iconset" -o "$WORK/RevenantElegy.icns"

make_app() {  # $1=app name  $2=binary name  $3=target exe  $4=bundle id
  APP="/Applications/$1.app"
  rm -rf "$APP"; mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
  cp "$WORK/RevenantElegy.icns" "$APP/Contents/Resources/RevenantElegy.icns"
  cat > "$APP/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleName</key><string>$1</string>
	<key>CFBundleIdentifier</key><string>$4</string>
	<key>CFBundleExecutable</key><string>$2</string>
	<key>CFBundleIconFile</key><string>RevenantElegy</string>
	<key>CFBundlePackageType</key><string>APPL</string>
	<key>CFBundleShortVersionString</key><string>1.0</string>
	<key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
PLIST
  cat > "$APP/Contents/MacOS/$2" <<SCRIPT
#!/bin/zsh
GAME="$GAME"
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"
[ -x "\$WHISKY" ] || { osascript -e 'display alert "$1" message "Whisky.app is missing."'; exit 1; }
[ -d "\$GAME" ]   || { osascript -e 'display alert "$1" message "Game folder not found."'; exit 1; }
eval "\$("\$WHISKY" shellenv ro)" || { osascript -e 'display alert "$1" message "Bottle \"ro\" not found."'; exit 1; }
export WINEMSYNC=0 WINEESYNC=0
export WINEDLLOVERRIDES="d3d9,dxgi,d3d10core,d3d11=b;msvcp140,vcruntime140,concrt140,vccorlib140=n,b;dinput=n,b;wtsapi32=n"
export WINE_CPU_TOPOLOGY=4:0,1,2,3
# a stale wineserver in the wrong sync mode blocks startup silently
if pgrep -f wineserver >/dev/null 2>&1; then WINEMSYNC=0 wineserver -k >/dev/null 2>&1; sleep 2; fi
$5
cd "\$GAME" || exit 1
exec wine64 $3
SCRIPT
  chmod +x "$APP/Contents/MacOS/$2"
  touch "$APP"
  /System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister -f "$APP" 2>/dev/null || true
  echo "built $APP"
}

# two patchers running makes the START button unclickable, so the patcher app clears them
make_app "Revenant Elegy Patcher" "revenant-elegy-patcher" '"Revenant Elegy.exe"' \
         "com.local.revenantelegy.patcher" 'pkill -f "Revenant Elegy.exe" >/dev/null 2>&1; sleep 1'
make_app "Revenant Elegy" "revenant-elegy" 'client.exe 1rag1' \
         "com.local.revenantelegy" ''
killall Dock 2>/dev/null || true
echo "done - drag either app to the Dock"
