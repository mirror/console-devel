:: Set the environment for the compiler:
::call "%MSVC60Dir%\Bin\VcVars32.bat"
::call "%VS71COMNTOOLS%vsvars32.bat"
::call "%VS80COMNTOOLS%vsvars32.bat"
call "%VS90COMNTOOLS%vsvars32.bat"

:: Build the version with the current version number (use it when you are disconnected from the repository).
version.py %*
@if not defined inconsole @pause
