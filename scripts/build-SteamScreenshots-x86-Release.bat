@echo off
setlocal
set "Configuration=Release"
call "%~dp0build-SteamScreenshots-x86.bat" %*
exit /b %errorlevel%
