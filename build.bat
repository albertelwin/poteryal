@echo off

IF NOT EXIST bin mkdir bin
pushd bin

set COMMON_COMPILER_FLAGS=-nologo -FePoteryal -EHa- -Gm- -GR- -fp:fast -Oi -WX -W4 -wd4996 -wd4100 -wd4189 -wd4127 -wd4201 -DWIN32=1 -DDEBUG_MODE=1
set COMMON_LINKER_FLAGS=-incremental:no -opt:ref

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% -MDd -Od -Z7
rem set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% -MT -O2

del *.pdb > NUL 2> NUL

echo "compiling..." > lock
cl %COMPILER_FLAGS% -I../src/ ../src/poteryal.cpp -LD opengl32.lib -link %COMMON_LINKER_FLAGS% -PDB:Poteryal%random%.pdb -EXPORT:game_update_and_render
del "lock"

cl %COMPILER_FLAGS% -I../src/ ../src/win32_main.cpp user32.lib gdi32.lib winmm.lib opengl32.lib -link %COMMON_LINKER_FLAGS%

popd
