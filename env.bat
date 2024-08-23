@echo off
set "vcfile1=G:\MicrosoftVisualStudio2019\1\VC\Auxiliary\Build\vcvars64.bat"
set "vcfile2=G:\MicrosoftVisualStudio2019\1\VC\Auxiliary\Build\vcvars64.bat"

if exist "%vcfile1%" (
    %comspec% /k "%vcfile1%"
) else (
    %comspec% /k "%vcfile2%"
)
