set deploypath="C:\Program Files\Console"
set version=x64\Release

taskkill /IM Console.exe /F
del %deploypath%\Console.exe
copy /Y bin\%version%\Console.exe %deploypath%

taskkill /IM explorer.exe /F
del %deploypath%\ExplorerIntegration.dll
copy /Y bin\%version%\ExplorerIntegration.dll %deploypath%
 