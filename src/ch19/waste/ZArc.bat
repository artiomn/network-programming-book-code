@echo off
for /F "delims=" %%i in (build\version) do (
  set ver=%%i
  goto next1
)
:next1
set ARCNAME=Waste v%ver% Source
del "..\%ARCNAME%.rar"
winrar a "-ap%ARCNAME%" -m5 -mdg -s "..\%ARCNAME%.rar" @zinclude.dat -x@zexclude.dat
