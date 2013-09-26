set deploypath="C:\Program Files\Console"
set version=x64\Debug

taskkill /IM Console.exe /F
del %deploypath%\Console.exe
copy /Y bin\%version%\Console.exe %deploypath%

del %deploypath%\ConsoleHook.dll
copy /Y bin\%version%\ConsoleHook.dll %deploypath%

del %deploypath%\ConsoleWow.exe
copy /Y bin\%version%\ConsoleWow.exe %deploypath%

del %deploypath%\ConsoleHook32.dll
copy /Y bin\%version%\ConsoleHook32.dll %deploypath%

taskkill /IM explorer.exe /F
del %deploypath%\ExplorerIntegration.dll
copy /Y bin\%version%\ExplorerIntegration.dll %deploypath%
