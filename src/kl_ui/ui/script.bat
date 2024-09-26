@echo off
for %%f in (*.ui) do (
    uic %%f -o ui_%%~nf.hpp
)
echo Conversion complete.