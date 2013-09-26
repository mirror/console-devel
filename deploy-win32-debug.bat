set deploypath="C:\Program Files\Console"
set version=Win32\Debug

taskkill /IM Console.exe /F
del %deploypath%\Console.exe
copy /Y bin\%version%\Console.exe %deploypath%

taskkill /IM explorer.exe /F
del %deploypath%\ExplorerIntegration.dll
copy /Y bin\%version%\ExplorerIntegration.dll %deploypath%
