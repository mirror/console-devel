#pragma once

/// critical session wrapper
class syncCriticalSection
{
public:
	syncCriticalSection() 	{InitializeCriticalSection(&oSync);}
	virtual ~syncCriticalSection() {DeleteCriticalSection(&oSync);}

	void	lock() {EnterCriticalSection(&oSync);}
	void	unlock() {LeaveCriticalSection(&oSync);}
protected:
	CRITICAL_SECTION	oSync;		///> synchronization object
};

class syncAutoLock
{
public:
	syncAutoLock(syncCriticalSection& oCS):oSect(oCS) {oSect.lock();}
	virtual	~syncAutoLock() {oSect.unlock();}

protected:
	syncCriticalSection&	oSect;
};