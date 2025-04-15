@echo off
cls


REM Manifest Touch
f:\tools\pack\pack64.exe /m %CD%\Package.appxmanifest /n CN=EDD191C1-439D-4D37-B608-9E521142451D

msbuild ExamAI.sln /clp:ErrorsOnly /p:Configuration="Release" /p:Platform=x64 /t:restore /p:RestorePackagesConfig=true
msbuild ExamAI.sln /clp:ErrorsOnly /p:Configuration="Release" /p:Platform=x64 
call clbcall
call sign x64\Release\ExamAI\ExamAI.exe

REM Portable
REM f:\tools\pack\pack64.exe /i %CD%\app.ico /c x64w,%CD%\x64\Release\ExamAI,ExamAI.exe /o %CD%\ExamAI.exe /u 00000000-0000-0000-0000-000000000000
REM call sign ExamAI.exe

REM MSIX
REM del ExamAI.msix
REM makeappx pack /d x64\Release\ExamAI /p %CD%\ExamAI.msix > nul


call clbcall
del "Generated Files"\* /s /q 
del packages\* /s /q
del x64\* /s /q
del ExamAI\x64\* /s /q
del .vs\ExamAI\CopilotIndices\* /s /q

REM f:\tools\tu8\uploader\uploader SmallTools
msbuild ExamAI.sln -target:Clean

