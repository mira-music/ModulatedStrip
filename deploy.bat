@echo off
echo Copying Modulated Strip VST3...

xcopy /s /i /y /q "C:\Projects\ModulatedStrip\build\ModulatedStrip_artefacts\Release\VST3\Modulated Strip.vst3" "C:\Program Files\Common Files\VST3\Modulated Strip.vst3"

echo Done. Plugin copied to VST3 folder.
pause