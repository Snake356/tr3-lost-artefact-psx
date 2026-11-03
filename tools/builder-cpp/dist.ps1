# Zips the distribution folder of a Release build into the download: <BuildDir>\TR3-Lost-Artefact-PS1-Builder-v<version>.zip,
# the files in a folder "TR3 Lost Artefact PS1 Builder" (version 1.12.0 is named 1.12)
#   -BuildDir <folder> (default build)   -Build (builds Release first, with build.ps1)
# The version is read from the EXE's VERSIONINFO, which CMake fills from project(VERSION).
param([string]$BuildDir = "build", [switch]$Build)
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$bd = if ([System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir } else { Join-Path $root $BuildDir }
if ($Build) {
    & (Join-Path $root "build.ps1") -Config Release -BuildDir $bd
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
$dist = Join-Path $bd "dist"
$exe = Join-Path $dist "TR3 Lost Artefact PS1 Builder.exe"
foreach ($need in @($exe, (Join-Path $dist "data.pak"), (Join-Path $dist "README.txt"), (Join-Path $dist "CHANGES.txt"), (Join-Path $dist "tools\trmod.exe"))) {
    if (-not (Test-Path $need)) { throw "$need is missing: build Release first (build.ps1, or dist.ps1 -Build)" }
}
$v = (Get-Item $exe).VersionInfo
$version = "{0}.{1}" -f $v.ProductMajorPart, $v.ProductMinorPart
if ($v.ProductBuildPart) { $version += ".{0}" -f $v.ProductBuildPart }
$zip = Join-Path $bd "TR3-Lost-Artefact-PS1-Builder-v$version.zip"
if (Test-Path $zip) { Remove-Item $zip }
# not Compress-Archive: Windows PowerShell 5.1 writes "tools\x.exe" entry names, ZIP wants "tools/x.exe"
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
$distFull = (Resolve-Path $dist).Path.TrimEnd('\') + '\'
$archive = [System.IO.Compression.ZipFile]::Open($zip, [System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($f in Get-ChildItem $dist -Recurse -File) {
        $name = "TR3 Lost Artefact PS1 Builder/" + $f.FullName.Substring($distFull.Length).Replace('\', '/')
        [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, $f.FullName, $name,
            [System.IO.Compression.CompressionLevel]::Optimal)
    }
} finally { $archive.Dispose() }
$kb = [math]::Round((Get-Item $zip).Length / 1KB)
Write-Output "$zip ($kb KB)"
