#!/usr/bin/env python3
"""
Rewrite savedata/OptionInfo.lua for Wine.

The shipped file has DX9DEVICENAME = ".DISPLAY1" - a display device that does not
exist. D3D9 device creation returns NULL, the client dereferences it, and dies with
"page fault on write access to 00000128".

    ./patch-optioninfo.py "<game dir>/savedata/OptionInfo.lua" [WIDTH] [HEIGHT]

IMPORTANT: the client REWRITES this file on exit. Quit the game fully before running
this, or your changes are silently overwritten.
"""
import os, re, sys, uuid, shutil

def setkv(s, key, value, is_string=False, table="OptionInfoList"):
    val = f'"{value}"' if is_string else str(value)
    pat = re.compile(r'(' + table + r'\["' + re.escape(key) + r'"\]\s*=\s*)(?:"[^"]*"|-?\d+)')
    s2, n = pat.subn(lambda m: m.group(1) + val, s)
    if n != 1:
        raise SystemExit(f"{key}: expected exactly 1 match, got {n}")
    return s2

def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    path = sys.argv[1]
    W = int(sys.argv[2]) if len(sys.argv) > 2 else 1600
    H = int(sys.argv[3]) if len(sys.argv) > 3 else 900

    shutil.copy(path, path + ".bak")
    s = open(path).read()

    for k, v in [("RENDERSYSTEM", 2),        # Direct3D
                 ("ISFULLSCREENMODE", 0),    # windowed - fullscreen fights macOS focus
                 ("MouseExclusive", 0),      # else the cursor vanishes over the window
                 ("WIDTH", W), ("HEIGHT", H),
                 ("OLD_WIDTH", W), ("OLD_HEIGHT", H)]:
        s = setkv(s, k, v)

    s = setkv(s, "DX9DEVICEID", "{%s}" % str(uuid.uuid4()).upper(), is_string=True)
    # In the raw file this must read: four backslashes, dot, two backslashes.
    s = setkv(s, "DX9DEVICENAME", "\\\\\\\\.\\\\DISPLAY1", is_string=True)

    for k in ("/bgm", "/sound"):             # audio toggles, independent of volume
        try:
            s = setkv(s, k, 1, table="CmdOnOffList")
        except SystemExit:
            pass

    open(path, "w").write(s)
    print(f"patched {os.path.basename(path)} ({W}x{H} windowed, audio on); backup at .bak")

if __name__ == "__main__":
    main()
