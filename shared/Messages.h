#pragma once

/// List of possible external commands:

enum eExternalCommands {
	eEC_Nothing = 0x1000,	///< dummy command, no params
	eEC_NewTab,				///< create new tab, use \ref ECNewTabParams
};

#if 0
#pragma pack(push)
#pragma pack(1)
#pragma warning(disable:4200)

//////////////////////////////////////////////////////////////////////////
// WM_COPYDATA message constants and structures
//////////////////////////////////////////////////////////////////////////

// Command is stored in COPYDATASTRUCT.dwData member.
// COPYDATASTRUCT.lpData points to structure with command params.

/// Parameters for \ref eEC_NewTab command
typedef struct _ECNewTabParams
{
	WORD	nTabNameSize;	///< tab name string size in wchar_ts (including trailing zero)
	WORD	nStartDirSize;	///< startup dir string size in wchar_ts (including trailing zero)
	WORD	nStartCmdSize;	///< startup command string size in wchar_ts (including trailing zero)
	wchar_t	szTabName[0];	///< tab name string (zero-ended)
//	wchar_t	szStartDir[0];	///< startup dir string (zero-ended)
//	wchar_t	szStartCmd[0];	///< startup command string (zero-ended)
} ECNewTabParams;

#pragma warning(default:4200)
#pragma pack(pop)

#endif