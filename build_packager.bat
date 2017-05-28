@echo off

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CommonCompilerFlags=-Zi -Od
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST build mkdir build
pushd build

IF NOT EXIST assets mkdir assets

del *.pdb > NUL 2> NUL

REM 64-bit build
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\asset_packager.cpp -Fmwin32_fall.map /link %CommonLinkerFlags%
if %errorlevel% neq 0 goto :error

popd
goto :EOF

:error
popd
exit /b %errorlevel%
