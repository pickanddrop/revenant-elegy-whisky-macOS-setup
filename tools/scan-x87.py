#!/usr/bin/env python3
"""
Find x87 FCOM/FCOMP encodings Rosetta cannot translate.

Rosetta rejects the alternate `DC D0`-`DC DF` forms. RO's OpenSetup uses them and
crashes with "Unhandled illegal instruction". The reliable signature is one of those
opcodes immediately followed by `DF E0` (FNSTSW AX) - the classic compare idiom.

    ./scan-x87.py "<game dir>"/*.exe

Offsets differ per build. NEVER reuse offsets from another binary - always rescan.
Patches (same length, so nothing shifts):
    dc d8 df e0  ->  dd d8 b4 40   FCOMP ST(0);FNSTSW  ->  FSTP ST(0); MOV AH,0x40
    dc d0 df e0  ->  d8 d0 df e0   alternate FCOM ST(0) -> canonical encoding
"""
import struct, sys, os

def text_section(d):
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    if d[pe:pe + 4] != b'PE\0\0':
        return None
    nsec  = struct.unpack_from('<H', d, pe + 6)[0]
    optsz = struct.unpack_from('<H', d, pe + 20)[0]
    magic = struct.unpack_from('<H', d, pe + 24)[0]
    base  = (struct.unpack_from('<I', d, pe + 52)[0] if magic == 0x10b
             else struct.unpack_from('<Q', d, pe + 48)[0])
    off = pe + 24 + optsz
    for _ in range(nsec):
        name = d[off:off + 8].rstrip(b'\0').decode(errors='replace')
        vsize, vaddr, rsize, raddr = struct.unpack_from('<IIII', d, off + 8)
        if name == '.text':
            return base, vaddr, raddr, rsize, magic
        off += 40
    return None

def scan(path):
    d = open(path, 'rb').read()
    sec = text_section(d)
    name = os.path.basename(path)
    if not sec:
        print(f"{name:28} not a PE / no .text (packed?)")
        return
    base, vaddr, raddr, rsize, magic = sec
    seg = d[raddr:raddr + rsize]
    hits = [(raddr + i, base + vaddr + i, seg[i:i + 4])
            for i in range(len(seg) - 3)
            if seg[i] == 0xDC and 0xD0 <= seg[i + 1] <= 0xDF
            and seg[i + 2] == 0xDF and seg[i + 3] == 0xE0]
    bits = '32-bit' if magic == 0x10b else '64-bit'
    print(f"{name:28} {bits}  {len(hits)} site(s)")
    for off, va, b in hits:
        fix = 'ddd8b440' if b[1] == 0xD8 else 'd8' + b[1:].hex()
        print(f"     file=0x{off:06x}  VA=0x{va:08x}  {b.hex()} -> {fix}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for p in sys.argv[1:]:
        scan(p)
