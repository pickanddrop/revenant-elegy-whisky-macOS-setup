# Build a 32-bit wtsapi32.dll with a real WTSEnumerateProcesses implementation.
#
#   brew install mingw-w64
#   make && make install BOTTLE=<bottle-uuid>
#
# All 52 exports must be present. If any is missing the client dies with
# "The procedure entry point WTSSendMessageW could not be located".
# mingw's linker CANNOT emit DLL forwarders from a .def, real stubs are required.

CC      := i686-w64-mingw32-gcc
PREFIX  := $(HOME)/Library/Containers/com.isaacmarovitz.Whisky/Bottles/$(BOTTLE)
CFLAGS  := -shared -O2 -Wl,--enable-stdcall-fixup

wtsapi32.dll: wtsapi32.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: install verify clean
install: wtsapi32.dll
	@test -n "$(BOTTLE)" || { echo "set BOTTLE=<uuid>"; exit 1; }
	cp wtsapi32.dll "$(PREFIX)/drive_c/windows/syswow64/wtsapi32.dll"
	@echo "installed -> $(PREFIX)/drive_c/windows/syswow64/"

verify: wtsapi32.dll
	@echo "exports: $$(strings wtsapi32.dll | grep -cE '^WTS')  (expect 52)"

clean:
	rm -f wtsapi32.dll
