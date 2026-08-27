#!/bin/zsh
# Revenant Elegy on macOS — working configuration (wine-7.7 + builtin wined3d)
WHISKY="/Applications/Whisky.app/Contents/Resources/WhiskyCmd"
eval "$("$WHISKY" shellenv ro)"

# Gepard threads deadlock/spin under msync/esync
export WINEMSYNC=0 WINEESYNC=0

# builtin wined3d for D3D9 (DXVK cannot create a device on wine-7.7's MoltenVK)
# native MSVC runtimes next to the exe (Gepard checks these)
# ROExt dinput.dll from the game folder; custom wtsapi32 with real WTSEnumerateProcesses
export WINEDLLOVERRIDES="d3d9,dxgi,d3d10core,d3d11=b;msvcp140,vcruntime140,concrt140,vccorlib140=n,b;dinput=n,b;wtsapi32=n"

# stabilises Gepard's CPU-detection
export WINE_CPU_TOPOLOGY=4:0,1,2,3

cd "$HOME/Games/Revenant Elegy" || exit 1
exec wine64 client.exe 1rag1
