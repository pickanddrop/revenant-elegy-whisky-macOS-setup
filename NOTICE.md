# Verification ledger

Nothing in this repo is written down as working because it sounded right. Every claim
carries a status, and the status is never inferred from confident prose.

**Rule:** nothing is promoted to a `VERIFIED` tag on the strength of an argument. Promotion
requires a command, its actual output, and a date.

## Status legend

| Tag | Means |
|---|---|
| `VERIFIED` | Tested on real hardware, with the evidence recorded. |
| `CARRIED` | Taken from prior work on RO-under-Whisky and applied here, but **not independently isolated** in this build. It works as part of the whole; its individual necessity is untested. |
| `UNVERIFIED` | Reasoned, applied, but never tested in isolation. A hypothesis. Must not be presented as fact. |

## Verification hardware

MacBook Air M2 · 8 GB · macOS 26.6.2 (arm64) · Whisky 2.3.5 · wine-7.7 · 2026-08-27.

Everything below was run on this one machine. **Single-machine verification is not
cross-machine verification** — see Known limits.

## Ledger

### Environment

| Claim | Status |
|---|---|
| wine-10/11's new-wow64 cannot grow a Gepard thread's stack; **wine-7.7's `x86_32on64` can** | `VERIFIED` — 2026-08-27; reproduced on wine-11.15 and wine-10.0, absent on wine-7.7 |
| DXVK cannot create a D3D9 device here (`bufferDeviceAddress: 0`, `timelineSemaphore: 0`) | `VERIFIED` — 2026-08-27; reproduced on DXVK 1.9.4, 1.10.3 and 3.0.2 |
| The `0x06315f6b` crash is DXVK's NULL device, **not** an anti-cheat block | `VERIFIED` — 2026-08-27; disappears entirely with builtin wined3d |
| `WINEMSYNC=0 WINEESYNC=0` required; Gepard's threads deadlock otherwise | `VERIFIED` — 2026-08-27 |
| A stale wineserver in the wrong sync mode silently blocks startup | `VERIFIED` — 2026-08-27 |
| Whisky's Homebrew cask is disabled; install from the GitHub release | `VERIFIED` — 2026-08-26; cask reported "disabled on 2026-04-09" |
| Whisky's own runtime CDN is dead; a mirror is required | `VERIFIED` — 2026-08-26, HTTP 404 |
| Whisky requires a *structured* version plist; a plain string is silently rejected | `CARRIED` |
| Use `ditto -xk`; macOS `unzip` no-ops on ZIP64 archives >4 GB | `CARRIED` |
| Keep the game directory out of iCloud (`~/Games`, not Desktop/Documents) | `CARRIED` |
| `WINE_CPU_TOPOLOGY=4:0,1,2,3` stabilises Gepard's CPU detection | `CARRIED` |
| Native MSVC overrides (`msvcp140,…=n,b`) are required by Gepard | `CARRIED` |
| `WINEDLLOVERRIDES` must be **appended**, never replaced | `VERIFIED` — 2026-08-26; replacing it silently disabled DXVK and caused hours of misdiagnosis |

### The client

| Claim | Status |
|---|---|
| Rosetta cannot translate alternate x87 `FCOM`/`FCOMP` encodings | `VERIFIED` — 2026-08-26; reproduced on `opensetup.exe` |
| x87 offsets **differ per build** and must be rescanned | `VERIFIED` — 2026-08-27; two builds of the same tool disagreed (`0x21E39`/`0x2C0CD` vs `0x1E1B9`/`0x2844D`) |
| `client.exe` and the patcher need **zero** x87 patches on this build | `VERIFIED` — 2026-08-27, `tools/scan-x87.py` |
| Client polls `GetFileAttributesW("Setup.exe")` forever; creating it unblocks | `VERIFIED` — 2026-08-27; 217,482 calls in 18 s via `WINEDEBUG=+file` |
| Shipped `DX9DEVICENAME = ".DISPLAY1"` causes a NULL D3D9 device | `VERIFIED` — 2026-08-27; page fault at `0x128`, fixed by `\\.\DISPLAY1` |
| Patching `client.exe` `SizeOfStackCommit` fixes the crash **and** trips Gepard integrity | `VERIFIED` — 2026-08-27; both halves observed directly |
| The client rewrites `OptionInfo.lua` on exit, clobbering live edits | `VERIFIED` — 2026-08-27 |
| Two running patchers make the START button unclickable | `VERIFIED` — 2026-08-27 |
| **The replacement `wtsapi32.dll` is required** | `UNVERIFIED` — the DLL builds, exports all 52 symbols, and is installed, but it was added *before* the true final blocker was found and **never re-tested without it.** It may be unnecessary. Recorded honestly rather than claimed |

## Known limits

- **One machine, one day.** Every `VERIFIED` tag rests on a single M2 running macOS 26.6.2.
  Different silicon, macOS versions, or GPU drivers may diverge — especially the MoltenVK
  feature findings, which are driver-dependent by nature.
- **One client build.** The x87 offsets, the `Setup.exe` poll, and the `OptionInfo.lua`
  defect are properties of the build current on 2026-08-27. A server patch can change any
  of them; `tools/scan-x87.py` exists so offsets are re-derived rather than trusted.
- **`wtsapi32.dll` is unproven**, as recorded above.
- **Long-session stability is untested.** Verified to launch, render, and reach the login
  screen. Extended play, map changes, and WoE-scale load are not characterised.
- **No claim about other servers.** Whether a given server permits Wine is its operator's
  configuration, not a property of this setup.
