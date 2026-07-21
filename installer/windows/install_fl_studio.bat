@echo off
REM One-run fallback installer (no Inno Setup required).
REM Run as Administrator. Copies VST3 into the FL Studio-friendly system folder.
setlocal
set "SRC=%~dp0..\..\dist\windows"
if not exist "%SRC%\Scorion.vst3" (
  echo ERROR: dist\windows\Scorion.vst3 missing. Build + pack_dist.ps1 first.
  exit /b 1
)

set "DEST=%CommonProgramFiles%\VST3\Scorion.vst3"
echo Installing Scorion VST3 to:
echo   %DEST%
mkdir "%CommonProgramFiles%\VST3" 2>nul
rmdir /s /q "%DEST%" 2>nul
xcopy /E /I /Y "%SRC%\Scorion.vst3" "%DEST%" >nul
if exist "%SRC%\ScorionResources" (
  mkdir "%DEST%\Contents\Resources\ScorionResources" 2>nul
  xcopy /E /I /Y "%SRC%\ScorionResources" "%DEST%\Contents\Resources\ScorionResources" >nul
  mkdir "%APPDATA%\Scorion\Resources" 2>nul
  xcopy /E /I /Y "%SRC%\ScorionResources" "%APPDATA%\Scorion\Resources" >nul
)

echo.
echo Done. Open FL Studio ^> Plugin Manager ^> find "Scorion" (Instrument / VST3).
echo If missing: Options ^> Manage plugins ^> Start scan / Verify plugins.
pause
endlocal
