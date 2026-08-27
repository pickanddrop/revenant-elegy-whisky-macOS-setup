# Provenance & verification ledger

This repo is a derivative of
**[jirukouya/auRO-whisky-macOS-setup](https://github.com/jirukouya/auRO-whisky-macOS-setup)**
(MIT), and a sibling of [Kafra](https://github.com/pickanddrop/Kafra).

Upstream's value is that **every fix in it was verified on real hardware**, not reasoned
out. A derivative that quietly mixes inherited-and-verified material with new, untested
material destroys exactly the property worth deriving from. So this file is the ledger:
every claim carries a status, and the status is never inferred from confident prose.

## Status legend

| Tag | Means |
|---|---|
| `INHERITED-VERIFIED` | From upstream, which verified it on real hardware. Not re-tested here. |
| `INHERITED-RETESTED` | From upstream **and** re-confirmed here, with date and command. |
| `INHERITED-DIVERGED` | From upstream, re-tested here, **result differs.** We follow the observation and record both. |
| `NEW-VERIFIED` | Original to this repo, tested on real hardware, evidence recorded. |
| `NEW-UNVERIFIED` | Original, reasoned but **not run**. A hypothesis. Must not be presented as fact. |
| `NOT-APPLICABLE` | Upstream material deliberately dropped, with the reason. |

**Rule:** nothing is promoted to a `VERIFIED` tag on the strength of an argument. Promotion
requires a command, its actual output, and a date.

## Verification hardware

MacBook Air M2 · 8 GB · macOS 26.6.2 (arm64) · Whisky 2.3.5 · wine-7.7 · 2026-08-27.
Everything below was run on this one machine. **Single-machine verification is not
cross-machine verification** — see Known limits.

## Ledger

### Inherited from upstream

| Claim | Status |
|---|---|
| Whisky's Homebrew cask is disabled; install from the GitHub release | `INHERITED-RETESTED` — 2026-08-26, cask reported "disabled on 2026-04-09" |
| `getwhisky.app/Libraries.zip` is dead; use the mirrored runtime | `INHERITED-RETESTED` — 2026-08-26, HTTP 404 |
| Whisky requires a *structured* version plist, not a string | `INHERITED-VERIFIED` |
| Use `ditto -xk`; macOS `unzip` no-ops on ZIP64 >4 GB | `INHERITED-VERIFIED` |
| Keep the game dir out of iCloud (`~/Games`, not Desktop/Documents) | `INHERITED-VERIFIED` |
| Rosetta cannot translate alternate x87 `FCOM`/`FCOMP` encodings | `INHERITED-RETESTED` — 2026-08-26, reproduced on `opensetup.exe` |
| `whisky list` prints a nonexistent bottle path (cosmetic) | `INHERITED-VERIFIED` |
| `WINE_CPU_TOPOLOGY=4:0,1,2,3` stabilises Gepard CPU detection | `INHERITED-VERIFIED` — carried forward, not independently isolated here |
| Native MSVC overrides (`msvcp140,…=n,b`) required by Gepard | `INHERITED-VERIFIED` — carried forward, not independently isolated here |
| `WINEDLLOVERRIDES` must be **appended**, never replaced | `INHERITED-RETESTED` — 2026-08-26; replacing it silently disabled DXVK and caused hours of misdiagnosis |
| Two x87 sites at `0x21E39` / `0x2C0CD` | `INHERITED-RETESTED` — same offsets found independently by `tools/scan-x87.py` on this build. **Coincidence, not a rule** — always rescan |

### New to this repo

| Claim | Status |
|---|---|
| wine-10/11's new-wow64 cannot grow a Gepard thread's stack; **wine-7.7's `x86_32on64` can** | `NEW-VERIFIED` — 2026-08-27; reproduced on wine-11.15 and wine-10.0, absent on wine-7.7 |
| DXVK cannot create a D3D9 device here (`bufferDeviceAddress: 0`, `timelineSemaphore: 0`) | `NEW-VERIFIED` — 2026-08-27; reproduced on DXVK 1.9.4, 1.10.3, 3.0.2 |
| The `0x06315f6b` crash is DXVK's NULL device, **not** an anti-cheat block | `NEW-VERIFIED` — 2026-08-27; disappears entirely with builtin wined3d |
| `WINEMSYNC=0 WINEESYNC=0` required; Gepard threads deadlock otherwise | `NEW-VERIFIED` — 2026-08-27 |
| Client polls `GetFileAttributesW("Setup.exe")` forever; creating it unblocks | `NEW-VERIFIED` — 2026-08-27; 217,482 calls in 18 s via `WINEDEBUG=+file` |
| Shipped `DX9DEVICENAME = ".DISPLAY1"` causes a NULL D3D9 device | `NEW-VERIFIED` — 2026-08-27; page fault at `0x128`, fixed by `\\.\DISPLAY1` |
| Patching `client.exe` `SizeOfStackCommit` fixes the crash **and** trips Gepard integrity | `NEW-VERIFIED` — 2026-08-27; both halves observed directly |
| `client.exe` and the patcher need **zero** x87 patches on this build | `NEW-VERIFIED` — 2026-08-27, `tools/scan-x87.py` |
| A stale wineserver in the wrong sync mode silently blocks startup | `NEW-VERIFIED` — 2026-08-27 |
| Two running patchers make START unclickable | `NEW-VERIFIED` — 2026-08-27 |
| Divine Armaments' Gepard license server refuses this environment (`GT Code: 301`) | `NEW-VERIFIED` — 2026-08-26; TCP to `139.99.40.25:6700` observed in `CLOSE_WAIT`, local firewall off |
| **The replacement `wtsapi32.dll` is required** | `NEW-UNVERIFIED` — the DLL works and is installed, but it was added *before* wall #7 was found. **Never re-tested without it.** It may be unnecessary. Recorded honestly rather than claimed |
| frankea Whisky app versions do not track Wine versions (v3.1.1 = wine-11.0) | `NEW-VERIFIED` — 2026-08-27 |

### Deliberately dropped from upstream

| Material | Status |
|---|---|
| uaRO installer / Inno Setup steps | `NOT-APPLICABLE` — different client, archive-distributed |
| `clientinfo.xml` rewiring for a self-hosted server | `NOT-APPLICABLE` — this targets a live remote server; see Kafra |
| DXVK enablement and tuning | `NOT-APPLICABLE` — actively harmful here; builtin wined3d instead |
| Wine Gecko pre-install | `NOT-APPLICABLE` — Gecko 2.47.2 already ships in this runtime |
| Upstream's hardware-ban material | `NOT-APPLICABLE` — out of scope; see Scope in the README |

## Known limits

- **One machine, one day.** Every `NEW-VERIFIED` tag rests on a single M2 running macOS
  26.6.2. Different silicon, macOS versions, or GPU drivers may diverge — especially the
  MoltenVK feature findings, which are driver-dependent by nature.
- **One client build.** The x87 offsets, the `Setup.exe` poll, and the `OptionInfo.lua`
  defect are all properties of the build current on 2026-08-27. A server patch can change
  any of them. `tools/scan-x87.py` exists so offsets are re-derived rather than trusted.
- **`wtsapi32.dll` is unproven**, as recorded above.
- **Long-session stability is untested.** Verified to launch, render, and reach the login
  screen. Extended play, map changes, and WoE-scale load are not characterised.
- **No claim about other Gepard servers.** Whether a given server permits Wine is the
  operator's configuration, not a property of this setup.
