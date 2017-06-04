@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set CommonCompilerFlags=-Zi -Od /I "..\libs\include" -DPLATFORM_WINDOWS -DRENDERER_OPENGL -DFALL_INTERNAL
set CommonLinkerFlags= -libpath:"..\libs\bin" -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib SDL2main.lib SDL2.lib openGL32.lib glew32.lib Judy.lib
set GameExports=-EXPORT:game_update_and_render -EXPORT:game_imgui_get_io -EXPORT:game_imgui_new_frame -EXPORT:game_imgui_shutdown -EXPORT:game_imgui_render -EXPORT:game_imgui_get_tex_data_as_rgba32 -EXPORT:game_debug_end_frame

IF NOT EXIST build mkdir build
pushd build

IF NOT EXIST shaders mkdir shaders

del *.pdb > NUL 2> NUL

REM 64-bit build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% -LD ..\fall_game.cpp -Fmfall_game.map /link -incremental:no -opt:ref -PDB:fall_game_%random%.pdb %GameExports% -libpath:"..\libs\bin" Judy.lib
del lock.tmp
cl %CommonCompilerFlags% -DTRANSLATION_UNIT_INDEX=2 ..\fall.cpp -Fmfall.map /link %CommonLinkerFlags%
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
