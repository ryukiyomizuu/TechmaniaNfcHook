param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build-ninja'

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $cmake) {
    throw 'cmake was not found. Run this from an x64 Visual Studio developer shell.'
}

cmake -S $repoRoot -B $buildDir -G Ninja `
    "-DCMAKE_BUILD_TYPE=$Configuration"
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }

cmake --build $buildDir
if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }

ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Native tests failed.' }

Write-Host "Built and tested: $buildDir"

