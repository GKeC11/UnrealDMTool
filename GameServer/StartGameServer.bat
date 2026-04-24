@echo off
setlocal

cd /d "%~dp0"
title NoOutsiders GameServer

echo [GameServer] Working directory: %CD%
echo [GameServer] Default URL: http://127.0.0.1:7788
echo.

where node >nul 2>nul
if errorlevel 1 (
    echo [GameServer] ERROR: Node.js was not found in PATH.
    echo [GameServer] Please install Node.js 18 or newer, then try again.
    echo.
    pause
    exit /b 1
)

where npm >nul 2>nul
if errorlevel 1 (
    echo [GameServer] ERROR: npm was not found in PATH.
    echo [GameServer] Please install Node.js 18 or newer, then try again.
    echo.
    pause
    exit /b 1
)

call npm start
set EXIT_CODE=%ERRORLEVEL%

echo.
if not "%EXIT_CODE%"=="0" (
    echo [GameServer] Exited with code %EXIT_CODE%.
) else (
    echo [GameServer] Stopped.
)

pause
exit /b %EXIT_CODE%
