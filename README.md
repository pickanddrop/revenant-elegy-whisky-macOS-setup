# revenant-elegy-whisky-macOS-setup

**A Gepard Shield 3.0 + Themida-packed Ragnarok Online client running on an Apple Silicon
Mac — windowed, with audio, launched from a Dock icon.**

**Status: working.** Verified 2026-08-27 on a MacBook Air M2, macOS 26.6.2 (arm64), 8 GB.

[`SKILL.md`](./SKILL.md) is written *for an AI coding agent* to read and execute. Hand it
over, say "set up the Revenant Elegy client," and it drives the whole install, stopping for
approval as it goes. A human can follow it just as well — it's ordinary prose with commands.

---

## Why this is hard

The client is 32-bit x86 Windows code, protected two ways at once:

- **Gepard Shield 3.0** — anti-cheat that spawns its own threads, hashes the executable's
  headers, and checks the environment around it.
- **Themida / WinLicense** — a commercial packer, so the real code only exists in memory
  after it unpacks itself.

Running that on Apple Silicon means three translation layers stacked on each other: Windows
calls to macOS (Wine), 32-bit to 64-bit (WoW64), and Intel to ARM (Rosetta). Each layer has
edges the client falls off, and several of the failures **look like anti-cheat blocks when
they are not** — the most expensive one in this repo crashed at an address inside
`gepard.dll` and turned out to be a graphics bug.

## The one-paragraph version

Use **wine-7.7**, because Wine's newer *experimental wow64* cannot grow a Gepard thread's
stack past its first guard page. Force **builtin wined3d**, because this runtime's MoltenVK
lacks two features every DXVK build hard-requires, and DXVK's NULL device gets dereferenced
inside a Gepard callback — which reads as an anti-cheat block and isn't one. Turn **msync
off**, create a **`Setup.exe`** the client polls for 12,000 times a second, fix a **display
device name** the client ships broken, and supply a **real `wtsapi32.dll`** because Wine
stubs the one function Gepard calls. Then never touch `client.exe` again.

## Quick start

```sh
git clone https://github.com/pickanddrop/revenant-elegy-whisky-macOS-setup
cd revenant-elegy-whisky-macOS-setup
```

Then either hand `SKILL.md` to a coding agent, or follow it yourself. Once installed:

```sh
./tools/build-apps.sh "$HOME/Games/Revenant Elegy"
```

That produces two double-clickable apps with the game's own icon:

| App | |
|---|---|
| **Revenant Elegy Patcher** | Runs the official patcher; its START button launches the game. **Normal use** — so you never miss a patch. |
| **Revenant Elegy** | Straight to `client.exe`, skipping the patch check. Faster, but won't catch updates. |

## The working configuration

| | |
|---|---|
| Wine | **7.7** — the old `x86_32on64` path. Newer Wine breaks Gepard. Do not "upgrade". |
| Graphics | **builtin wined3d**. Not DXVK. |
| Sync | **msync/esync off**. |
| `client.exe` | **Byte-identical. Never modified.** |

```sh
export WINEMSYNC=0 WINEESYNC=0
export WINEDLLOVERRIDES="d3d9,dxgi,d3d10core,d3d11=b;msvcp140,vcruntime140,concrt140,vccorlib140=n,b;dinput=n,b;wtsapi32=n"
export WINE_CPU_TOPOLOGY=4:0,1,2,3
```

## What's here

| File | |
|---|---|
| [`SKILL.md`](./SKILL.md) | The runbook. 14 steps, progress table, verification gates. |
| [`TROUBLESHOOTING.md`](./TROUBLESHOOTING.md) | Symptom → cause → fix, the seven walls in order, and every dead end so they aren't retried. |
| [`NOTICE.md`](./NOTICE.md) | Verification ledger. Every claim carries a status, including what is *not* proven. |
| `tools/scan-x87.py` | Finds Rosetta-untranslatable x87 sites in any PE. Offsets differ per build — always rescan. |
| `tools/patch-optioninfo.py` | Rewrites `OptionInfo.lua` — display device, windowed size, audio. |
| `tools/wtsapi32/` | Source + Makefile for the replacement `wtsapi32.dll`. |
| `tools/build-apps.sh` | Builds both `.app` bundles with the extracted icon. |
| `tools/play.sh` | Terminal launcher carrying the full environment. |

## How claims are verified

Nothing here is written down as working because it sounded right. Every claim in
[`NOTICE.md`](./NOTICE.md) carries a status, and a `VERIFIED` tag requires a command, its
actual output, and a date. One claim is tagged `NEW-UNVERIFIED` because it was never
re-tested in isolation — recorded honestly rather than quietly assumed.

The limits are stated too: one machine, one day, one client build. A server patch can move
the byte offsets, which is exactly why `tools/scan-x87.py` exists instead of a hardcoded
list.

## Scope, and a deliberate limit

This repo makes a legitimately-obtained client **run** on a Mac. It does **not** defeat,
disable, patch, or evade anti-cheat, and it will not help you do so.

That line is load-bearing rather than decorative. Pre-committing the client's stack in the
PE header genuinely fixes a crash — and is documented here as **forbidden**, because Gepard
hashes that header and doing it is tampering. Where a server's Gepard configuration refuses
Wine outright, the answer is to ask that server's operators, not to work around the check.

## License

MIT — see [`LICENSE`](./LICENSE). Portions are derived from prior MIT-licensed work on
running RO clients under Whisky, credited there.
