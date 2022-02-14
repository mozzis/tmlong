@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
echo %1
del ..\obj\%1.obj
shift
goto loop
:end
make -DDEBUG ..\run\admsr.exe
