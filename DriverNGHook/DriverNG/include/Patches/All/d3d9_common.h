#ifndef D3D9_COMMON
#define D3D9_COMMON

#include <Windows.h>

class WrappedD3DDevice9;

class RefCounter9
{
private:
    IUnknown* m_pReal;
    unsigned int m_iRefcount;
    bool m_SelfDeleting;

protected:
    void SetSelfDeleting(bool selfDelete) { m_SelfDeleting = selfDelete; }
    // used for derived classes that need to soft ref but are handling their
    // own self-deletion
    static void AddDeviceSoftref(WrappedD3DDevice9* device);
    static void ReleaseDeviceSoftref(WrappedD3DDevice9* device);

public:
    RefCounter9(IUnknown* real, bool selfDelete = true)
        : m_pReal(real), m_iRefcount(1), m_SelfDeleting(selfDelete)
    {
    }
    virtual ~RefCounter9() {}
    unsigned int GetRefCount() { return m_iRefcount; }
    //////////////////////////////
    // implement IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */
        __RPC__deref_out void** ppvObject)
    {
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        InterlockedIncrement(&m_iRefcount);
        return m_iRefcount;
    }
    ULONG STDMETHODCALLTYPE Release()
    {
        unsigned int ret = InterlockedDecrement(&m_iRefcount);
        if (ret == 0 && m_SelfDeleting)
            delete this;
        return ret;
    }

    unsigned int SoftRef(WrappedD3DDevice9* device);
    unsigned int SoftRelease(WrappedD3DDevice9* device);
};

#endif // D3D9_COMMON