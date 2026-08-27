# Revenant Elegy on macOS (Apple Silicon)

Run the Revenant Elegy Ragnarok Online client on an Apple Silicon Mac.

The client is 32-bit Windows code protected by Gepard Shield 3.0 and packed with
Themida, so most standard Wine setups fail on it. This repo is the configuration that
works, plus the tools to apply it.

Tested on macOS 26.6.2 (M2) on 2026-08-27.

## Install

You need the client archive from the server's official download page, and about 15 GB
free.

**If you use a coding agent:** hand it [`SKILL.md`](./SKILL.md) and ask it to set up the
client. It runs the whole install and checks its own work.

**By hand:** follow [`SKILL.md`](./SKILL.md) top to bottom. It is 14 steps with the exact
commands.

Either way, the last step builds a launcher app you double-click to play.

## What makes it work

Five things. Each one is required, and skipping any of them produces a failure that looks
like something else.

| Setting | Why |
|---|---|
| **wine-7.7**, not newer | Newer Wine uses the experimental wow64 path, which cannot grow a Gepard thread's stack. The client loads fully, then crashes. |
| **Builtin wined3d**, not DXVK | This runtime's MoltenVK is missing two features DXVK requires, so DXVK hands the client a NULL D3D9 device. The crash lands inside `gepard.dll`, so it looks like anti-cheat. It is not. |
| **msync and esync off** | Gepard's threads deadlock under them and spin at 87% CPU with no window. |
| **A file named `Setup.exe`** | The client polls for it about 12,000 times a second and never opens a window until it exists. Copy `opensetup.exe` to `Setup.exe`. |
| **Fix `DX9DEVICENAME`** | The client ships `.DISPLAY1`, which is not a real device. D3D9 returns NULL and the client crashes on it. It needs `\\.\DISPLAY1`. |

The launch environment:

```sh
export WINEMSYNC=0 WINEESYNC=0
export WINEDLLOVERRIDES="d3d9,dxgi,d3d10core,d3d11=b;msvcp140,vcruntime140,concrt140,vccorlib140=n,b;dinput=n,b;wtsapi32=n"
export WINE_CPU_TOPOLOGY=4:0,1,2,3
```

## Do not modify client.exe

Gepard hashes the executable's headers. Any edit produces "Game EXE file corrupted!".

This matters because one tempting fix actually works: pre-committing the stack in the PE
header stops the crash on newer Wine. It also trips the integrity check immediately. Fix
the Wine version instead.

## If something breaks

[`TROUBLESHOOTING.md`](./TROUBLESHOOTING.md) lists each symptom with its cause and fix,
including the failures that look like anti-cheat blocks but are not.

## Tools

| | |
|---|---|
| `tools/scan-x87.py` | Finds the x87 instructions Rosetta cannot translate. Offsets differ between builds, so scan rather than copying offsets from anywhere. |
| `tools/patch-optioninfo.py` | Sets the display device, window size, and audio in `OptionInfo.lua`. |
| `tools/wtsapi32/` | Source and Makefile for a replacement `wtsapi32.dll`. |
| `tools/build-apps.sh` | Builds the launcher apps with the game's icon. |
| `tools/play.sh` | Launches from a terminal with the full environment. |

## Known limits

- Verified on one machine (M2, macOS 26.6.2). The MoltenVK findings depend on the GPU
  driver, so other hardware may differ.
- Verified against one client build. A server patch can move the byte offsets, which is
  why `scan-x87.py` exists.
- The replacement `wtsapi32.dll` was added before the real cause of the final crash was
  found, and never tested in isolation. It may not be necessary.
- Tested to the login screen. Long sessions, map changes, and WoE-scale load are not
  characterised.

## Scope

This repo makes a client you already have run on a Mac. It does not defeat, disable, or
evade anti-cheat, and it will not help you do that. If a server's Gepard configuration
refuses Wine, that is the operator's setting to change, not something to work around.

## License

MIT, see [`LICENSE`](./LICENSE).
