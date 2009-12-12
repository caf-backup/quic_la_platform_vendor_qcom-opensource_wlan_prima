::Current directory should be cfgUtil

@echo off
setlocal
set src=cfgGen.c
set VC98=c:\Program Files\Microsoft Visual Studio\VC98
set PATH=%VC98%\Bin;%PATH%
set INCLUDE=%VC98%\ATL\include;%VC98%\mfc\include;%VC98%\include
set LIB=%VC98%\mfc\lib;%VC98%\lib
set FLAGS=/FD /Gd /D"_CONSOLE" /D"_MBCS" /D"Win32" /O2 /GX

if NOT exist %src% (
    echo Error!! Please run from the directory containing this batch file
    goto Exit
)

@echo on

cl %src% /I .. /I %CRT_INC_PATH% %FLAGS% /link /subsystem:console /Libpath:"!lib!"

@if NOT exist cfgGen.exe (
    echo Error!! Unable to compile configuration utility: cfgGen.exe
)

@set dest=..\..\..\inc

@if exist ..\cfgParamname.c    (del /q /f ..\cfgParamName.c)
@if exist %dest%\cfg.dat       (del /q /f %dest%\cfg.dat)
@if exist %dest%\wniCfgAp.h    (del /q /f %dest%\wniCfgAp.h)
@if exist %dest%\wniCfgAp.bin  (del /q /f %dest%\wniCfgAp.bin)
@if exist %dest%\wniCfgSta.h   (del /q /f %dest%\wniCfgSta.h)
@if exist %dest%\wniCfgSta.bin (del /q /f %dest%\wniCfgSta.bin)

cfgGen.exe -d %dest%

:Exit
