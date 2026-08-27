---
name: revenant-elegy-macos
version: 1.0.0
description: Installs and configures the Revenant Elegy Ragnarok Online client (Gepard Shield 3.0 + Themida/WinLicense) on an Apple Silicon Mac via Homebrew + Whisky + a manually-sourced wine-7.7 runtime. Covers Rosetta x87 byte-patches, the Setup.exe poll, OptionInfo.lua display config, a replacement wtsapi32.dll, and two double-clickable .app bundles. Trigger on "set up Revenant Elegy on Mac", "install the Elegy client", or when this file is handed to a fresh session.
---

# Revenant Elegy on Apple Silicon

Read this file top to bottom before running anything. It is written for an AI agent to
execute, stopping for the user's approval at each phase. A human can follow it directly.

## 0. Operating principles

1. **Verify with real command output, not exit codes.** `exit 0` is not proof a cask
   installed anything, and silence after a patch is not proof it applied.
2. **After any binary patch, byte-diff against a backup** (`cmp -l`). Prove the patch did
   only what was intended.
3. **Never modify `client.exe`.** Gepard hashes its PE header. See Step 9.
4. **Angle-bracket placeholders** (`<BOTTLE>`, `<GAME_DIR>`) are not shell variables.
   Substitute real resolved values before writing anything that runs later.
5. **Re-derive `$GAME_DIR` / `$WHISKY` at the top of every block.** Blocks run in separate
   shells.
6. If a step's verification fails, **stop and report** — do not continue to the next step.

## 1. Progress table

Keep this updated as you go; report it when handing back.

| Step | What | Status | Notes |
|---|---|---|---|
| 1 | Homebrew + tools | ☐ | |
| 2 | Rosetta 2 | ☐ | |
| 3 | Whisky.app 2.3.5 | ☐ | |
| 4 | wine-7.7 runtime | ☐ | |
| 5 | Bottle `ro` | ☐ | |
| 6 | Extract client | ☐ | |
| 7 | x87 byte-patches | ☐ | |
| 8 | `Setup.exe` | ☐ | |
| 9 | `OptionInfo.lua` | ☐ | |
| 10 | `wtsapi32.dll` | ☐ | |
| 11 | Registry | ☐ | |
| 12 | First launch | ☐ | |
| 13 | `.app` bundles | ☐ | |

## 2. Parameters

| Name | Value | Notes |
|---|---|---|
| `BOTTLE` | `ro` | Server-neutral; reusable for other RO clients |
| `GAME_DIR` | `~/Games/Revenant Elegy` | **Must not** be under iCloud (Desktop/Documents) |
| `WIDTH`×`HEIGHT` | `1600`×`900` | Windowed. Must fit the display's *logical* resolution |
| `WHISKY` | `/Applications/Whisky.app/Contents/Resources/WhiskyCmd` | |

## 3. Pre-flight

```sh
sw_vers; uname -m                      # expect arm64
df -h / | tail -1                      # need ~15 GB free
ls -d "$HOME/Games" 2>/dev/null || mkdir -p "$HOME/Games"
```

Detect existing state before installing anything — this skill is re-runnable:

```sh
ls -d /Applications/Whisky.app 2>/dev/null
"$HOME/Library/Application Support/com.isaacmarovitz.Whisky/Libraries/Wine/bin/wine64" --version 2>/dev/null
/Applications/Whisky.app/Contents/Resources/WhiskyCmd list 2>/dev/null
```

---

# Phase A — Wine environment

## Step 1 — Homebrew + tools

```sh
command -v brew || /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install winetricks p7zip icoutils mingw-w64
```

`icoutils` is needed for Step 13's icon; `mingw-w64` for Step 10's DLL. Warn the user that
Homebrew's installer may prompt for their password — **they** type it, never you.

## Step 2 — Rosetta 2

```sh
pgrep -q oahd && echo "Rosetta OK" || softwareupdate --install-rosetta --agree-to-license
```

## Step 3 — Whisky.app 2.3.5

The Homebrew cask was **disabled 2026-04-09** (`brew install --cask whisky` exits 0 and
installs nothing). Install from the GitHub release:

```sh
curl -fL -o /tmp/Whisky.zip https://github.com/Whisky-App/Whisky/releases/download/v2.3.5/Whisky.zip
ditto -xk /tmp/Whisky.zip /tmp/whisky-x
ditto /tmp/whisky-x/Whisky.app /Applications/Whisky.app
xattr -dr com.apple.quarantine /Applications/Whisky.app
ln -sf /Applications/Whisky.app/Contents/Resources/WhiskyCmd /opt/homebrew/bin/whisky
defaults read /Applications/Whisky.app/Contents/Info.plist CFBundleShortVersionString   # expect 2.3.5
```

## Step 4 — wine-7.7 runtime

> **This version is not incidental — it is the core finding of this repo.** wine-10 and
> wine-11 use Wine's *new/experimental wow64*, which cannot grow a Gepard thread's stack
> past its first 4 KB guard page (`stack overflow 960 bytes addr 0x0`). wine-7.7 uses the
> old `x86_32on64` path and does not have this bug. Do not "upgrade" it.

Whisky's own runtime CDN (`getwhisky.app/Libraries.zip`) is **dead (404)**, so the runtime
must come from a mirror:

```sh
SUP="$HOME/Library/Application Support/com.isaacmarovitz.Whisky"
curl -fL -C - --retry 8 --retry-all-errors -o ~/Downloads/WhiskyWine-Libraries.zip \
  https://github.com/jirukouya/auRO-whisky-macOS-setup/releases/download/whisky-backup-2026-07-25/WhiskyWine-Libraries-2.5.0.zip
cd ~/Downloads && ditto -xk WhiskyWine-Libraries.zip . && mkdir -p "$SUP"
tar -xzf Libraries.tar.gz -C "$SUP"
chmod -R u+w "$SUP/Libraries" || true
xattr -dr com.apple.quarantine "$SUP/Libraries" || true
```

Whisky requires a **structured** version plist — a plain string is silently rejected:

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
"$SUP/Libraries/Wine/bin/wine64" --version              # MUST print wine-7.7
ls "$SUP/Libraries/Wine/lib/wine/" | grep x86_32on64    # MUST exist — the old path
```

**Verification gate:** if either of those last two lines is wrong, stop. Everything
downstream depends on them.

## Step 5 — Bottle `ro`

```sh
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"
"$WHISKY" create ro
UUID=$(ls -d ~/Library/Containers/com.isaacmarovitz.Whisky/Bottles/*/ | head -1 | xargs basename)
META="$HOME/Library/Containers/com.isaacmarovitz.Whisky/Bottles/$UUID/Metadata.plist"
plutil -replace wineConfig.windowsVersion -string "win10" "$META"
plutil -replace wineConfig.avxEnabled     -bool   true    "$META"
eval "$("$WHISKY" shellenv ro)"
WINEMSYNC=0 wine64 wineboot -u
echo "$UUID"        # record this — Step 10 needs it
```

> `whisky list` prints a bottle path that does not exist (cosmetic CLI bug). Always use
> `~/Library/Containers/com.isaacmarovitz.Whisky/Bottles/<UUID>`.

Leave DXVK's DLLs in the prefix. Step 12 overrides them to builtin; **deleting** them
causes `c0000135`.

---

# Phase B — The client

## Step 6 — Extract

Obtain the client archive from the server's official download page. Then:

```sh
mkdir -p "$HOME/Games/Revenant Elegy"
unrar t "<archive>.rar"                                  # integrity first
cd "$HOME/Games/Revenant Elegy" && unrar x -o+ "<archive>.rar" .
```

> Use `~/Games`, never `~/Desktop` or `~/Documents` — iCloud relocates files mid-install
> and Wine then reports `c0000135`. After extracting, wait ~30 s and re-check the files
> are still there.

Expect `client.exe` (32-bit, Themida-packed), `Revenant Elegy.exe` (64-bit patcher,
WebView2 UI), `opensetup.exe`, `gepard.dll`, `dinput.dll` + `dinput.ini`, and the GRFs.

## Step 7 — x87 byte-patches (Rosetta)

Rosetta cannot translate alternate x87 `FCOM`/`FCOMP` encodings; `opensetup.exe` crashes
with *"Unhandled illegal instruction"*.

```sh
./tools/scan-x87.py "$HOME/Games/Revenant Elegy"/*.exe
```

**Scan, never reuse offsets** — they differ per build. On the verified build,
`opensetup.exe` had 2 sites and `client.exe` had **0** (which is what makes Step 9's rule
survivable). Apply with a backup, then prove it:

```sh
G="$HOME/Games/Revenant Elegy"; S="$G/opensetup.exe"
cp "$S" "$S.orig-backup"
python3 - <<'PY'
p="/Users/<you>/Games/Revenant Elegy/opensetup.exe"
d=bytearray(open(p,'rb').read())
for off, old, new in [(0x21E39,'dcd8dfe0','ddd8b440'), (0x2C0CD,'dcd0dfe0','d8d0dfe0')]:
    assert d[off:off+4]==bytes.fromhex(old), f"context mismatch at {hex(off)} - RESCAN"
    d[off:off+4]=bytes.fromhex(new)
open(p,'wb').write(d); print("patched")
PY
cmp -l "$S.orig-backup" "$S" | wc -l      # expect exactly 4 changed bytes
```

If a context assertion fails, the build differs: rescan and treat the result as a new
finding rather than forcing the known offsets.

## Step 8 — `Setup.exe`

The client calls `GetFileAttributesW("Setup.exe")` roughly **12,000×/second, forever**, and
never opens its window until that file exists. Only `opensetup.exe` ships.

```sh
G="$HOME/Games/Revenant Elegy"
cp "$G/opensetup.exe" "$G/Setup.exe"      # the PATCHED one, after Step 7
```

## Step 9 — `OptionInfo.lua`

The client ships `DX9DEVICENAME = ".DISPLAY1"` — a device that does not exist. D3D9 device
creation returns NULL, the client dereferences it, and dies with *page fault on write
access to 00000128*.

```sh
./tools/patch-optioninfo.py "$HOME/Games/Revenant Elegy/savedata/OptionInfo.lua" 1600 900
grep -E 'DX9DEVICENAME|RENDERSYSTEM|ISFULLSCREEN' "$HOME/Games/Revenant Elegy/savedata/OptionInfo.lua"
```

In the raw file `DX9DEVICENAME` must read **four backslashes, dot, two backslashes**. If it
has half that, an unquoted heredoc collapsed them — use a quoted heredoc or a real script.

> ### The rule that outranks the rest
> **`client.exe` must remain byte-identical.** Gepard hashes its PE header. Patching
> `SizeOfStackCommit` (`0x1000` → `0xFF000`) *does* cleanly fix a thread-stack crash — and
> immediately produces **"Game EXE file corrupted!"**. Do not do it, and do not accept it
> from anyone else's guide. Verify with `shasum -a 256` against the archive copy.

## Step 10 — `wtsapi32.dll`

wine-7.7 stubs `WTSEnumerateProcessesA`, handing Gepard a NULL process list.

```sh
cd tools/wtsapi32 && make && make verify        # expect 52 exports
make install BOTTLE=<UUID-from-step-5>
```

All 52 exports are required — a missing one produces *"The procedure entry point
WTSSendMessageW could not be located"*. mingw's linker **cannot** emit DLL forwarders from
a `.def`; the source uses real stub functions for that reason.

> Honest status: this was added *before* the true final blocker (Step 12) was identified,
> and was never re-tested without it. It is harmless and part of the verified-working
> configuration, but its individual necessity is unproven. See `NOTICE.md`.

## Step 11 — Registry

```sh
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"; eval "$("$WHISKY" shellenv ro)"
WINEMSYNC=0 wine64 reg add "HKCU\\Software\\Wine\\Drivers" /v Audio /d coreaudio /f
WINEMSYNC=0 wine64 reg add "HKCU\\Software\\Wine\\WineDbg" /v ShowCrashDialog /t REG_DWORD /d 0 /f
```

---

# Phase C — Launch

## Step 12 — First launch

```sh
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"; eval "$("$WHISKY" shellenv ro)"
export WINEMSYNC=0 WINEESYNC=0
export WINEDLLOVERRIDES="d3d9,dxgi,d3d10core,d3d11=b;msvcp140,vcruntime140,concrt140,vccorlib140=n,b;dinput=n,b;wtsapi32=n"
export WINE_CPU_TOPOLOGY=4:0,1,2,3
cd "$HOME/Games/Revenant Elegy" && wine64 client.exe 1rag1
```

Each override earns its place:

| Setting | Without it |
|---|---|
| `WINEMSYNC=0 WINEESYNC=0` | Gepard's threads deadlock-spin at ~87% CPU forever |
| `d3d9,dxgi,d3d10core,d3d11=b` | DXVK returns a NULL D3D9 device → `0xc0000005` |
| `msvcp140,…=n,b` | Gepard rejects Wine's builtin MSVC runtimes |
| `dinput=n,b` | Wine's builtin `dinput` replaces ROExt's. `=n` alone is **not** enough |
| `wtsapi32=n` | Step 10's DLL is ignored |
| `WINE_CPU_TOPOLOGY` | Gepard's CPU detection destabilises |

> **On DXVK.** Do not "fix" the graphics by reinstating it. This runtime's MoltenVK reports
> `bufferDeviceAddress: 0` and `timelineSemaphore: 0`; every DXVK build hard-requires both,
> so `vkCreateDevice` fails with `VK_ERROR_FEATURE_NOT_PRESENT`. The resulting NULL device
> is dereferenced at an address that lands *inside `gepard.dll`'s range* — which reads as an
> anti-cheat block and is not one.

**Healthy signs:** RSS climbs past ~500 MB, a window appears within ~20 s, CPU settles into
a render loop. Zero sockets before login is correct.

## Step 13 — `.app` bundles

```sh
./tools/build-apps.sh "$HOME/Games/Revenant Elegy"
```

Produces **Revenant Elegy Patcher.app** (normal use — patches, then its START button
launches the game) and **Revenant Elegy.app** (skips the patch check). Both clear a stale
wineserver first; the patcher also clears any existing patcher, because two running
patchers make START unclickable.

## Step 14 — Hand back

Report the progress table, the bottle UUID, and these standing warnings:

- The client **rewrites `OptionInfo.lua` on exit** — quit fully before editing it.
- Use the **Patcher** app normally; the direct one skips patch checks.
- Never modify `client.exe`.
