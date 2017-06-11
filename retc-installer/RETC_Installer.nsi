;--------------------------------
;Constants
!define FILES ".\files"

;This would get the version number from the EXE, but the exe doesn't have a version number
;with no version number in the EXE, compile will fail
;testing this now
!ifndef VERSION
	!getdllversion "${FILES}\win64\retc-rpc-server-64.exe" vers_
	!define VERSION ${vers_1}.${vers_2}.${vers_3}.${vers_4}
!endif

;--------------------------------
;Includes
!include "MUI2.nsh"
!include LogicLib.nsh
!include x64.nsh

;--------------------------------
;Variables
Var OW64Path
Var OWT64Path
Var OW32Path
Var OWT32Path
Var "DLLName"

;--------------------------------
;General Options

; The name of the installer
Name "RETC"

Icon "${NSISDIR}\Contrib\Graphics\Icons\nsis1-install.ico"

; The file to write
OutFile "RETC_Setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\RETC

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\RETC" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
;Interface Settings
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TEXT "RETC is a piece of software that allows Corsair RGB devices to take advantage of software that supports Razer Chroma RGB devices."
!define MUI_FINISHPAGE_TEXT "Installation is complete.  Remember, RETC will only work with supported software when the RazerChroma DLL files have been placed in the software directory.  If you wish to use it with any other software, you will need to copy RzChromaSDK.dll and RzChromaSDK64.dll from the wrappers folder to that software directory as well."
!define MUI_FINISHPAGE_LINK "Visit the RETC Github Repo"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/MartB/RETC"
!define MUI_FINISHPAGE_NOAUTOCLOSE

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${FILES}\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

;--------------------------------
;Macros
!macro ReadPNFromDLL LIBRARY VAR
	MoreInfo::GetProductName "${LIBRARY}"
	Pop $1
	StrCpy ${VAR} $1
!macroend

; Return on top of stack the total size of the selected (installed) sections, formated as DWORD
; Assumes no more than 256 sections are defined
Var GetInstalledSize.total
!macro GetInstalledSize
	Push $0
	Push $1
	StrCpy $GetInstalledSize.total 0
	${ForEach} $1 0 256 + 1
		${if} ${SectionIsSelected} $1
			SectionGetSize $1 $0
			IntOp $GetInstalledSize.total $GetInstalledSize.total + $0
		${Endif}
 
		; Error flag is set when an out-of-bound section is referenced
		${if} ${errors}
			${break}
		${Endif}
	${Next}
 
	ClearErrors
	Pop $1
	Pop $0
	IntFmt $GetInstalledSize.total "0x%08X" $GetInstalledSize.total
!macroend

;--------------------------------

;--------------------------------
;Functions
Function .onInit
	${If} ${RunningX64} 
	   ; disable registry redirection (enable access to 64-bit portion of registry)
	   SetRegView 64
	   ; change install dir 
	   StrCpy $INSTDIR "$PROGRAMFILES64\RETC"
	${EndIf}
	
	ReadRegStr $0 HKLM "Software\RETC" "Install_Dir"
	${If} $0 != ""
		MessageBox MB_YESNO "RETC is already installed, would you like to re-install?" IDYES reinstall_yes IDNO reinstall_no
		reinstall_no:
		Abort
		reinstall_yes:
	${EndIf}
FunctionEnd

Function un.onInit
	${If} ${RunningX64} 
	   ; disable registry redirection (enable access to 64-bit portion of registry)
	   SetRegView 64
	   ; change install dir 
	   StrCpy $INSTDIR "$PROGRAMFILES64\RETC"
	${EndIf}
FunctionEnd

; The stuff to install
Section "RETC (required)" Sec_RETC
	SectionIn RO

	; Set output path to the installation directory.
	SetOutPath $INSTDIR

	;If nssm is already installed, attempt to stop before installing
	${If} ${FileExists} "$INSTDIR\nssm.exe"
		nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" stop RETC'
	${EndIf}
	
	${If} ${FileExists} "$SYSDIR\RzChromaSDK.dll"
		!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK.dll" $DLLName
		StrCmp $DLLName "" rzchroma_ask
		StrCmp $DLLName "RETC" rzchroma_ask
	${EndIf}
	${If} ${FileExists} "$SYSDIR\RzChromaSDK64.dll"
		!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK64.dll" $DLLName
		StrCmp $DLLName "" rzchroma_ask
		StrCmp $DLLName "RETC" rzchroma_ask
	${EndIf}
	${DisableX64FSRedirection}
	${If} ${FileExists} "$SYSDIR\RzChromaSDK.dll"
		!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK.dll" $DLLName
		StrCmp $DLLName "" rzchroma_ask
		StrCmp $DLLName "RETC" rzchroma_ask
	${EndIf}
	${If} ${FileExists} "$SYSDIR\RzChromaSDK64.dll"
		!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK64.dll" $DLLName
		StrCmp $DLLName "" rzchroma_ask
		StrCmp $DLLName "RETC" rzchroma_ask
	${EndIf}
	${EnableX64FSRedirection}

	Goto nodelete_rzchroma

	rzchroma_ask:
		${EnableX64FSRedirection}
		MessageBox MB_YESNO "Found RETC RazerChroma DLL's in the Windows system directory.$\r$\nThese should never be put here!$\r$\n$\r$\nWould you like to remove these?" IDYES delete_rzchroma IDNO nodelete_rzchroma
	delete_rzchroma:
		${If} ${FileExists} "$SYSDIR\RzChromaSDK.dll"
			!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK.dll" $DLLName
			${If} $DLLName == ""
			${OrIf} $DLLName == "RETC"
				Delete "$SYSDIR\RzChromaSDK.dll"
			${EndIf}
		${EndIf}
		${If} ${FileExists} "$SYSDIR\RzChromaSDK64.dll"
			!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK64.dll" $DLLName
			${If} $DLLName == ""
			${OrIf} $DLLName == "RETC"
				Delete "$SYSDIR\RzChromaSDK64.dll"
			${EndIf}
		${EndIf}
		${DisableX64FSRedirection}
		${If} ${FileExists} "$SYSDIR\RzChromaSDK.dll"
			!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK.dll" $DLLName
			${If} $DLLName == ""
			${OrIf} $DLLName == "RETC"
				Delete "$SYSDIR\RzChromaSDK.dll"
			${EndIf}
		${EndIf}
		${If} ${FileExists} "$SYSDIR\RzChromaSDK64.dll"
			!insertmacro ReadPNFromDLL "$SYSDIR\RzChromaSDK64.dll" $DLLName
			${If} $DLLName == ""
			${OrIf} $DLLName == "RETC"
				Delete "$SYSDIR\RzChromaSDK64.dll"
			${EndIf}
		${EndIf}
		${EnableX64FSRedirection}
	nodelete_rzchroma:
	
	${If} ${RunningX64}
		File "${FILES}\win64\retc-rpc-server-64.exe"
		File "${FILES}\win64\CUESDK.x64_2015.dll"
	${Else}
		File "${FILES}\win32\retc-rpc-server-32.exe"
		File "${FILES}\win32\CUESDK_2015.dll"
	${EndIf}
	
	File "${FILES}\win32\nssm.exe"
	File "${FILES}\win64\nssm.exe"
	File "${FILES}\LICENSE"
	File "${FILES}\README.md"
	
	SetOutPath $INSTDIR\wrappers
	File "${FILES}\win32\RzChromaSDK.dll"
	File "${FILES}\win64\RzChromaSDK64.dll"
	
	${If} ${FileExists} "$INSTDIR\RzChromaSDK.dll"
		Delete "$INSTDIR\RzChromaSDK.dll"
	${EndIf}
	${If} ${FileExists} "$INSTDIR\RzChromaSDK64.dll"
		Delete "$INSTDIR\RzChromaSDK64.dll"
	${EndIf}
	
	WriteINIStr "$INSTDIR\RETC Github Repo.URL" "InternetShortcut" "URL" "https://github.com/MartB/RETC"

	${If} ${RunningX64} 
		nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" install RETC "$INSTDIR\retc-rpc-server-64.exe"'
	${Else}
		nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" install RETC "$INSTDIR\retc-rpc-server-32.exe"'
	${EndIf}
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppDirectory "$INSTDIR"'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppExit Default Restart'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppNoConsole 1'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppStdout "$INSTDIR\retc-server-nssm.log"'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppStdoutCreationDisposition 2'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppStderr "$INSTDIR\retc-server-nssm.log"'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC AppStderrCreationDisposition 2'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC Description "Allows programs that support the Razer Chroma SDK to use Corsair RGB devices.  Visit https://github.com/MartB/RETC for more info."'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC DisplayName "RETC Service"'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC ObjectName LocalSystem'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC Start SERVICE_AUTO_START'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" set RETC Type SERVICE_WIN32_OWN_PROCESS'

	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" start RETC'

	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\RETC "Install_Dir" "$INSTDIR"

	; Write the uninstall keys for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "DisplayName" "RETC"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "Publisher" "RETC Project"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "URLUpdateInfo" "https://github.com/MartB/RETC/releases"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "URLInfoAbout" "https://martb.github.io/RETC/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "HelpLink" "https://github.com/MartB/RETC/issues"
	!ifdef VERSION
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "DisplayVersion" "${VERSION}"
	!endif
	!insertmacro GetInstalledSize
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "EstimatedSize" $GetInstalledSize.total

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC" "NoRepair" 1
	WriteUninstaller "uninstall.exe"

SectionEnd

Section "Overwatch Support" Sec_OW

	ReadRegStr $OW64Path HKLM "SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch" InstallLocation
	ReadRegStr $OWT64Path HKLM "SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch Test" InstallLocation
	ReadRegStr $OW32Path HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch" InstallLocation
	ReadRegStr $OWT32Path HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch Test" InstallLocation

	; Set output path to the installation directory.
	${If} $OW64Path != ""
		${If} ${FileExists} "$OW64Path"
			SetOutPath $OW64Path
			File "${FILES}\win32\RzChromaSDK.dll"
			File "${FILES}\win64\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}
	${If} $OWT64Path != ""
		${If} ${FileExists} "$OWT64Path"
			SetOutPath $OWT64Path
			File "${FILES}\win32\RzChromaSDK.dll"
			File "${FILES}\win64\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}
	${If} $OW32Path != ""
		${If} ${FileExists} "$OW32Path"
			SetOutPath $OW32Path
			File "${FILES}\win32\RzChromaSDK.dll"
			File "${FILES}\win64\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}
	${If} $OWT32Path != ""
		${If} ${FileExists} "$OWT32Path"
			SetOutPath $OWT32Path
			File "${FILES}\win32\RzChromaSDK.dll"
			File "${FILES}\win64\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" Sec_SM
	CreateDirectory "$SMPROGRAMS\RETC"
	CreateShortCut "$SMPROGRAMS\RETC\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\RETC\RETC Github Repo.lnk" "$INSTDIR\RETC Github Repo.URL" "" \
		"$SYSDIR\SHELL32.DLL" 13
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

	; Remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\RETC"
	DeleteRegKey HKLM "SOFTWARE\RETC"
	
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" stop RETC'
	nsExec::ExecToLog /OEM '"$INSTDIR\nssm.exe" remove RETC confirm'

	; Remove files and uninstaller
	Delete "$INSTDIR\*.*"

	; Remove shortcuts, if any
	Delete "$SMPROGRAMS\RETC\*.*"

	; Remove directories used
	RMDir "$SMPROGRAMS\RETC"
	RMDir "$INSTDIR"

SectionEnd

Section "un.Remove Overwatch Support Files" unSec_OW

	ReadRegStr $OW64Path HKLM "SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch" InstallLocation
	ReadRegStr $OWT64Path HKLM "SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch Test" InstallLocation
	ReadRegStr $OW32Path HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch" InstallLocation
	ReadRegStr $OWT32Path HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Overwatch Test" InstallLocation

	; Set output path to the installation directory.
	${If} $OW64Path != ""
		${If} ${FileExists} "$OW64Path"
			Delete "$OW64Path\RzChromaSDK.dll"
			Delete "$OW64Path\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}
	${If} $OWT64Path != ""
		${If} ${FileExists} "$OWT64Path"
			Delete "$OWT64Path\RzChromaSDK.dll"
			Delete "$OWT64Path\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}
	${If} $OW32Path != ""
		${If} ${FileExists} "$OW32Path"
			Delete "$OW32Path\RzChromaSDK.dll"
			Delete "$OW32Path\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}
	${If} $OWT32Path != ""
		${If} ${FileExists} "$OWT32Path"
			Delete "$OWT32Path\RzChromaSDK.dll"
			Delete "$OWT32Path\RzChromaSDK64.dll"
		${EndIf}
	${EndIf}

SectionEnd


LangString DESC_Sec_RETC ${LANG_ENGLISH} "The main RETC service files, as well as the RETC service."
LangString DESC_Sec_OW ${LANG_ENGLISH} "This will copy the Razer Chroma DLL files to any Overwatch installations that are found."
LangString DESC_Sec_SM ${LANG_ENGLISH} "Install start menu shortcuts"
LangString DESC_Un_Sec_OW ${LANG_ENGLISH} "Remove Razer Chroma DLL files from any Overwatch installations that are found."


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_RETC} $(DESC_Sec_RETC)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_OW} $(DESC_Sec_OW)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_SM} $(DESC_Sec_SM)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_OW} $(DESC_Un_Sec_OW)
!insertmacro MUI_FUNCTION_DESCRIPTION_END