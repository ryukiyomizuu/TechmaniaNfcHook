param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath,
    [string]$BuildPath = $(Join-Path (Split-Path -Parent $PSScriptRoot) 'build-ninja\TechmaniaNfcHook.dll')
)

$ErrorActionPreference = 'Stop'
$project = (Resolve-Path -LiteralPath $ProjectPath).Path
$source = (Resolve-Path -LiteralPath $BuildPath).Path
$assets = Join-Path $project 'Assets'
if (-not (Test-Path -LiteralPath $assets -PathType Container)) {
    throw "Not a Unity project: $project"
}

$destinationFolder = Join-Path $assets 'Plugins\x86_64'
New-Item -ItemType Directory -Path $destinationFolder -Force | Out-Null
$destination = Join-Path $destinationFolder 'TechmaniaNfcHook.dll'
Copy-Item -LiteralPath $source -Destination $destination -Force

$sourceHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $source).Hash
$destinationHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $destination).Hash
if ($sourceHash -ne $destinationHash) {
    throw 'Deployment hash verification failed.'
}

Write-Host "Deployed: $destination"
Write-Host "SHA-256: $destinationHash"
