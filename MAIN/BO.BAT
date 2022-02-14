@if '%1'=='' goto noarg
@if '%2'=='' goto onearg
b \code\oma4000\main\%1 \code\oma4000\main\%2
@goto noarg
:onearg
b \code\oma4000\main\%1
:noarg
