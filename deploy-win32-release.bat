set deploypath="C:\Program Files\Console"
set version=Win32\Release

taskkill /IM Console.exe /F
del %deploypath%\Console.exe
copy /Y bin\%version%\Console.exe %deploypath%

del %deploypath%\ConsoleHook.dll
copy /Y bin\%version%\ConsoleHook.dll %deploypath%

taskkill /IM explorer.exe /F
del %deploypath%\ExplorerIntegration.dll
copy /Y bin\%version%\ExplorerIntegration.dll %deploypath%
