@echo off
set DEBUG_LIST=
:loop
if "%1"=="" goto end
echo %1
del ..\obj\%1.obj
shift
goto loop
:end
make %1 %2 %3 %4 %5 %6 %7 %8 %9
