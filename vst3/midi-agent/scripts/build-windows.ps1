# Build MIDI Agent on Windows with the VS 2022 Build Tools.
#   .\vst3\midi-agent\scripts\build-windows.ps1 -SdkDir X:\AI-2\Steinberg-VST-factory\vst3sdk
param(
    [string]$SdkDir = $env:VST3_SDK_DIR,
    [string]$Config = "Release",
    [string]$BuildDir = "build\midi-agent"
)
$ErrorActionPreference = "Stop"
if (-not $SdkDir) { throw "Pass -SdkDir <vst3sdk root> or set VST3_SDK_DIR" }
$root = Resolve-Path (Join-Path $PSScriptRoot "..\..\..")
Push-Location $root
try {
    cmake -S vst3/midi-agent -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DSMTG_VST3_SDK_DIR="$SdkDir"
    if ($LASTEXITCODE) { throw "configure failed" }
    cmake --build $BuildDir --config $Config
    if ($LASTEXITCODE) { throw "build failed" }
    ctest --test-dir $BuildDir -C $Config --output-on-failure
    if ($LASTEXITCODE) { throw "tests failed" }
    $bundle = Join-Path $BuildDir "VST3\$Config\MIDI Agent.vst3"
    Write-Host "Built: $bundle"
    Write-Host "Install: copy the bundle to C:\Program Files\Common Files\VST3\"
}
finally { Pop-Location }
