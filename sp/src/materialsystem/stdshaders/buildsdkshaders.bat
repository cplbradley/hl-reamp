@echo off
setlocal

rem Use dynamic shaders to build .inc files only
rem set dynamic_shaders=1
rem == Setup path to nmake.exe, from vc 2005 common tools directory ==
call "%VS120COMNTOOLS%vsvars32.bat"


set TTEXE=..\..\devtools\bin\timeprecise.exe
if not exist %TTEXE% goto no_ttexe
goto no_ttexe_end

:no_ttexe
set TTEXE=time /t
:no_ttexe_end


rem echo.
rem echo ~~~~~~ buildsdkshaders %* ~~~~~~
%TTEXE% -cur-Q
set tt_all_start=%ERRORLEVEL%
set tt_all_chkpt=%tt_start%

set BUILD_SHADER=call buildshaders.bat
set ARG_EXTRA=
//%BUILD_SHADER% pbr_dx9_30			-game %GAMEDIR% -source %SOURCEDIR% -dx9_30	-force30 
//%BUILD_SHADER% core_dx9_20b			-game %GAMEDIR% -source %SOURCEDIR% -dx9_20
//%BUILD_SHADER% wigglywater			-game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30
//%BUILD_SHADER% alphacable			-game %GAMEDIR% -source %SOURCEDIR% -dx9_20b
//%BUILD_SHADER% taa_dx9_30			-game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30

//%BUILD_SHADER% simplemotionblur			-game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30

//%BUILD_SHADER% nightvision_enemyoverlay		-game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30
//%BUILD_SHADER% unlitstatic -game %GAMEDIR% -source %SOURCEDIR% -dx9_20b
%BUILD_SHADER% nvpostprocess -game %GAMEDIR% -source %SOURCEDIR% -dx9_30 -force30
//%BUILD_SHADER% viewprojection -game %GAMEDIR% -source %SOURCEDIR% -dx9_20


rem echo.
if not "%dynamic_shaders%" == "1" (
  rem echo Finished full buildallshaders %*
) else (
  rem echo Finished dynamic buildallshaders %*
)

rem %TTEXE% -diff %tt_all_start% -cur
rem echo.
