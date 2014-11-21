::%comspec% /k ""D:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"" x86
::call "D:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
call "D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

set PATH=%PATH%;E:\chromium\depot_tools_20130620
::set PATH=%PATH%;C:\Cygwin\bin
set WEBKITOUTPUTDIR=F:\webkit-windows\WebKit-r57134\out-r57134
set WEBKITLIBRARIESDIR=F:\svn_beijing\webkit-r55397\Branches\WebKit-r57134\WebKitLibraries\win
set HIPPO_GLUE_PATH=F:\svn_beijing\webkit-r55397\Branches\WebKit-r57134\WebCore\Hippo_glue
set HIPPO_PATH=F:\svn_beijing\ChinaTC_IPTV20\Branches\Experiment

@echo off
pushd %~dp0\..

python build\gyp_chromium rhino\rhino.gyp --depth=. && devenv rhino\rhino.sln /log