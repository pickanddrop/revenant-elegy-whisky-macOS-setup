# revenant-elegy-whisky-macOS-setup

**A Gepard-protected, Themida-packed Ragnarok Online client running on an Apple Silicon
Mac — windowed, with audio, launched from a Dock icon.**

Sibling to [Kafra](https://github.com/pickanddrop/Kafra) (self-hosted rAthena client) and
[Hollgrehenn](https://github.com/pickanddrop/Hollgrehenn) (the server half). Derived from
[jirukouya/auRO-whisky-macOS-setup](https://github.com/jirukouya/auRO-whisky-macOS-setup),
whose operating principle this repo keeps: **nothing is written down as working unless a
command actually proved it on real hardware.**

[`SKILL.md`](./SKILL.md) is written *for an AI coding agent* to read and execute. Hand it
over, say "set up the Revenant Elegy client," and it drives the whole thing.

**Status: working.** Verified 2026-08-27 on a MacBook Air M2, macOS 26.6.2 (arm64), 8 GB.

---

## Why this is its own repo

Kafra targets a client you control, pointed at your own server. This one targets a **live
remote server running Gepard Shield 3.0**, where the client is also **Themida/WinLicense
packed**. That combination invalidates most of Kafra's defaults:

| | Kafra | This repo |
|---|---|---|
| Wine | current builds | **wine-7.7 specifically** — newer Wine breaks Gepard |
| Graphics | DXVK | **builtin wined3d** — DXVK cannot create a D3D9 device here |
| Client binary | patchable | **must stay byte-identical** — Gepard hashes the PE header |
| Sync | msync on | **msync off** — Gepard threads deadlock under it |

Four of the seven failures in this repo have no analogue upstream.

## The one-paragraph version

Use **wine-7.7**, because Wine's newer *experimental wow64* cannot grow a Gepard thread's
stack past its first guard page. Force **builtin wined3d**, because this runtime's MoltenVK
lacks two features every DXVK build hard-requires, and DXVK's null device gets dereferenced
at an address that happens to land inside `gepard.dll` — which looks exactly like an
anti-cheat block and is not one. Turn **msync off**, create a **`Setup.exe`** the client
polls for 12,000 times a second, fix a **display-device name** the client ships broken, and
supply a **real `wtsapi32.dll`** because Wine stubs the one function Gepard calls. Then
never touch `client.exe` again.

## What's here

| File | |
|---|---|
| [`SKILL.md`](./SKILL.md) | The runbook. Agent-executable, step by step, with a progress table. |
| [`TROUBLESHOOTING.md`](./TROUBLESHOOTING.md) | Symptom → cause → fix, plus every dead end so they aren't retried. |
| [`NOTICE.md`](./NOTICE.md) | Provenance and the verification ledger. Every claim carries a status. |
| `tools/scan-x87.py` | Finds the Rosetta-untranslatable x87 sites in any PE. Offsets differ per build — always rescan. |
| `tools/patch-optioninfo.py` | Rewrites `OptionInfo.lua` (display device, windowed size, audio). |
| `tools/wtsapi32/` | Source + Makefile for the replacement `wtsapi32.dll`. |
| `tools/build-apps.sh` | Builds both `.app` bundles with the game's own icon. |
| `tools/play.sh` | Terminal launcher carrying the full environment. |

## Scope, and a deliberate limit

This repo makes a legitimately-obtained client run on a Mac. It does **not** defeat,
disable, patch, or evade anti-cheat, and it will not help you do so.

That line is load-bearing rather than decorative. Pre-committing the client's stack in the
PE header genuinely fixes a crash — and is documented here as **forbidden**, because Gepard
hashes that header and it is tampering. Where a server's Gepard configuration refuses Wine
outright, the answer is to ask that server's operators, not to work around the check. One
such server is documented in `TROUBLESHOOTING.md` as unplayable, with the evidence.

## Credit

The hard-won upstream material — Whisky sourcing when its cask and CDN are both dead, the
version-plist schema Whisky silently rejects, ZIP64 archives macOS's `unzip` no-ops on,
iCloud relocating files mid-install, Rosetta's untranslatable x87 FCOM encodings — is
jirukouya's, verified on their hardware before it was ever written down. MIT, dual
attribution in [`LICENSE`](./LICENSE).
