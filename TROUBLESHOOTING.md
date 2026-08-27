# Troubleshooting

Everything here was observed on real hardware (MacBook Air M2, macOS 26.6.2, 2026-08-27)
unless marked otherwise.

## The seven walls, in the order they appeared

Each had to fall before the next became visible. **Two were misdiagnosed as anti-cheat
blocks and were not** — that is the single most expensive lesson in this repo.

### 1. "Game EXE file corrupted!" on wine-7.7 — *false alarm*

**Symptom:** Gepard rejects the client immediately on the correct Wine version.

**Cause:** the environment was incomplete — msync on, no native MSVC overrides. The
verdict was recorded before the configuration was finished, and sent the investigation
down a multi-hour detour through three other Wine versions.

**Fix:** complete the environment (Step 12) before concluding anything about integrity.
With it, wine-7.7 passes.

**Lesson:** do not promote an early failure to a conclusion about anti-cheat.

### 2 & 3. wine-11.15 / wine-10.0 — stack overflow in a Gepard thread

**Symptom:** integrity passes, the launcher UI renders, the client loads the full game
(~470–650 MB), then dies at ~16 s — or busy-spins at 84% and never opens a window.

```
err:virtual:virtual_setup_exception stack overflow 960 bytes addr 0x0 stack 0x44b0c40
NtRaiseException Exception frame is not in stack limits
```

**Cause:** Wine's **new/experimental wow64** cannot grow a normal
1 MB-reserve / 4 KB-commit thread stack past its first guard page under Rosetta. Gepard
spawns such a thread.

**Fix: use wine-7.7**, whose old `x86_32on64` path grows the stack correctly.

**Do not "fix" it by patching the client** — see the boxed rule below.

### 4. Infinite 87% CPU spin, no window

**Cause:** Gepard's threads deadlock under msync/esync.

**Fix:** `WINEMSYNC=0 WINEESYNC=0`.

Toggling this requires **fully killing the wineserver** (`wineserver -k`), or you get
`Server is running with WINEMSYNC but this process is not` and nothing starts.

### 5. Infinite file-poll hang

**Symptom:** full game loads, then one thread pins a core forever with no window.

**Evidence:** `WINEDEBUG=+file` captured **217,482** `GetFileAttributesW("Setup.exe")`
calls in 18 seconds.

**Fix:** `cp opensetup.exe Setup.exe` (Step 8).

### 6. `WTSEnumerateProcessesA` returns NULL

**Cause:** wine-7.7 stubs it; Gepard walks a NULL process list.

**Fix:** the replacement `wtsapi32.dll` in `tools/wtsapi32/` (Step 10).

### 7. The real final blocker — DXVK, not Gepard

**Symptom:** `0xc0000005` at `eip 0x06315f6b`, an address inside `gepard.dll`'s mapped
range. Deterministic, identical stack every run. Reads exactly like an anti-cheat block.

```
[mvk-error] VK_ERROR_FEATURE_NOT_PRESENT: vkCreateDevice(): Requested feature is not available
err:   DxvkAdapter: Failed to create device
```

**Cause:** this runtime's MoltenVK reports `bufferDeviceAddress: 0` and
`timelineSemaphore: 0`. Every DXVK build hard-requires both, so device creation fails, DXVK
hands back a NULL device, and the client dereferences it — inside a Gepard callback, hence
the misleading address.

**Fix:** force builtin wined3d — `d3d9,dxgi,d3d10core,d3d11=b`.

---

> ## The rule that outranks the rest
>
> **`client.exe` must remain byte-identical.** Gepard hashes its PE header.
>
> Pre-committing the stack (`SizeOfStackCommit` `0x1000` → `0xFF000`) **does** cleanly fix
> wall #2/#3 — the client stops crashing and idles correctly. It also **immediately**
> produces *"Game EXE file corrupted!"*. The crash-fix and the integrity check are mutually
> exclusive on the client side; the fix belongs in Wine, not the binary.
>
> Verify with `shasum -a 256` against the copy inside the distribution archive.

---

## Dead ends

Tried and failed. Do not repeat.

| Attempt | Result |
|---|---|
| DXVK 3.0.2 | Requires Vulkan 1.3; client dies instantly |
| DXVK 1.9.4 | Same `FEATURE_NOT_PRESENT` — the limit is MoltenVK, not DXVK's version |
| `dxvk.conf` tuning | No effect; the missing features are hard requirements |
| Graft wine-10/11 MoltenVK onto wine-7.7 | `err:vulkan:wine_vk_init Failed to load Wine graphics driver` — winevulkan ABI mismatch |
| Patch `client.exe` stack commit | Fixes the crash, triggers "corrupted" |
| Delete `d3d9.dll` from the prefix | `c0000135`; the override alone is sufficient |
| frankea Whisky **v3.1.1** (sought wine-9.x) | Actually ships **wine-11.0** — their app version does not track the Wine version |
| Partial-range download to probe a runtime's version | gzip needs the whole stream; download fully or don't bother |

## Everyday gotchas

| Symptom | Cause | Fix |
|---|---|---|
| Settings revert after quitting | The client **rewrites `OptionInfo.lua` on exit** | Quit fully *before* editing it |
| Game looks fullscreen despite `ISFULLSCREENMODE=0` | Client reset WIDTH/HEIGHT to the panel's full resolution — a screen-sized *window* | Set `WIDTH`/`HEIGHT` to something smaller (1600×900) |
| Nothing launches, no error | Stale wineserver in the wrong sync mode | `wineserver -k`, then relaunch. Both `.app`s do this automatically |
| Patcher's START button does nothing | Two patchers running | Kill all, launch one. The patcher `.app` does this automatically |
| Silent audio despite volume 100 | `CmdOnOffList["/bgm"]` / `["/sound"]` are `0` | Set both to `1`, or type `/bgm` and `/sound` in game chat |
| `The procedure entry point WTSSendMessageW could not be located` | Replacement `wtsapi32.dll` missing exports | Rebuild; all 52 required |
| Generic document icon on the `.app` | No `CFBundleIconFile`, or no icon in `Contents/Resources` | Re-run `tools/build-apps.sh` |
| Cursor disappears over the window | `MouseExclusive=1` | Set to `0` |

**Harmless log noise** — all expected, none worth chasing:
`RoGetActivationFactory ... Windows.UI.ViewManagement.UISettings` (×2, the patcher's
WebView2 theme probe), `wldap32 No libldap support`, `winebth` driver failure,
`ca_channel_layout_to_channel_mask Unhandled channel 0xffffffff`.

---

## When it is genuinely the anti-cheat

Not every wall is a bug on your side.

Gepard has a **server-side option** governing whether Wine clients are permitted. Where a
server has it switched off, the client will run, `gepard.dll` will initialise, and the
connection to Gepard's license server will be **closed by the remote end** — typically
surfacing as a `Gepard::GT Code` error rather than a crash. A useful discriminator: check
whether a TCP connection to the license server is actually established and then dropped
(`lsof -p <pid> -i`, looking for `CLOSE_WAIT`) with the local firewall confirmed off.

That situation is a **server-side refusal of the environment**, and only that server's
operators can change it. The correct response is to ask them — not to patch `gepard.dll`,
redirect the license host, or otherwise defeat the check. This repo will not help with
that, and nothing in it does.

Distinguishing the two cases is the whole skill: **wall #7 above looked exactly like an
anti-cheat block and was a graphics bug.** Exhaust the environment before concluding a
server has refused you.
