!include MUI2.nsh

; General
Name "Subtitle Composer"
OutFile "SubtitleComposerSetup.exe"

SetCompressor /SOLID lzma

InstallDir "$PROGRAMFILES\SubtitleComposer"
InstallDirRegKey HKCU "Software\SubtitleComposer" ""
RequestExecutionLevel admin

; Interface
!define MUI_ABORTWARNING

; Pages
!insertmacro MUI_PAGE_LICENSE "../LICENSE"
;!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"
!define MUI_PAGE_HEADER_TEXT "Subtitle Composer Installation"
!define MUI_PAGE_HEADER_SUBTEXT "Subtitle Composer - KF5/Qt Video Subtitle Editor"

; Installer
Section "StartMenu" SecStartMenu
	CreateDirectory "$SMPROGRAMS\Subtitle Composer"
	CreateShortcut "$SMPROGRAMS\Subtitle Composer\Subtitle Composer.lnk" "$INSTDIR\bin\SubtitleComposer.exe"
	CreateShortcut "$SMPROGRAMS\Subtitle Composer\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Installation" SecInstall
	SetOutPath "$INSTDIR"
	File /r "{BUILD_PATH}/*"

	; Store installation folder
	WriteRegStr HKCU "Software\SubtitleComposer" "" $INSTDIR
	
	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

LangString DESC_SecStartMenu ${LANG_ENGLISH} "Start Menu Shortcuts"
LangString DESC_SecInstall ${LANG_ENGLISH} "Subtitle Composer Application"
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
!insertmacro MUI_DESCRIPTION_TEXT ${SecInstall} $(DESC_SecInstall)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Uninstaller
Section "Uninstall"
	RMDir /r "$INSTDIR"
	RMDir /r "$SMPROGRAMS\SubtitleComposer"
	DeleteRegKey HKCU "Software\SubtitleComposer"
SectionEnd
