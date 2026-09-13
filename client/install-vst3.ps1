$ErrorActionPreference = "Stop"
$src = Join-Path $PSScriptRoot "build\SONORA_VST3_artefacts\Release\VST3\SONORA.vst3"
$dstDir = "C:\Program Files\Common Files\VST3"
$dst = Join-Path $dstDir "SONORA.vst3"

if (-not (Test-Path -LiteralPath $src)) {
    throw "Build the VST3 first: cmake --build client/build --config Release --target SONORA_VST3_VST3"
}

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Start-Process -FilePath "powershell.exe" -Verb RunAs -Wait -ArgumentList @(
        "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "`"$PSCommandPath`""
    )
    exit $LASTEXITCODE
}

New-Item -ItemType Directory -Force -Path $dstDir | Out-Null
if (Test-Path -LiteralPath $dst) {
    Remove-Item -LiteralPath $dst -Recurse -Force
}
Copy-Item -LiteralPath $src -Destination $dst -Recurse -Force
Write-Host "Installed: $dst"
Write-Host "In FL Studio: Options > Manage plugins > Find plugins"
