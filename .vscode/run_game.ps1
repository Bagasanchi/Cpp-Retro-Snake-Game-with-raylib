Set-Location (Resolve-Path (Join-Path $PSScriptRoot ".."))

& "C:/raylib/w64devkit/bin/mingw32-make.exe" RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=*.cpp BUILD_MODE=DEBUG
if ($LASTEXITCODE -eq 0) {
    .\main.exe
}
