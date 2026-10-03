@echo off
setlocal
set "Configuration=Debug"
call "%~dp0build-SteamScreenshots-x86.bat" %*
exit /b %errorlevel%
