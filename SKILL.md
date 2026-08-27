---
name: revenant-elegy-macos
version: 1.1.0
description: Installs and configures the Revenant Elegy Ragnarok Online client (Gepard Shield 3.0 + Themida) on an Apple Silicon Mac using Homebrew, Whisky, and a wine-7.7 runtime. Covers Rosetta x87 byte-patches, the Setup.exe poll, OptionInfo.lua display config, a replacement wtsapi32.dll, and the launcher apps. Trigger on "set up Revenant Elegy on Mac", "install the Elegy client", or when this file is handed to a fresh session.
---

# Installing Revenant Elegy on Apple Silicon

Fourteen steps. Run them in order and verify each one before moving on.

If you are an AI agent executing this: stop and report if a verification fails, rather than
continuing. Do not modify `client.exe` under any circumstances (step 9 explains why).

**Before you start**, you need:

- An Apple Silicon Mac. Verify with `uname -m` (expect `arm64`).
- The client archive from the server's official download page.
- About 15 GB free.

## Progress table

Track your position. Report it if you hand off.

| Step | What | Done |
|---|---|---|
| 1 | Homebrew and tools | |
| 2 | Rosetta 2 | |
| 3 | Whisky.app | |
| 4 | wine-7.7 runtime | |
| 5 | Bottle | |
| 6 | Extract client | |
| 7 | x87 byte-patches | |
| 8 | Setup.exe | |
| 9 | OptionInfo.lua | |
| 10 | wtsapi32.dll | |
| 11 | Registry | |
| 12 | First launch | |
| 13 | Launcher apps | |
| 14 | Verify | |

## Settings used below

| Name | Value |
|---|---|
| Bottle name | `ro` |
| Game directory | `~/Games/Revenant Elegy` |
| Window size | 1600x900 |

Keep the game in `~/Games`. If you put it in Desktop or Documents, iCloud can move the
files mid-install and Wine will report `c0000135`.

---

## Step 1: Homebrew and tools

```sh
command -v brew || /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install winetricks p7zip icoutils mingw-w64
```

`icoutils` is for the launcher icon in step 13. `mingw-w64` builds the DLL in step 10.

Homebrew may ask for your password the first time. Type it yourself.

## Step 2: Rosetta 2

```sh
pgrep -q oahd && echo "Rosetta present" || softwareupdate --install-rosetta --agree-to-license
```

## Step 3: Whisky.app

The Homebrew cask was disabled in April 2026. It exits successfully and installs nothing,
so use the GitHub release:

```sh
curl -fL -o /tmp/Whisky.zip https://github.com/Whisky-App/Whisky/releases/download/v2.3.5/Whisky.zip
ditto -xk /tmp/Whisky.zip /tmp/whisky-x
ditto /tmp/whisky-x/Whisky.app /Applications/Whisky.app
xattr -dr com.apple.quarantine /Applications/Whisky.app
```

Verify:

```sh
defaults read /Applications/Whisky.app/Contents/Info.plist CFBundleShortVersionString
```

Expect `2.3.5`.

## Step 4: wine-7.7 runtime

Use this version. Wine 10 and 11 use the experimental wow64 path, which cannot grow a
Gepard thread's stack. The client will load the entire game and then crash. Do not upgrade
it later.

Whisky's own runtime CDN returns 404, so download the mirror:

```sh
SUP="$HOME/Library/Application Support/com.isaacmarovitz.Whisky"
curl -fL -C - --retry 8 --retry-all-errors -o ~/Downloads/WhiskyWine-Libraries.zip \
  https://github.com/pickanddrop/revenant-elegy-whisky-macOS-setup/releases/download/runtime-wine-7.7/WhiskyWine-Libraries.zip

shasum -a 256 ~/Downloads/WhiskyWine-Libraries.zip
# expect dedc05a05e6b4635173ac7e9c146d0703389a695dac7d9f059911cbaf1ec537a

cd ~/Downloads && ditto -xk WhiskyWine-Libraries.zip . && mkdir -p "$SUP"
tar -xzf Libraries.tar.gz -C "$SUP"
chmod -R u+w "$SUP/Libraries" || true
xattr -dr com.apple.quarantine "$SUP/Libraries" || true
```

Whisky needs a structured version plist. A plain string is silently rejected:

```sh
SUP="$HOME/Library/Application Support/com.isaacmarovitz.Whisky"
cat > "$SUP/Libraries/WhiskyWineVersion.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict><key>version</key><dict>
 <key>major</key><integer>2</integer>
 <key>minor</key><integer>5</integer>
 <key>patch</key><integer>0</integer>
 <key>preRelease</key><string></string>
 <key>build</key><string></string>
</dict></dict>
</plist>
PLIST
plutil -lint "$SUP/Libraries/WhiskyWineVersion.plist"
"$SUP/Libraries/Wine/bin/wine64" --version
ls "$SUP/Libraries/Wine/lib/wine/" | grep x86_32on64
```

The last two commands must print `wine-7.7` and `x86_32on64-unix`. If either is wrong,
stop here. Everything downstream depends on them.

## Step 5: Create the bottle

```sh
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"
"$WHISKY" create ro
UUID=$(ls -d ~/Library/Containers/com.isaacmarovitz.Whisky/Bottles/*/ | head -1 | xargs basename)
META="$HOME/Library/Containers/com.isaacmarovitz.Whisky/Bottles/$UUID/Metadata.plist"
plutil -replace wineConfig.windowsVersion -string "win10" "$META"
plutil -replace wineConfig.avxEnabled     -bool   true    "$META"
eval "$("$WHISKY" shellenv ro)"
WINEMSYNC=0 wine64 wineboot -u
echo "$UUID"
```

Write down that UUID. Step 10 needs it.

`whisky list` prints a bottle path that does not exist. That is a display bug in the CLI.
Use `~/Library/Containers/com.isaacmarovitz.Whisky/Bottles/<UUID>`.

Leave DXVK's DLLs where they are. Step 12 overrides them; deleting them causes `c0000135`.

## Step 6: Extract the client

```sh
unrar t "<archive>.rar"
mkdir -p "$HOME/Games/Revenant Elegy"
cd "$HOME/Games/Revenant Elegy" && unrar x -o+ "<archive>.rar" .
```

Test the archive first. A truncated download wastes the next several steps.

You should now have `client.exe` (the game), `Revenant Elegy.exe` (the patcher),
`opensetup.exe`, `gepard.dll`, `dinput.dll`, and several `.grf` files.

## Step 7: Patch opensetup.exe

Rosetta cannot translate some x87 `FCOM` and `FCOMP` encodings, so `opensetup.exe` crashes
with "Unhandled illegal instruction".

Scan for the sites rather than using offsets from anywhere else. They differ between
builds:

```sh
./tools/scan-x87.py "$HOME/Games/Revenant Elegy"/*.exe
```

On the tested build, `opensetup.exe` had two sites and `client.exe` had none. Apply the
patches the scanner prints, with a backup:

```sh
G="$HOME/Games/Revenant Elegy"
cp "$G/opensetup.exe" "$G/opensetup.exe.orig-backup"
```

Then patch each site, replacing the offsets and bytes with what the scanner reported:

```sh
python3 - <<'PY'
p = "<full path to opensetup.exe>"
sites = [(0x21E39, 'dcd8dfe0', 'ddd8b440'), (0x2C0CD, 'dcd0dfe0', 'd8d0dfe0')]
d = bytearray(open(p, 'rb').read())
for off, old, new in sites:
    assert d[off:off+4] == bytes.fromhex(old), f"mismatch at {hex(off)}, rescan"
    d[off:off+4] = bytes.fromhex(new)
open(p, 'wb').write(d)
print("patched")
PY
cmp -l "$G/opensetup.exe.orig-backup" "$G/opensetup.exe" | wc -l
```

The byte count should match what you changed (4 for the two sites above). If the assertion
fails, your build differs. Rescan and use the new offsets.

## Step 8: Create Setup.exe

The client calls `GetFileAttributesW("Setup.exe")` about 12,000 times a second and never
opens a window until that file exists. It only ships `opensetup.exe`:

```sh
G="$HOME/Games/Revenant Elegy"
cp "$G/opensetup.exe" "$G/Setup.exe"
```

Copy it after step 7 so you get the patched version.

## Step 9: Fix OptionInfo.lua

The client ships `DX9DEVICENAME = ".DISPLAY1"`, which is not a real display device. D3D9
returns NULL, the client uses it anyway, and crashes with `page fault on write access to
00000128`.

```sh
./tools/patch-optioninfo.py "$HOME/Games/Revenant Elegy/savedata/OptionInfo.lua" 1600 900
grep -E 'DX9DEVICENAME|RENDERSYSTEM|ISFULLSCREEN' "$HOME/Games/Revenant Elegy/savedata/OptionInfo.lua"
```

In the file, `DX9DEVICENAME` must read as four backslashes, a dot, then two backslashes. If
you see half that many, a shell collapsed them; use a quoted heredoc or run the tool.

**Do not modify `client.exe`.** Gepard hashes its headers, and any change produces "Game
EXE file corrupted!". This is worth stating plainly because one edit genuinely works:
raising `SizeOfStackCommit` in the PE header stops the stack crash on newer Wine. It also
trips the integrity check immediately. Use wine-7.7 instead.

## Step 10: Build wtsapi32.dll

wine-7.7 stubs `WTSEnumerateProcessesA` and returns NULL, which Gepard then reads as a
process list.

```sh
cd tools/wtsapi32
make
make verify
make install BOTTLE=<the UUID from step 5>
```

`make verify` should report 52 exports. All of them are required; a missing one produces
"The procedure entry point WTSSendMessageW could not be located".

This was added before the real cause of the final crash was found, and never tested in
isolation, so it may not be strictly necessary. It is harmless and part of the tested
configuration.

## Step 11: Registry

```sh
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"
eval "$("$WHISKY" shellenv ro)"
WINEMSYNC=0 wine64 reg add "HKCU\\Software\\Wine\\Drivers" /v Audio /d coreaudio /f
WINEMSYNC=0 wine64 reg add "HKCU\\Software\\Wine\\WineDbg" /v ShowCrashDialog /t REG_DWORD /d 0 /f
```

## Step 12: First launch

```sh
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"
eval "$("$WHISKY" shellenv ro)"
export WINEMSYNC=0 WINEESYNC=0
export WINEDLLOVERRIDES="d3d9,dxgi,d3d10core,d3d11=b;msvcp140,vcruntime140,concrt140,vccorlib140=n,b;dinput=n,b;wtsapi32=n"
export WINE_CPU_TOPOLOGY=4:0,1,2,3
cd "$HOME/Games/Revenant Elegy" && wine64 client.exe 1rag1
```

What each override does:

| Setting | Without it |
|---|---|
| `WINEMSYNC=0 WINEESYNC=0` | Gepard's threads deadlock and spin at 87% CPU |
| `d3d9,dxgi,d3d10core,d3d11=b` | DXVK returns a NULL D3D9 device and the client crashes |
| `msvcp140,...=n,b` | Gepard rejects Wine's builtin MSVC runtimes |
| `dinput=n,b` | Wine's builtin dinput replaces the game's. `=n` alone is not enough |
| `wtsapi32=n` | Step 10's DLL is ignored |
| `WINE_CPU_TOPOLOGY` | Gepard's CPU detection becomes unstable |

Do not reinstate DXVK to "fix" graphics. This runtime's MoltenVK lacks
`bufferDeviceAddress` and `timelineSemaphore`, which every DXVK build requires, so
`vkCreateDevice` fails and the NULL device it returns crashes the client at an address
inside `gepard.dll`. That looks like an anti-cheat block and is not one.

A healthy launch: memory climbs past 500 MB, a window appears within about 20 seconds, and
CPU settles into a render loop. No network connections before you log in is normal.

## Step 13: Launcher apps

```sh
./tools/build-apps.sh "$HOME/Games/Revenant Elegy"
```

This builds two apps in `/Applications`, both carrying the environment above. Use
**Revenant Elegy Patcher** normally, since it checks for patches first. The other one skips
the patcher and starts the game directly.

Drag either to the Dock.

## Step 14: Verify

Confirm all of these before calling it done:

- `client.exe` is unmodified: `shasum -a 256` matches the copy inside the archive.
- The game reaches the login screen.
- Sound plays. If not, type `/bgm` and `/sound` in chat.
- Double-clicking the launcher app works from a cold start.

Two things to remember afterwards:

- The client rewrites `OptionInfo.lua` when it exits. Quit fully before editing that file,
  or your changes are overwritten.
- Never modify `client.exe`, including after a patch.
