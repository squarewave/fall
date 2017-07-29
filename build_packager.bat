@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CommonCompilerFlags=-Zi -Od
set CommonLinkerFlags=-PDB:preprocessor.pdb -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL

REM 64-bit build
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\preprocessor.cpp /link %CommonLinkerFlags%
if %errorlevel% neq 0 goto :error

popd

build\preprocessor.exe platform.h assets.h > "packager_typeinfo.h"

pushd build

set CommonCompilerFlags=-Zi -Od -DFALL_INTERNAL
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST assets mkdir assets

del *.pdb > NUL 2> NUL

REM 64-bit build
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\asset_packager.cpp /link %CommonLinkerFlags%
if %errorlevel% neq 0 goto :error

popd

call "build\asset_packager.exe"
echo "packaged"

goto :EOF

:error
echo "Error!"
popd
exit /b %errorlevel%
