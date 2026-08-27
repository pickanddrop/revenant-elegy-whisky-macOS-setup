# Troubleshooting

Find your symptom, apply the fix. Observed on macOS 26.6.2 (M2), 2026-08-27.

## Client will not start

| Symptom | Cause | Fix |
|---|---|---|
| "Game EXE file corrupted!" | Either `client.exe` was modified, or the environment is incomplete (msync on, missing MSVC overrides) | Restore `client.exe` byte-for-byte, then apply the full environment from step 12 |
| Loads the whole game, then crashes around 16s | Wine 10 or 11. Their experimental wow64 cannot grow a Gepard thread's stack: `stack overflow 960 bytes addr 0x0` | Use wine-7.7, which has the older `x86_32on64` path |
| Runs at ~87% CPU forever, no window | Gepard's threads deadlock under msync/esync | `WINEMSYNC=0 WINEESYNC=0`. Kill the wineserver first (`wineserver -k`) or the change does not take |
| One thread pins a core, no window | The client is polling for `Setup.exe`, which it does not ship | `cp opensetup.exe Setup.exe` |
| Crash at `0xc0000005`, address inside `gepard.dll` | DXVK could not create a D3D9 device and returned NULL. Looks like anti-cheat, is not | Force builtin wined3d: `d3d9,dxgi,d3d10core,d3d11=b` |
| `page fault on write access to 00000128` | `DX9DEVICENAME` is `.DISPLAY1`, not a real device, so D3D9 returned NULL | Set it to `\\.\DISPLAY1` (four backslashes, dot, two backslashes in the raw file) |
| "Unhandled illegal instruction" from `opensetup.exe` | Rosetta cannot translate alternate x87 `FCOM`/`FCOMP` encodings | Run `tools/scan-x87.py` and patch the sites it reports |
| `The procedure entry point WTSSendMessageW could not be located` | The replacement `wtsapi32.dll` is missing exports | Rebuild it. All 52 exports are required |
| `c0000135` | A required DLL is missing, or iCloud moved the game folder mid-install | Keep the game in `~/Games`, never Desktop or Documents. Do not delete DXVK's DLLs from the prefix; the override is enough |
| Nothing launches, no error at all | A stale wineserver is running in the wrong sync mode | `wineserver -k`, then launch again |

## Once it runs

| Symptom | Cause | Fix |
|---|---|---|
| Settings revert after you quit | The client rewrites `OptionInfo.lua` on exit | Quit fully before editing that file |
| Looks fullscreen even with `ISFULLSCREENMODE=0` | The client reset the size to your full panel resolution, so it is a screen-sized window | Set `WIDTH` and `HEIGHT` smaller, for example 1600x900 |
| No sound, volume already at 100 | `CmdOnOffList["/bgm"]` and `["/sound"]` are `0` | Set both to `1`, or type `/bgm` and `/sound` in chat |
| Cursor vanishes over the window | `MouseExclusive` is `1` | Set it to `0` |
| Patcher's START button does nothing | Two patchers are running | Quit both, launch one |
| Launcher app shows a blank document icon | The icon is missing from the bundle | Re-run `tools/build-apps.sh` |

## Log noise you can ignore

These appear in normal runs and mean nothing is wrong:

```
err:combase:RoGetActivationFactory ... Windows.UI.ViewManagement.UISettings
err:wldap32:DllMain No libldap support, expect problems
err:ntoskrnl:ZwLoadDriver ... winebth
fixme:coreaudio:ca_channel_layout_to_channel_mask Unhandled channel 0xffffffff
```

## Things that do not work

Do not spend time on these:

| Attempt | Result |
|---|---|
| DXVK 3.0.2 | Needs Vulkan 1.3, client dies instantly |
| DXVK 1.9.4 | Same feature error. The limit is MoltenVK, not DXVK's version |
| `dxvk.conf` tuning | No effect. The missing features are hard requirements |
| Copying a newer MoltenVK into wine-7.7 | `Failed to load Wine graphics driver`, winevulkan ABI mismatch |
| Patching `client.exe` stack commit | Stops the crash, immediately trips the integrity check |
| Deleting `d3d9.dll` from the prefix | `c0000135`. The override alone is enough |
| Picking a Whisky fork by its app version | Those version numbers do not track the Wine version inside |

## Telling a real anti-cheat block from a local bug

Gepard has a server-side setting that decides whether Wine clients are allowed. If a
server has it off, the client will start, `gepard.dll` will initialise, and the connection
to Gepard's license server will be closed by the remote end, usually surfacing as a
`Gepard::GT Code` error rather than a crash.

To check, find the client's PID and look at its connections while it starts:

```sh
lsof -p <pid> -i -a -nP
```

A connection that establishes and then sits in `CLOSE_WAIT`, with your local firewall
confirmed off, means the server refused the environment. Only that server's operators can
change that. Ask them.

Before concluding that, rule out everything above first. The DXVK failure in this document
crashes inside `gepard.dll` and looks exactly like an anti-cheat block.
