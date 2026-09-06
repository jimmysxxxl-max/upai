$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Push-Location $root
try {
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw "git was not found in PATH"
    }
    if (-not (Get-Command xmake -ErrorAction SilentlyContinue)) {
        throw "xmake was not found in PATH (requires XMake 3.0+)"
    }

    $commonLib = Join-Path $root "lib\commonlibf4\xmake.lua"
    if (-not (Test-Path $commonLib)) {
        & (Join-Path $root "tools\fetch-commonlib.ps1")
    }

    Write-Host "Configuring releasedbg build..."
    xmake f -m releasedbg
    if ($LASTEXITCODE -ne 0) { throw "xmake configure failed" }

    Write-Host "Building UniquePlayerRedirector..."
    xmake build UniquePlayerRedirector
    if ($LASTEXITCODE -ne 0) { throw "xmake build failed" }

    $dll = Get-ChildItem -Path (Join-Path $root "build") -Recurse -File -Filter "UniquePlayerRedirector.dll" |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
    if (-not $dll) {
        throw "Build succeeded but UniquePlayerRedirector.dll was not found under build\\"
    }

    $distPlugins = Join-Path $root "dist\Data\F4SE\Plugins"
    New-Item -ItemType Directory -Force -Path $distPlugins | Out-Null
    Copy-Item $dll.FullName (Join-Path $distPlugins "UniquePlayerRedirector.dll") -Force
    Copy-Item (Join-Path $root "Data\F4SE\Plugins\UniquePlayerRedirector.ini") $distPlugins -Force

    Write-Host ""
    Write-Host "Build packaged to:"
    Write-Host (Join-Path $root "dist\Data\F4SE\Plugins")
    Write-Host ""
    Write-Host "Copy the Data folder into Fallout 4 (or install dist as a mod)."
}
finally {
    Pop-Location
}
