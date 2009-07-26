#pragma once

/// class to store tab settings loaded from console.xml
class CTabSettings
{
public:
	CTabSettings(void);
	virtual ~CTabSettings(void);

	wstring		sName;			///> tab name
	wstring		sIconFile;		///> icon file
	HBITMAP		hIconBmp;		///> icon converted to bitmap form with size 16x16
};
