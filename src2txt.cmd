@echo off
setlocal enabledelayedexpansion
REM ===============================================================================
REM src2txt.cmd — Export project source into a single text file for AI ingestion
REM ===============================================================================
REM Usage:
REM    src2txt.cmd [ProjectName]
REM
REM Description:
REM    Combines README.md and all relevant source files into one text file.
REM    Creates a directory tree listing and includes each file's content.
REM    Automatically backs up old output files with timestamps.
REM
REM Arguments:
REM    ProjectName    Set project name (default: default-project)
REM
REM ===============================================================================

REM --- Configuration ---
set "PROJECTNAME=%~1"
if "%PROJECTNAME%"=="" set "PROJECTNAME=default-project"

set "SRC_TEXT_FOLDER=..\src_text"
set "SRC_BACKUP_FOLDER=..\backup"
set "OUTPUT_FILE=%~dp0%PROJECTNAME%.txt"

REM Folders to exclude
set "EXCLUDE_FOLDERS=.git build .qtcreator .venv node_modules bin obj Debug Release"

REM Check if running from explorer (not a terminal)
set "RAN_FROM_EXPLORER=0"
echo %CMDCMDLINE% | findstr /i /c:"%~nx0" >nul
if not errorlevel 1 (
    echo %CMDCMDLINE% | findstr /i /c:"cmd /c" >nul
    if not errorlevel 1 set "RAN_FROM_EXPLORER=1"
)

REM --- Start ---
echo ===============================================================================
echo Project: %PROJECTNAME%
echo Generated on: %date% %time%
echo ===============================================================================
echo.

REM --- Create backup directories ---
if not exist "%SRC_BACKUP_FOLDER%" mkdir "%SRC_BACKUP_FOLDER%"

REM --- Backup old output file ---
if exist "%OUTPUT_FILE%" (
    REM Get date and time in YYYY-MM-DD_HH.MM.SS format using PowerShell
    for /f "tokens=*" %%i in ('powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd_HH.mm.ss'"') do set "TIMESTAMP=%%i"
    set "BACKUP_FILE=%SRC_BACKUP_FOLDER%\%PROJECTNAME%-!TIMESTAMP!.txt"
    copy "%OUTPUT_FILE%" "!BACKUP_FILE!" >nul
    echo Backup created: !BACKUP_FILE!
)

REM --- Backup old src_text folder ---
if exist "%SRC_TEXT_FOLDER%" (
    REM Get date and time in YYYY-MM-DD_HH.MM.SS format using PowerShell
    for /f "tokens=*" %%i in ('powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd_HH.mm.ss'"') do set "TIMESTAMP=%%i"
    set "TARGET_BACKUP=%SRC_BACKUP_FOLDER%\src_text-!TIMESTAMP!"
    move "%SRC_TEXT_FOLDER%" "!TARGET_BACKUP!" >nul 2>&1
    if exist "!TARGET_BACKUP!" echo Old src_text files moved to: !TARGET_BACKUP!
)

REM --- Create src_text folder ---
if not exist "%SRC_TEXT_FOLDER%" mkdir "%SRC_TEXT_FOLDER%"

REM --- Start writing output file ---
(
    echo #===============================================================================
    echo # Project: %PROJECTNAME%
    echo # Generated on: %date% %time%
    echo # OS: Windows on %OS%
    echo # This file is used to show full source code, cmake, read me, and file locates.
    echo #===============================================================================
    echo.
) > "%OUTPUT_FILE%"

REM --- Process files ---
echo Processing files...
for /r %%F in (*.cpp *.h *.hpp *.ui *.qrc *.sh *.cmd CMakeLists.txt README.md) do (
    set "FILEPATH=%%F"
    set "SKIP=0"
    
    REM Check if file is in excluded folder
    for %%E in (%EXCLUDE_FOLDERS%) do (
        echo !FILEPATH! | find "\%%E\" >nul
        if !errorlevel! equ 0 set "SKIP=1"
    )
    
    REM Check if file exists and is not empty
    if exist "%%F" (
        for %%A in ("%%F") do set "FILESIZE=%%~zA"
        if !FILESIZE! equ 0 set "SKIP=1"
    ) else (
        set "SKIP=1"
    )
    
    if !SKIP! equ 0 (
        set "RELPATH=%%F"
        set "RELPATH=!RELPATH:%~dp0=!"
        
        REM Copy to src_text folder
        set "FILENAME=%%~nxF"
        copy "%%F" "%SRC_TEXT_FOLDER%\!FILENAME!.txt" >nul 2>&1
        
        REM Append to main output
        (
            echo #-------------------------------------------------------------------------------
            echo # File: !RELPATH!
            echo #-------------------------------------------------------------------------------
            type "%%F"
            echo.
        ) >> "%OUTPUT_FILE%"
        
        echo   Processed: !RELPATH!
    )
)

REM --- Generate directory tree ---
echo.
echo Generating directory tree...
echo # Directory Structure >> "%OUTPUT_FILE%"
echo #------------------------------------------------------------------------------- >> "%OUTPUT_FILE%"

REM Use PowerShell to generate tree with exclusions
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$excludes = @('%EXCLUDE_FOLDERS: =','%'.Split(','));" ^
    "function Show-Tree {" ^
    "    param([string]$Path = '.', [string]$Prefix = '', [bool]$IsLast = $true);" ^
    "    $items = Get-ChildItem -Path $Path -Force | Where-Object {" ^
    "        $item = $_;" ^
    "        $excluded = $false;" ^
    "        foreach ($ex in $excludes) {" ^
    "            if ($item.Name -eq $ex) { $excluded = $true; break; }" ^
    "        }" ^
    "        -not $excluded" ^
    "    } | Sort-Object {$_.PSIsContainer}, Name -Descending;" ^
    "    $count = $items.Count;" ^
    "    for ($i = 0; $i -lt $count; $i++) {" ^
    "        $item = $items[$i];" ^
    "        $isLast = ($i -eq ($count - 1));" ^
    "        $connector = if ($isLast) { '└──' } else { '├──' };" ^
    "        $line = $Prefix + $connector + ' ' + $item.Name;" ^
    "        Write-Output $line;" ^
    "        if ($item.PSIsContainer) {" ^
    "            $newPrefix = $Prefix + $(if ($isLast) { '    ' } else { '│   ' });" ^
    "            Show-Tree -Path $item.FullName -Prefix $newPrefix -IsLast $isLast;" ^
    "        }" ^
    "    }" ^
    "}" ^
    "Show-Tree" >> "%OUTPUT_FILE%"

REM --- Finalize ---
(
    echo.
    echo #*** End of file %PROJECTNAME% ***
) >> "%OUTPUT_FILE%"

echo.
echo ===============================================================================
echo Output written to: %OUTPUT_FILE%
echo Individual file copies written to: %SRC_TEXT_FOLDER%
echo ===============================================================================

REM --- Pause if run from Explorer ---
if %RAN_FROM_EXPLORER% equ 1 (
    echo.
    echo Press any key to exit...
    pause >nul
)

endlocal
REM End of script src2txt.cmd