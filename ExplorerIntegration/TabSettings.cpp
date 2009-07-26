#include "StdAfx.h"
#include "TabSettings.h"

CTabSettings::CTabSettings(void):hIconBmp(NULL)
{
}

CTabSettings::~CTabSettings(void)
{
	if(hIconBmp) DeleteObject(hIconBmp);
}
