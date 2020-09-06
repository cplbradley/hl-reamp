
setlocal

rem ================================
rem ==== MOD PATH CONFIGURATIONS ===

rem == Set the absolute path to your mod's game directory here ==
set GAMEDIR= D:\Steam\STEAMA~1\common\SO1FC0~1\sdk\SOURCE~1\sp\game\mod_hl2\

rem == Set the relative or absolute path to Source SDK Base 2013 Singleplayer\bin ==
set SDKBINDIR=D:\Steam\STEAMA~1\common\SOURCE~3\bin

rem ==  Set the Path to your mod's root source code ==
rem This should already be correct, accepts relative paths only!
set SOURCEDIR=..\..

rem ==== MOD PATH CONFIGURATIONS END ===
rem ====================================


call buildsdkshaders.bat
