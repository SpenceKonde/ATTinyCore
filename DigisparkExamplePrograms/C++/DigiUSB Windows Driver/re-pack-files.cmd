@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

SET FILES_TO_PACK=*.inf dpinst* dpscat.* .\data* .\x86* .\amd64*
SET SFX_MODULE=7zDP_LZMA.sfx
SET SFX_MODULE_CFG=7zDP_LZMA.cfg
SET INSTALL_DRIVER_EXE=InstallDriver.exe
SET DRIVER_FILES_7Z=_DriverFiles.7z
SET 7Z_SWITCHES=-air0 -mx

SET CMD_EXIT_CODE=0

IF EXIST "!7ZA_EXE!" (SET 7Z_RUN="!7ZA_EXE!") ELSE CALL :Find7Zip 7z.exe 7za.exe 7zan.exe
IF !7Z_RUN! EQU "" (
	ECHO 7Zip is required to re-pack this installer.
	ECHO 1] Download and install 7Zip. http://www.7-zip.org/
	ECHO 2] Add the bin folder to the PATH environment variable.
	ECHO    "Control Panel->System->Advanced->Environment Variables..."
	SET CMD_EXIT_CODE=1
	GOTO Error
)

IF EXIST "!INSTALL_DRIVER_EXE!" DEL /Q "!INSTALL_DRIVER_EXE!"
IF NOT "!ERRORLEVEL!" EQU "0" (
	ECHO Access denied or file in-use "!INSTALL_DRIVER_EXE!"
	SET CMD_EXIT_CODE=2
	GOTO Error
)

IF EXIST "!DRIVER_FILES_7Z!" DEL /Q "!DRIVER_FILES_7Z!"
IF NOT "!ERRORLEVEL!" EQU "0" (
	ECHO Access denied or file in-use "!DRIVER_FILES_7Z!"
	SET CMD_EXIT_CODE=3
	GOTO Error
)

!7Z_RUN! a "!DRIVER_FILES_7Z!" !FILES_TO_PACK! !7Z_SWITCHES!
IF NOT "!ERRORLEVEL!" EQU "0" (
	ECHO Failed re-packing.  Check your 7Zip installation at
	ECHO !7Z_RUN!
	SET CMD_EXIT_CODE=4
	GOTO Error
)

COPY /B "!SFX_MODULE!"+"!SFX_MODULE_CFG!"+"!DRIVER_FILES_7Z!" "!INSTALL_DRIVER_EXE!"

ECHO. 
ECHO Done.
ECHO "!INSTALL_DRIVER_EXE!" re-packed!
GOTO :EOF

:Find7Zip
	SET 7Z_RUN="%~$PATH:1"
	IF NOT !7Z_RUN! EQU "" (
		ECHO 7Zip found at: !7Z_RUN!
		SET 7Z_RUN="%~1"
		GOTO :EOF
	)
	SHIFT /1
	IF "%~1" EQU "" GOTO :EOF
	GOTO Find7Zip
GOTO :EOF

:Error
	IF NOT DEFINED NO_REPACK_ERROR_WAIT PAUSE
	EXIT %CMD_EXIT_CODE%
GOTO :EOF