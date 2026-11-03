# Builds the builder with Visual Studio 2026. Result: <BuildDir>\<Config>\
#   -Config Debug|Release   -BuildDir <folder> (default build)   -Groups "anim;geom" (level tool groups, default all)
param([string]$Config = "Release", [string]$BuildDir = "build", [string]$Groups = "all")
$cmake = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$root = $PSScriptRoot
$bd = if ([System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir } else { Join-Path $root $BuildDir }
& $cmake -S $root -B $bd -G "Visual Studio 18 2026" -A x64 "-DLEVELTOOL_GROUPS=$Groups" | Out-Null
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $cmake --build $bd --config $Config -- /m /v:m /nologo
exit $LASTEXITCODE
