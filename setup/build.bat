@pushd %~dp0

if exist "c:\Program Files\Inno Setup 5\ISCC.exe" "c:\Program Files\Inno Setup 5\ISCC.exe" /f console.iss /q
if exist "c:\Program Files (x86)\Inno Setup 5\ISCC.exe" "c:\Program Files (x86)\Inno Setup 5\ISCC.exe" /f console.iss /q

if exist "c:\Program Files\Inno Setup 5\ISCC.exe" "c:\Program Files\Inno Setup 5\ISCC.exe" /f console-x64.iss /q
if exist "c:\Program Files (x86)\Inno Setup 5\ISCC.exe" "c:\Program Files (x86)\Inno Setup 5\ISCC.exe" /f console-x64.iss /q

@popd
