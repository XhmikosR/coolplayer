@ECHO OFF
rem *
rem * make_packages.bat, batch file for building Regshot with WDK
rem * and creating the zip packages
rem *
rem * Copyright (C) 2010-2011 XhmikosR
rem *
rem * This program is free software; you can redistribute it and/or modify
rem * it under the terms of the GNU General Public License as published by
rem * the Free Software Foundation; either version 2 of the License, or
rem * (at your option) any later version.
rem *
rem * This program is distributed in the hope that it will be useful,
rem * but WITHOUT ANY WARRANTY; without even the implied warranty of
rem * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem * GNU General Public License for more details.
rem *
rem * You should have received a copy of the GNU General Public License
rem * along with this program; if not, write to the Free Software
rem * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


SETLOCAL
CD /D %~dp0

SET CPVER=1.1.0.220

CALL "build_wdk.bat"

CALL :SubZipFiles Release_x86 x86
CALL :SubZipFiles Release_x64 x64


:END
TITLE Finished!
ECHO.
ENDLOCAL
PAUSE
EXIT /B


:SubZipFiles
TITLE Creating the %2 ZIP file...
CALL :SUBMSG "INFO" "Creating the %2 ZIP file..."

IF NOT EXIST "temp_zip" MD "temp_zip"
COPY /Y /V "..\coolplayer\res\changes.txt" "temp_zip\Changes.txt"
COPY /Y /V "..\coolplayer\res\readme.txt"  "temp_zip\Readme.txt"
COPY /Y /V "..\bin\WDK\%1\coolplayer.exe"  "temp_zip\"

PUSHD "temp_zip"
START "" /B /WAIT "..\7za.exe" a -tzip -mx=9 "CoolPlayer_%CPVER%_%2_WDK.zip" >NUL
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"

CALL :SUBMSG "INFO" "CoolPlayer_%CPVER%_%2_WDK.zip created successfully!"

MOVE /Y "CoolPlayer_%CPVER%_%2_WDK.zip" "..\" >NUL
POPD
IF EXIST "temp_zip" RD /S /Q "temp_zip"
EXIT /B


:SUBMSG
ECHO. & ECHO ______________________________
ECHO [%~1] %~2
ECHO ______________________________ & ECHO.
IF /I "%~1"=="ERROR" (
  PAUSE
  EXIT
) ELSE (
  EXIT /B
)
