# Windows packaging (maintainers)

Scripts used by CI to produce a Windows VST3 tree under `dist/windows/`.

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSCORION_BUILD_AU=OFF -DSCORION_BUILD_STANDALONE=OFF
cmake --build build --config Release --target Scorion_VST3
.\installer\windows\pack_dist.ps1 -BuildDir build
```

Optional: compile `ScorionSetup.iss` with Inno Setup if you need a GUI wrapper around the VST3 folder.
