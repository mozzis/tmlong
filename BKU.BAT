@if '%1'=='' goto useage
@xcopy main %1\project\main
@xcopy inc  %1\project\inc
@xcopy run  %1\project\run
@xcopy .    %1\project
@goto done
:useage
@be beep
@echo Type %0 [drive] (e.g. a:)
:done
