@echo off
echo Copying Modulated Strip VST3...

set "BUILD_DIR=%~dp0build"
set "VST3_DIR=%PROGRAMFILES%\Common Files\VST3"

if not exist "%BUILD_DIR%\ModulatedStrip_artefacts\Release\VST3\Modulated Strip.vst3" (
    echo ERROR: Build output not found at:
    echo   %BUILD_DIR%\ModulatedStrip_artefacts\Release\VST3\Modulated Strip.vst3
    echo Please build the plugin first.
    pause
    exit /b 1
)

xcopy /s /i /y /q "%BUILD_DIR%\ModulatedStrip_artefacts\Release\VST3\Modulated Strip.vst3" "%VST3_DIR%\Modulated Strip.vst3"

echo Done. Plugin copied to VST3 folder.
pause