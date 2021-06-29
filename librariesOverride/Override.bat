@echo off

Set LibraryOverridePath=%~dp0

cd..
set Root=%CD%
Set LibraryPath=%Root%\libraries

cd  /d %LibraryOverridePath%
set ParentFolder=%CD%

echo ******************
echo   Override Start
echo ******************
:: Retrive all the files in Override folders
for /r . %%g in (*) do (call :ProcessFixup %%g)
goto :End

:ProcessFixup
  setlocal EnableDelayedExpansion

  :: Get file name and file path
  set FileName=%1
  set FilePath=%~d1%~p1
  call set FileName=%%FileName:!ParentFolder!\=%%
  call set FilePath=%%FilePath:!ParentFolder!\=%%

  :: Override file
  if "%FileName%" NEQ "Override.bat" (
    xcopy /S /Y /R %FileName% %LibraryPath%\%FilePath%
  )
  endlocal
  goto :EOF

:End
echo ******************
echo   Override End
echo ******************
pause
