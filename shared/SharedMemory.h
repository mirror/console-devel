#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

enum SyncObjectTypes
{
	syncObjNone		= 0,
	syncObjRequest	= 1,
	syncObjBoth		= 2
};

//////////////////////////////////////////////////////////////////////////////

template<typename T>
class SharedMemory
{
	public:

		SharedMemory();
		SharedMemory(const wstring& strName, DWORD dwSize, SyncObjectTypes syncObjects, bool bCreate = true);

		~SharedMemory();

		void Create(const wstring& strName, DWORD dwSize/* = 1*/, SyncObjectTypes syncObjects);
		void Open(const wstring& strName, SyncObjectTypes syncObjects/* = syncObjNone*/);

		inline void Lock();
		inline void Release();
		inline void SetReqEvent();
		inline void SetRespEvent();

		inline T* Get() const;
		inline HANDLE GetReqEvent() const;
		inline HANDLE GetRespEvent() const;

		inline T& operator[](size_t index) const;
		inline T* operator->() const;
		inline T& operator*() const;
		inline SharedMemory& operator=(const T& val);

	private:

		void CreateSyncObjects(SyncObjectTypes syncObjects, const wstring& strName);

	private:

		wstring				m_strName;
		DWORD				m_dwSize;

		shared_ptr<void>	m_hSharedMem;
		shared_ptr<T>		m_pSharedMem;

		shared_ptr<void>	m_hSharedMutex;
		shared_ptr<void>	m_hSharedReqEvent;
		shared_ptr<void>	m_hSharedRespEvent;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class SharedMemoryLock
{
	public:

		template <typename T> explicit SharedMemoryLock(SharedMemory<T>& sharedMem)
		: m_lock((sharedMem.Lock(), &sharedMem), boost::mem_fn(&SharedMemory<T>::Release))
		{
		}

	private:

		shared_ptr<void>	m_lock;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
SharedMemory<T>::SharedMemory()
: m_strName(L"")
, m_dwSize(0)
, m_hSharedMem()
, m_pSharedMem()
, m_hSharedMutex()
, m_hSharedReqEvent()
, m_hSharedRespEvent()
{
}


template<typename T>
SharedMemory<T>::SharedMemory(const wstring& strName, DWORD dwSize, SyncObjectTypes syncObjects, bool bCreate)
: m_strName(strName)
, m_dwSize(dwSize)
, m_hSharedMem()
, m_pSharedMem()
, m_hSharedMutex()
, m_hSharedReqEvent()
, m_hSharedRespEvent()
{
	if (bCreate)
	{
		Create(strName, dwSize, syncObjects);
	}
	else
	{
		Open(strName, syncObjects);
	}
}


template<typename T>
SharedMemory<T>::~SharedMemory()
{
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::Create(const wstring& strName, DWORD dwSize, SyncObjectTypes syncObjects)
{
	m_strName	= strName;
	m_dwSize	= dwSize;

	m_hSharedMem = shared_ptr<void>(::CreateFileMapping(
										INVALID_HANDLE_VALUE, 
										NULL, 
										PAGE_READWRITE, 
										0, 
										m_dwSize * sizeof(T), 
										m_strName.c_str()),
									::CloseHandle);

	// TODO: error handling
	//if (m_hSharedMem.get() == NULL) return false;

	m_pSharedMem = shared_ptr<T>(static_cast<T*>(::MapViewOfFile(
													m_hSharedMem.get(), 
													FILE_MAP_ALL_ACCESS, 
													0, 
													0, 
													0)),
												::UnmapViewOfFile);

	::ZeroMemory(m_pSharedMem.get(), m_dwSize * sizeof(T));

	if (syncObjects > syncObjNone) CreateSyncObjects(syncObjects, strName);

	//if (m_pSharedMem.get() == NULL) return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::Open(const wstring& strName, SyncObjectTypes syncObjects)
{
	m_strName	= strName;

	m_hSharedMem = shared_ptr<void>(::OpenFileMapping(
										FILE_MAP_ALL_ACCESS, 
										FALSE, 
										m_strName.c_str()),
									::CloseHandle);

	// TODO: error handling
	//if (m_hSharedMem.get() == NULL) return false;

	m_pSharedMem = shared_ptr<T>(static_cast<T*>(::MapViewOfFile(
													m_hSharedMem.get(), 
													FILE_MAP_ALL_ACCESS, 
													0, 
													0, 
													0)),
												::UnmapViewOfFile);

	if (syncObjects > syncObjNone) CreateSyncObjects(syncObjects, strName);

	//if (m_pSharedMem.get() == NULL) return false;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::Lock()
{
	if (m_hSharedMutex.get() == NULL) return;
	::WaitForSingleObject(m_hSharedMutex.get(), INFINITE);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::Release()
{
	if (m_hSharedMutex.get() == NULL) return;
	::ReleaseMutex(m_hSharedMutex.get());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::SetReqEvent()
{
	if (m_hSharedReqEvent.get() == NULL) return;
	::SetEvent(m_hSharedReqEvent.get());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::SetRespEvent()
{
	if (m_hSharedRespEvent.get() == NULL) return;
	::SetEvent(m_hSharedRespEvent.get());
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
T* SharedMemory<T>::Get() const
{
	return m_pSharedMem.get();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
HANDLE SharedMemory<T>::GetReqEvent() const
{
	return m_hSharedReqEvent.get();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
HANDLE SharedMemory<T>::GetRespEvent() const
{
	return m_hSharedRespEvent.get();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
T& SharedMemory<T>::operator[](size_t index) const
{
	return *(m_pSharedMem.get() + index);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
T* SharedMemory<T>::operator->() const
{
	return m_pSharedMem.get();
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
T& SharedMemory<T>::operator*() const
{
	return *m_pSharedMem;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
SharedMemory<T>& SharedMemory<T>::operator=(const T& val)
{
	*m_pSharedMem = val;
	return *this;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<typename T>
void SharedMemory<T>::CreateSyncObjects(SyncObjectTypes syncObjects, const wstring& strName)
{
	if (syncObjects >= syncObjRequest)
	{
		m_hSharedMutex = shared_ptr<void>(
							::CreateMutex(NULL, FALSE, (strName + wstring(L"_mutex")).c_str()),
							::CloseHandle);

		m_hSharedReqEvent = shared_ptr<void>(
							::CreateEvent(NULL, FALSE, FALSE, (strName + wstring(L"_req_event")).c_str()),
							::CloseHandle);
	}

	if (syncObjects >= syncObjBoth)
	{
		m_hSharedRespEvent = shared_ptr<void>(
							::CreateEvent(NULL, FALSE, FALSE, (strName + wstring(L"_resp_event")).c_str()),
							::CloseHandle);
	}
}

//////////////////////////////////////////////////////////////////////////////
