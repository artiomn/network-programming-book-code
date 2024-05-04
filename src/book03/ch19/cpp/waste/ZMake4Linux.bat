@echo off
del ..\wastelinux.exe
winrar a -apwastelinux -m5 -mdg -s -r -sfxlinux.sfx ..\wastelinux.exe *.cpp *.hpp build\build.hpp rsa\*.cpp rsa\*.hpp -ep1 makefiles\gnu\Makefile
del ..\wastelinux.sfx
ren ..\wastelinux.exe wastelinux.sfx
