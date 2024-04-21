@echo off
setlocal

set WindowsFiles=%CD%\platform\windows\*.cc
set SourceFiles=%WindowsFiles% %CD%\*.cc 
set OutputDir=build

set DBG=
set DBG=%DBG% -MDd 
set DBG=%DBG% -DDEBUG
set DBG=%DBG% -DLOGS
set DBG=%DBG% -DFPS_COUNTER

set NOWARN=-Wno-vla-cxx-extension

set CC=clang-cl
set STD=-std:c++latest
set CFLAGS=%STD% -W3 -Odi -Zi -EHsc -nologo %DBG% %NOWARN%
set LDFLAGS=-opt:ref -debug:full -subsystem:console user32.lib gdi32.lib opengl32.lib
set BIN=run.exe

if not exist %OutputDir% mkdir %OutputDir%
pushd %OutputDir%
@echo on
call %CC% %CFLAGS% %SourceFiles% -link %LDFLAGS% -out:%BIN% || exit /b
@echo off
popd

ctags -R --language-force=C++ --extras=+q+r --c++-kinds=+p+l+x+L+A+N+U+Z

:: call %OutputDir%\%BIN%
