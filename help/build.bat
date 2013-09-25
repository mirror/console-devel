pushd %~dp0
call hhc.bat console.hhp
copy console.chm ..\bin\Win32\Debug\Console.chm
copy console.chm ..\bin\Win32\Release\Console.chm
copy console.chm ..\bin\x64\Debug\Console.chm
copy console.chm ..\bin\x64\Release\Console.chm
popd
exit /B 0
