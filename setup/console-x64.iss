[Setup]
OutputDir=setup
SourceDir=..\
OutputBaseFilename=Console 2.00b148d x64
VersionInfoVersion=2.00
MinVersion=0,5.0.2195sp3
AppName=Console
AppVersion=2.00b148d
AppVerName=Console
AppPublisher=Bozho & Kirill
AppPublisherURL=https://sourceforge.net/projects/console-devel/
DefaultDirName={pf}\Console
AllowNoIcons=true
ShowLanguageDialog=no
AppID={{99537A70-81BE-46EA-AA30-81C8F074AF45}
Compression=lzma
DefaultGroupName=Console
SetupIconFile=Console\res\Console.ico
UninstallDisplayIcon={app}\Console.exe
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
[Files]
; Binary files
Source: bin\x64\Release\Console.exe; DestDir: {app}; Components: main; Flags: 64bit
Source: help\Console.chm; DestDir: {app}; Components: main
Source: bin\x64\Release\ConsoleHook.dll; DestDir: {app}; Components: main; Flags: 64bit
Source: bin\x64\Release\ConsoleWow.exe; DestDir: {app}; Components: main;
Source: bin\x64\Release\ConsoleHook32.dll; DestDir: {app}; Components: main;
Source: bin\x64\Release\ExplorerIntegration.dll; DestDir: {app}; Components: main; Flags: 64bit regserver uninsrestartdelete restartreplace
Source: setup\dlls\x64\FreeImage.dll; DestDir: {app}; Components: main; Flags: 64bit
Source: setup\dlls\x64\FreeImagePlus.dll; DestDir: {app}; Components: main; Flags: 64bit
; Config files
Source: setup\config\console.xml; DestDir: {app}; Components: main
Source: setup\config\console.xml; DestDir: {userappdata}\Console; Components: main
; Fonts
Source: setup\fonts\FixedMedium5x7.fon; DestDir: {fonts}; FontInstall: FixedMedium5x7; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium5x8.fon; DestDir: {fonts}; FontInstall: FixedMedium5x8; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x9.fon; DestDir: {fonts}; FontInstall: FixedMedium6x9; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x10.fon; DestDir: {fonts}; FontInstall: FixedMedium6x10; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x12.fon; DestDir: {fonts}; FontInstall: FixedMedium6x12; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium6x13.fon; DestDir: {fonts}; FontInstall: FixedMedium6x13; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium7x13.fon; DestDir: {fonts}; FontInstall: FixedMedium7x13; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium7x14.fon; DestDir: {fonts}; FontInstall: FixedMedium7x14; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium8x13.fon; DestDir: {fonts}; FontInstall: FixedMedium8x13; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium9x15.fon; DestDir: {fonts}; FontInstall: FixedMedium9x15; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium9x18.fon; DestDir: {fonts}; FontInstall: FixedMedium9x18; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\FixedMedium10x20.fon; DestDir: {fonts}; FontInstall: FixedMedium10x20; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
Source: setup\fonts\TerminalMedium14.fon; DestDir: {fonts}; FontInstall: TerminalMedium14; Components: fonts; Flags: uninsrestartdelete fontisnttruetype
[Registry]
Root: HKCR; Subkey: *\ShellEx\ContextMenuHandlers\Console; ValueType: string; ValueData: "{{88076FF3-A8B5-4059-AB7D-9D7DEF3792FD}"; Tasks: shell_extention
Root: HKCR; Subkey: Directory\ShellEx\ContextMenuHandlers\Console; ValueType: string; ValueData: "{{88076FF3-A8B5-4059-AB7D-9D7DEF3792FD}"; Tasks: shell_extention
Root: HKCR; Subkey: Directory\Background\ShellEx\ContextMenuHandlers\Console; ValueType: string; ValueData: "{{88076FF3-A8B5-4059-AB7D-9D7DEF3792FD}"; Tasks: shell_extention
Root: HKCR; Subkey: Drive\ShellEx\ContextMenuHandlers\Console; ValueType: string; ValueData: "{{88076FF3-A8B5-4059-AB7D-9D7DEF3792FD}"; Tasks: shell_extention
[Icons]
Name: {group}\Console; Filename: {app}\Console.exe; WorkingDir: {app}; IconFilename: {app}\Console.exe; Comment: Console application; IconIndex: 0; Components: main
Name: {group}\Help; Filename: {app}\Console.chm; WorkingDir: {app}; 
Name: {userdesktop}\Console; Filename: {app}\Console.exe; WorkingDir: {app}; IconFilename: {app}\Console.exe; Comment: Console application; IconIndex: 0; Components: main; Tasks: desktop_icon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Console; Filename: {app}\Console.exe; WorkingDir: {app}; IconFilename: {app}\Console.exe; Comment: Console application; IconIndex: 0; Components: main; Tasks: quicklaunch_icon
[Components]
Name: main; Description: Program files; Types: custom compact full; Flags: fixed
Name: fonts; Description: Install additional console fonts; Types: custom full
[Tasks]
Name: shell_extention; Description: "Install Shell extension"
Name: desktop_icon; Description: Create desktop icon
Name: quicklaunch_icon; Description: Create Quick launch icon
