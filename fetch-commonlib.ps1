$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$target = Join-Path $root "lib\commonlibf4"

if (Test-Path $target) {
    Write-Host "lib\commonlibf4 already exists. Remove it first if you want a clean checkout."
    exit 0
}

New-Item -ItemType Directory -Force -Path (Join-Path $root "lib") | Out-Null
git clone --recurse-submodules https://github.com/libxse/commonlibf4.git $target
Write-Host "CommonLibF4 cloned to $target"
