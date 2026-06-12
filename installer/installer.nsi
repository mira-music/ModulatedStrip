; Modulated Strip VST Installer
; NSIS Script

!define PRODUCT_NAME "Modulated Strip"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Modulated"
!define PRODUCT_WEB_SITE "https://www.instagram.com/modulated_ofc/"
!define INSTALL_DIR "$PROGRAMFILES64\Common Files\VST3"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\ModulatedStrip"

;──────────────────────────────────────────────
; Modern UI
;──────────────────────────────────────────────
!include "MUI2.nsh"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "ModulatedStrip_Setup_v${PRODUCT_VERSION}.exe"
InstallDir "${INSTALL_DIR}"
RequestExecutionLevel admin
SetCompressor /SOLID lzma

;──────────────────────────────────────────────
; Interface settings
;──────────────────────────────────────────────
!define MUI_ABORTWARNING
!define MUI_ICON "..\icons\ModulatedStrip.ico"
!define MUI_UNICON "..\icons\ModulatedStrip.ico"

;──────────────────────────────────────────────
; Pages
;──────────────────────────────────────────────
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;──────────────────────────────────────────────
; Languages
;──────────────────────────────────────────────
!insertmacro MUI_LANGUAGE "English"

;──────────────────────────────────────────────
; Installer sections
;──────────────────────────────────────────────
Section "Modulated Strip VST3" SecMain

    SectionIn RO

    SetOutPath "$INSTDIR\Modulated Strip.vst3\Contents\x86_64-win"

    File "..\build\ModulatedStrip_artefacts\Release\VST3\Modulated Strip.vst3\Contents\x86_64-win\Modulated Strip.vst3"

    SetOutPath "$INSTDIR\Modulated Strip.vst3\Contents\Resources"
    File /nonfatal "..\build\ModulatedStrip_artefacts\Release\VST3\Modulated Strip.vst3\Contents\Resources\*.*"

    WriteUninstaller "$INSTDIR\Modulated Strip.vst3\Uninstall.exe"

    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr HKLM "${UNINSTALL_KEY}" "UninstallString" \
        '"$INSTDIR\Modulated Strip.vst3\Uninstall.exe"'
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair" 1

    CreateDirectory "$APPDATA\ModulatedStrip"
    CreateDirectory "$APPDATA\ModulatedStrip\Presets"

    DetailPrint "Modulated Strip installed to:"
    DetailPrint "$INSTDIR\Modulated Strip.vst3"
    DetailPrint ""
    DetailPrint "Rescan plugins in your DAW to use it."

SectionEnd

;──────────────────────────────────────────────
; Uninstaller
;──────────────────────────────────────────────
Section "Uninstall"

    RMDir /r "$PROGRAMFILES64\Common Files\VST3\Modulated Strip.vst3"
    DeleteRegKey HKLM "${UNINSTALL_KEY}"

    DetailPrint "User presets preserved in:"
    DetailPrint "$APPDATA\ModulatedStrip"

SectionEnd

;──────────────────────────────────────────────
; Version info
;──────────────────────────────────────────────
VIProductVersion "${PRODUCT_VERSION}.0"
VIAddVersionKey "ProductName"     "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName"     "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright"  "Copyright 2026 ${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion"     "${PRODUCT_VERSION}"
VIAddVersionKey "ProductVersion"  "${PRODUCT_VERSION}"
