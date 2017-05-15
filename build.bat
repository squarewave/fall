@echo off

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CommonCompilerFlags=-Zi -Od /I "..\libs\include"
set CommonLinkerFlags= -libpath:"..\libs\bin" -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib SDL2main.lib SDL2.lib openGL32.lib glew32.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL

REM 64-bit build
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\fall.cpp -Fmwin32_fall.map /link %CommonLinkerFlags%
if %errorlevel% neq 0 goto :error

cp ..\libs\bin\SDL2.dll .
cp ..\libs\bin\glew32.dll .
popd
goto :EOF

:error
popd
exit /b %errorlevel%
