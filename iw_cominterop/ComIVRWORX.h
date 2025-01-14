// IComIVRWORX.h : Declaration of the CIComIVRWORX

#pragma once
#include "resource.h"       // main symbols

#include "iw_cominterop.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CIComIVRWORX

class ATL_NO_VTABLE CComIVRWORX :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComIVRWORX, &CLSID_ComIVRWORX>,
	public IDispatchImpl<IComIVRWORX, &IID_IComIVRWORX, &LIBID_iw_cominteropLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CComIVRWORX()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMIVRWORX)


BEGIN_COM_MAP(CComIVRWORX)
	COM_INTERFACE_ENTRY(IComIVRWORX)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	
	virtual HRESULT STDMETHODCALLTYPE Init( 
		/* [in] */ BSTR *conf_file);


};

OBJECT_ENTRY_AUTO(__uuidof(ComIVRWORX), CComIVRWORX)
