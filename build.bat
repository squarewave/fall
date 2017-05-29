@echo off

rem call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CommonCompilerFlags=-Zi -Od /I "..\libs\include" -DPLATFORM_WINDOWS -DRENDERER_OPENGL
set CommonLinkerFlags= -libpath:"..\libs\bin" -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib SDL2main.lib SDL2.lib openGL32.lib glew32.lib Judy.lib

IF NOT EXIST build mkdir build
pushd build

IF NOT EXIST shaders mkdir shaders

del *.pdb > NUL 2> NUL

REM 64-bit build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% -LD ..\fall.cpp -Fmfall.map /link -incremental:no -opt:ref -PDB:fall_%random%.pdb -EXPORT:game_update_and_render -libpath:"..\libs\bin" Judy.lib
del lock.tmp
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\platform.cpp -Fmwin32_fall.map /link %CommonLinkerFlags%
if %errorlevel% neq 0 goto :error

cp ../libs/bin/SDL2.dll .
cp ../libs/bin/Judy.dll .
cp ../libs/bin/glew32.dll .
cp ../shaders/* ./shaders
popd
goto :EOF

:error
popd
exit /b %errorlevel%
