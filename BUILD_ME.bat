@echo off
setlocal
cd /d "%~dp0"
echo ============================================================
echo   UniquePlayerRedirector - Alpha Build Helper
echo ============================================================
echo.

where git >nul 2>nul
if errorlevel 1 (
  echo [MISSING] Git is not installed or is not in PATH.
  echo Install Git for Windows, then reopen this window.
  echo.
  goto :requirements
) else (
  echo [OK] Git found.
)

where xmake >nul 2>nul
if errorlevel 1 (
  echo [MISSING] XMake 3.0+ is not installed or is not in PATH.
  echo Install XMake, then reopen this window.
  echo.
  goto :requirements
) else (
  echo [OK] XMake found.
)

echo.
echo Starting build...
echo.
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\build-release.ps1"
set ERR=%ERRORLEVEL%
echo.
if not "%ERR%"=="0" (
  echo ============================================================
  echo BUILD FAILED with exit code %ERR%.
  echo Copy everything shown above and send it to ChatGPT.
  echo ============================================================
) else (
  echo ============================================================
  echo BUILD FINISHED.
  echo Install the contents of:
  echo   dist\Data\F4SE\Plugins\
  echo into your Fallout 4 Data folder / mod manager.
  echo ============================================================
)
goto :end

:requirements
echo Required before building:
echo   1. Visual Studio 2022 with Desktop development with C++
echo   2. Git for Windows
echo   3. XMake 3.0 or newer
echo.
echo After installing them, CLOSE this window, reopen the folder,
echo and double-click BUILD_ME.bat again.

:end
echo.
pause
endlocal
