# Scorion Windows installer

One-run install that puts **Scorion.vst3** where FL Studio already looks:

`C:\Program Files\Common Files\VST3\Scorion.vst3`

## End users

1. Download **Scorion-Setup-Windows.exe** from [Releases](https://github.com/WajihKazmi/scorion/releases).
2. Run it (one click / Next → Install).
3. Open FL Studio → **Plugin Manager** → find **Scorion** (or refresh plugins).
4. Load as an instrument channel.

No separate preset install — factory bank ships inside the VST3.

## Maintainers (build the Setup.exe)

On Windows CI or a Windows machine:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSCORION_BUILD_AU=OFF -DSCORION_BUILD_STANDALONE=OFF
cmake --build build --config Release --target Scorion_VST3
.\installer\windows\pack_dist.ps1 -BuildDir build
# Then compile installer\windows\ScorionSetup.iss with Inno Setup 6
# Output: dist\Scorion-Setup-Windows.exe
```

Fallback (no Inno): run `install_fl_studio.bat` as Administrator after packing.

## Size

`pack_dist.ps1` drops duplicate wavetable `.wav` files (keeps `.wtbin` only) so the installer stays lean while preserving the full preset bank.
