# Building UniquePlayerRedirector

## Easiest Windows build

After installing the requirements below, simply double-click:

```text
BUILD_ME.bat
```

It launches the PowerShell helper with a process-only execution-policy bypass and keeps the console open so build errors remain visible.

## Requirements

- Windows 10/11
- Visual Studio 2022 with **Desktop development with C++**
- Git
- XMake 3.0+
- F4SE installed for the Fallout 4 runtime you intend to test

CommonLibF4 is the compile-time library dependency; F4SE remains a runtime requirement.

## 1. Fetch CommonLibF4

From the project root in PowerShell:

```powershell
.\tools\fetch-commonlib.ps1
```

or manually:

```powershell
git clone --recurse-submodules https://github.com/libxse/commonlibf4.git lib/commonlibf4
```

After that, this path must exist:

```text
lib\commonlibf4\xmake.lua
```

## 2. Configure

A normal release-with-debug-info build:

```powershell
xmake f -m releasedbg
```

If you use the CommonLibF4 deployment environment variables, configure either `XSE_FO4_MODS_PATH` or `XSE_FO4_GAME_PATH` as you normally would for that template.

## 3. Build

```powershell
xmake
```

The produced plugin is `UniquePlayerRedirector.dll`.

## 4. Install

Copy the DLL to:

```text
Fallout 4\Data\F4SE\Plugins\UniquePlayerRedirector.dll
```

Copy the supplied INI to:

```text
Fallout 4\Data\F4SE\Plugins\UniquePlayerRedirector.ini
```

No ESP/ESL/ESM is used.

## First test checklist

Use a disposable save for the first binary test.

1. Put a visibly different `FemaleBody.nif` (or male equivalent) in `PlayerCharacterAssets`.
2. Put one visibly different body DDS in the matching `PlayerHumanFemale`/`PlayerHumanMale` folder.
3. Confirm the player changes and a nearby NPC does not.
4. Add the hand/1P assets.
5. Only after that, add the face DDS files and test opening/closing LooksMenu.
6. Inspect the plugin log for each `Mesh:`, `Skin texture:` and `Face texture:` redirect.

If the face behaves incorrectly, set `EnableFaceTextures=false`; body/hands remain independent.
