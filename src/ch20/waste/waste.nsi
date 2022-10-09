; WASTE Delivery System

; NOTE: NSIS 1.x is no longer supported! NSIS 2.x series is now very stable and must be used.
;       Many features used by this installer are not supported by NSIS 1.x.

; The defines wether to build as Full or Minimal. Full includes the support PDF. Comment this line out to build as minimal.
;!define FULL_BUILD

; Choose wether to support XP style
!define XP_STYLE_ON

; Reserve the custom welcome page for solid compression
ReserveFile "setup-welcome.ini"
ReserveFile "waste.bmp"

; Define app name variables
!define APP_NAME_BIG "WASTE"
!define APP_NAME_VER "WASTE 1.5 Beta 3"
!define APP_NAME_SMALL "WASTE" ; used for directory name and registry
;!define APP_VERSION "1.5.0" ; only first 3 (x.x.x), used to compare current installed version to check for newer builds

; Define the name of the main executable
!define APP_EXENAME "waste.exe"

; Choose the name of your UPX executable
!define HAVE_UPX "upx.exe"

; Filename of installer
!ifdef FULL_BUILD
   OutFile waste-setup-full.exe
!else
   OutFile waste-setup-mini.exe
!endif

; Installer icon
Icon "modern-install.ico"

; Uninstaller icon
UninstallIcon "modern-uninstall.ico"

; Branding text at bottom of installer
BrandingText "${APP_NAME_VER}"

; Name of bitmap which contains checkmark images
CheckBitmap "modern.bmp"

; Turn on XP Style
!ifdef XP_STYLE_ON
   XPStyle on
!endif

; Make progress bar more accurate
InstProgressFlags smooth

; Force use to accept the license via checkbox
LicenseForceSelection checkbox "I have read and understood the rights granted by the GNU GPL license."

; Installer information
VIAddVersionKey ProductName "WASTE/DS"
VIAddVersionKey FileVersion "v1.5 beta 2"
VIAddVersionKey FileDescription "WASTE Delivery System for ${APP_NAME_VER}"
VIAddVersionKey LegalCopyright "(C) 2003 Nullsoft, Inc., (C) 2004 WASTE Development Team"
VIProductVersion 1.5.0.0

; Define what pages to show
Page custom welcome
Page license
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

; Name and caption of installer
Name "${APP_NAME_VER}"
Caption "${APP_NAME_VER} - Setup"

; License info
LicenseText "WASTE is free software. Please agree to the license below to continue."
LicenseData license.txt

ComponentText "This will install ${APP_NAME_VER}"
AutoCloseWindow true
ShowInstDetails show
ShowUninstDetails show
DirText "Please select a location to install ${APP_NAME_BIG} (or use the default):"
SetOverwrite on
SetDateSave on
!ifdef HAVE_UPX
   !packhdr tmp.dat "upx --force --best tmp.dat"
!endif

; Change install directory
InstallDir $PROGRAMFILES\${APP_NAME_SMALL}
InstallDirRegKey HKLM SOFTWARE\${APP_NAME_SMALL} ""

!ifndef NOINSTTYPES ; only if not defined
   !ifdef FULL_BUILD
      InstType "Full"
   !endif
   InstType "Minimal"
   ;InstType /NOCUSTOM
   ;InstType /COMPONENTSONLYONCUSTOM
!endif

; NOTE!
; SectionSetFlags is no longer documented and doesn't seem to work correctly.
; This is not that important anyway, however, because it now uses Full vs. Minimal install instead.
; sH4RD

;Function .onInit
;  ReadRegStr $0 HKLM SOFTWARE\${APP_NAME_BIG} ""
;  StrCmp $0 "" end
;   IfFileExists $SMPROGRAMS\${APP_NAME_BIG}\*.* smthere
;     SectionSetFlags 3 0
;   smthere:
;end:
;   IfFileExists $SMSTARTUP\${APP_NAME_BIG}.lnk slthere ; by default do not do startup
;     SectionSetFlags 4 0
;   slthere:
;
;SectionSetFlags 3 0
;
;FunctionEnd



Function .onInit
   SetOutPath $TEMP
   File waste.bmp
   advsplash::show 850 375 250 -1 $TEMP\waste
   Pop $0
   Delete $TEMP\waste.bmp
; Used in future to compare version of installer to program version already installed
;   GetDllVersion "$INSTDIR\waste.exe" $R0 $R1
;   IntOp $R2 $R0 / 0x00010000
;   IntOp $R3 $R0 & 0x0000FFFF
;   IntOp $R4 $R1 / 0x00010000
;   IntOp $R5 $R1 & 0x0000FFFF
;   StrCpy $0 "$R2.$R3.$R4"
;   MessageBox MB_OK|MB_ICONSTOP "v.$0"
FunctionEnd


Function welcome
   GetTempFileName $R0
   File /oname=$R0 setup-welcome.ini
   InstallOptions::dialog $R0
   Pop $R1
   StrCmp $R1 "cancel" done
   StrCmp $R1 "back" done
   StrCmp $R1 "success" done
   MessageBox MB_OK|MB_ICONSTOP "InstallOptions error:$\r$\n$R1"
   done:
FunctionEnd

; Make sure user wants to stop install
Function .onUserAbort
   MessageBox MB_YESNO "Are you sure you wish to cancel installation?" IDYES NoCancelAbort
      Abort ; causes uninstaller to not quit.
   NoCancelAbort:
FunctionEnd

; The following Sections are installed

; Main application, installs for Full and Minimal, is required
Section "${APP_NAME_BIG} (required)"
   SectionIn 1 2 RO
   SetOutPath $INSTDIR
   DetailPrint "Copying main files..."
   File release\${APP_EXENAME}
   File release\${APP_EXENAME}.manifest
   File license.txt
   CreateDirectory $INSTDIR\Downloads
SectionEnd

; Sub Section which contains documentation, expanded by default
SubSection /e Documentation
   !ifdef FULL_BUILD
      ; PDF Version of Documentation, installs for Full
      Section "PDF Version"
         SectionIn 1
         SetOutPath $INSTDIR\Docs
         CreateDirectory $INSTDIR\Docs
         DetailPrint "Copying PDF Documentation..."
         File Docs\documentation.pdf
         SetOutPath $INSTDIR
         ; NOTE!
         ; This documentation ALWAYS installs start menu shortcuts no matter what option is chosen. This need to be fixed!
         ; sH4RD
         CreateShortCut "$SMPROGRAMS\${APP_NAME_BIG}\${APP_NAME_SMALL} Documentation (pdf).lnk" \
                   "$INSTDIR\Docs\documentation.pdf"
      SectionEnd
   !endif

   ; Link to HTML version of Documentation, installs for Full and Minimal
   Section "HTML Version (Online)"
      SectionIn 1 2
      SetOutPath $INSTDIR\Docs
      CreateDirectory $INSTDIR\Docs
      DetailPrint "Copying HTML Documentation..."
      File Docs\documentation.html
      SetOutPath $INSTDIR
     ; NOTE!
     ; This documentation ALWAYS installs start menu shortcuts no matter what option is chosen. This need to be fixed!
     ; sH4RD
     CreateShortCut "$SMPROGRAMS\${APP_NAME_BIG}\${APP_NAME_SMALL} Documentation (html).lnk" \
                   "$INSTDIR\Docs\documentation.html"
   SectionEnd

; End Sub Section Documentation
SubSectionEnd

;SubSection /e Options

   ; Installs the Start Menu shortcuts, installs for Full and Minimal
   Section "Start Menu shortcuts"
      SectionIn 1 2
      SetOutPath $INSTDIR
      DetailPrint "Creating shortcuts..."
      CreateDirectory $SMPROGRAMS\${APP_NAME_BIG}
      CreateShortCut "$SMPROGRAMS\${APP_NAME_BIG}\Uninstall ${APP_NAME_BIG}.lnk" \
                     "$INSTDIR\uninst.exe"
      CreateShortCut "$SMPROGRAMS\${APP_NAME_BIG}\${APP_NAME_BIG}.lnk" \
                     "$INSTDIR\${APP_EXENAME}"
      CreateShortCut "$SMPROGRAMS\${APP_NAME_BIG}\${APP_NAME_BIG} License.lnk" \
                     "$INSTDIR\license.txt"
      CreateShortCut "$SMPROGRAMS\${APP_NAME_BIG}\${APP_NAME_BIG} Downloads Directory.lnk" \
                     "$INSTDIR\downloads" "" "$INSTDIR\${APP_EXENAME}" 10
   SectionEnd

   ; Installs a shortcut to launch on startup, installs for Full
   Section "Launch on startup"
      SectionIn 1
      DetailPrint "Setting launch on startup..."
      CreateShortCut "$SMSTARTUP\${APP_NAME_BIG}.lnk" \
                   "$INSTDIR\${APP_EXENAME}"
   SectionEnd

;SubSectionEnd

; This runs after the section installation
Section -post

   DetailPrint "Adding registry keys..."
   WriteRegStr HKLM SOFTWARE\${APP_NAME_SMALL} "" $INSTDIR
   WriteRegStr HKCR waste "" "URL: WASTE Command Protocol"
   WriteRegStr HKCR waste "URL Protocol" ""
   WriteRegStr HKCR .wastestate "" "WASTESTATE"
   WriteRegStr HKCR WASTESTATE "" "WASTE transfer information file"
   WriteRegStr HKCR WASTESTATE\DefaultIcon "" "$INSTDIR\WASTE.exe,1"
   WriteRegStr HKCR waste\shell\open\command "" '"$INSTDIR\waste.exe" "%1"'
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "DisplayName" "${APP_NAME_BIG}"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "UninstallString" '"$INSTDIR\uninst.exe"'
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "HelpLink" "http://sourceforge.net/forum/forum.php?forum_id=281190"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "URLInfoAbout" "http://waste.sf.net"
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "NoModify" 1
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "NoRepair" 1
   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}" \
                    "Publisher" "WASTE Development Team"


  ; since the installer is now created last (in 1.2+), this makes sure 
  ; that any old installer that is readonly is overwritten.
   Delete $INSTDIR\uninst.exe 
   WriteUninstaller $INSTDIR\uninst.exe
SectionEnd

; If install is sucessful, and user profile has not been created, this executes WASTE to complete setup.
; Otherwise this prompts to run WASTE or not.
Function .onInstSuccess
  DetailPrint "Checking for previous ${APP_NAME_BIG} installs..."
  IfFileExists "$INSTDIR\default.pr0" 0 RunIt
  IfFileExists "$INSTDIR\default.pr1" 0 RunIt
  IfFileExists "$INSTDIR\default.pr2" 0 RunIt
  IfFileExists "$INSTDIR\default.pr3" 0 RunIt
  IfFileExists "$INSTDIR\default.pr4" DontRunIt RunItDetail
  RunItDetail:
  DetailPrint "New install detected. Enjoy your new ${APP_NAME_BIG} install! :D"
  RunIt:
  DetailPrint "Starting ${APP_NAME_BIG}..."
  Exec '"$INSTDIR\${APP_EXENAME}"'
  Goto Exit
  DontRunIt:
  DetailPrint "${APP_NAME_BIG} has been previously installed. Enjoy the upgrade! :D"
  MessageBox MB_YESNO|MB_ICONQUESTION "${APP_NAME_VER} installed sucessfully, would you like to start ${APP_NAME_BIG}?" IDYES RunIt
  Exit:
FunctionEnd

!ifndef NO_UNINST
UninstallText "This will uninstall ${APP_NAME_BIG} from your system:"

; This is the script for uninstallation
Section Uninstall

  DetailPrint "Uninstalling ${APP_NAME_BIG}..."

; Remove main exe, if it fails to remove prompt
TryAgain:
  SetFileAttributes $INSTDIR\${APP_EXENAME} NORMAL
  Delete $INSTDIR\${APP_EXENAME}
  IfFileExists "$INSTDIR\${APP_EXENAME}" 0 DeletedEXE
    MessageBox MB_RETRYCANCEL|MB_ICONQUESTION "${APP_EXENAME} appears to be locked, please make sure ${APP_NAME_BIG} is not running before uninstalling." IDRETRY TryAgain
    Abort "${APP_EXENAME} could not be removed. ${APP_NAME_BIG} may have been running."

; Delete other information and files
DeletedEXE:
  
  Delete $INSTDIR\readme.txt
  Delete $INSTDIR\uninst.exe
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\${APP_EXENAME}.manifest
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME_SMALL}"
  DeleteRegKey HKLM "Software\${APP_NAME_SMALL}"
  DeleteRegKey HKCR waste
  DeleteRegKey HKCR .wastestate
  DeleteRegKey HKCR WASTESTATE
  RMDir /r $SMPROGRAMS\${APP_NAME_BIG}
  Delete $SMSTARTUP\${APP_NAME_BIG}.lnk

  RMDir $INSTDIR\Downloads
  RMDir /r $INSTDIR\Docs

; If files are in the downloads dir, prompt to remove them
  IfFileExists $INSTDIR\downloads\*.* 0 NoDownloads

    MessageBox MB_YESNO|MB_ICONQUESTION "Remove any files you have downloaded (in $INSTDIR\downloads)?" IDNO NoDownloads
    Delete $INSTDIR\Downloads\*.*
    RMDir $INSTDIR\Downloads
    DetailPrint "WARNING: Downloads removed by user!"

NoDownloads:

  RMDir $INSTDIR
  
; If any files are still in the folder prompt
  MessageBox MB_YESNO|MB_ICONQUESTION "Remove network profiles from your ${APP_NAME_BIG} directory?" IDNO AllDone
    Delete $INSTDIR\*.pr?
    Delete $INSTDIR\*.ini
    DetailPrint "WARNING: Network profiles removed by user!"
  AllDone:
    RMDir $INSTDIR
    DetailPrint "Thanks for using ${APP_NAME_BIG}!"
SectionEnd
!endif