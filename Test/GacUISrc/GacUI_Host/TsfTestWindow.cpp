#define GAC_HEADER_USE_NAMESPACE
#include "..\..\..\Source\GacUI.h"
#include "..\..\..\Source\Compiler\InstanceQuery\GuiInstanceQuery.h"
#include "..\..\..\Source\Resources\GuiParserManager.h"
#include "..\..\..\Source\PlatformProviders\Windows\WinNativeWindow.h"
#include <Windows.h>
#include <msctf.h>

using namespace vl::collections;
using namespace vl::stream;
using namespace vl::presentation;

namespace test
{

/***********************************************************************
TsfTestWindow
***********************************************************************/

	class TsfTestWindow : public GuiWindow, private ITfThreadMgrEventSink, private ITfContextOwnerCompositionSink
	{
	private:
		ComPtr<ITfThreadMgr>				threadManager;
		ComPtr<ITfSource>					threadManagerSource;
		ComPtr<ITfDocumentMgr>				documentManager;
		ComPtr<ITfContext>					editContext;
		TfClientId							clientId;
		TfEditCookie						editCookie;

		IUnknown* GetUnknown()
		{
			return static_cast<IUnknown*>(static_cast<ITfThreadMgrEventSink*>(this));
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
		{
			if(!ppvObject)
			{
				return E_POINTER;
			}
			if(riid==IID_IUnknown)
			{
				*ppvObject=GetUnknown();
			}
			else if(riid==IID_ITfThreadMgrEventSink)
			{
				*ppvObject=static_cast<ITfThreadMgrEventSink*>(this);
			}
			else if(riid==IID_ITfContextOwnerCompositionSink)
			{
				*ppvObject=static_cast<ITfContextOwnerCompositionSink*>(this);
			}
			else
			{
				*ppvObject=NULL;
				return E_NOINTERFACE;
			}
			return S_OK;
		}

		ULONG STDMETHODCALLTYPE AddRef(void)
		{
			return 1;
		}

		ULONG STDMETHODCALLTYPE Release(void)
		{
			return 1;
		}

		HRESULT STDMETHODCALLTYPE OnInitDocumentMgr(ITfDocumentMgr *pdim)
		{
			return S_OK;
		}
        
		HRESULT STDMETHODCALLTYPE OnUninitDocumentMgr(ITfDocumentMgr *pdim)
		{
			return S_OK;
		}
        
		HRESULT STDMETHODCALLTYPE OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus)
		{
			return S_OK;
		}
        
		HRESULT STDMETHODCALLTYPE OnPushContext(ITfContext *pic)
		{
			return S_OK;
		}
        
		HRESULT STDMETHODCALLTYPE OnPopContext(ITfContext *pic)
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk)
		{
			return S_OK;
		}
        
		HRESULT STDMETHODCALLTYPE OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew)
		{
			return S_OK;
		}
        
		HRESULT STDMETHODCALLTYPE OnEndComposition(ITfCompositionView *pComposition)
		{
			return S_OK;
		}

	public:
		TsfTestWindow()
			:GuiWindow(theme::ThemeName::Window)
			,clientId(0)
		{
			SetText(GetApplication()->GetExecutableFolder());
			GetContainerComposition()->SetPreferredMinSize(Size(640, 480));
			ForceCalculateSizeImmediately();
			MoveToScreenCenter();

			Initialize();
		}

		~TsfTestWindow()
		{
		}

		void Initialize()
		{
			ITfThreadMgr*		pThreadMgr=0;
			ITfSource*			pSource=0;
			ITfDocumentMgr*		pDocumentMgr=0;
			ITfContext*			pContext=0;

			HRESULT hr=CoCreateInstance(
				CLSID_TF_ThreadMgr, 
                NULL, 
                CLSCTX_INPROC_SERVER, 
                IID_ITfThreadMgr, 
                (void**)&pThreadMgr
				);
			hr=pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource);
			DWORD cookie=0;
			hr=pSource->AdviseSink(IID_ITfThreadMgrEventSink, static_cast<ITfThreadMgrEventSink*>(this), &cookie);
			hr=pThreadMgr->Activate(&clientId);
			hr=pThreadMgr->CreateDocumentMgr(&pDocumentMgr);
			hr=pDocumentMgr->CreateContext(clientId, 0, GetUnknown(), &pContext, &editCookie);
			hr=pDocumentMgr->Push(pContext);
			{
				HWND hwnd=windows::GetWindowsForm(GetNativeWindow())->GetWindowHandle();
				ITfDocumentMgr* pPrevious=0;
				hr=pThreadMgr->AssociateFocus(hwnd, pDocumentMgr, &pPrevious);
				if(pPrevious)
				{
					pPrevious->Release();
				}
			}

			threadManager=pThreadMgr;
			threadManagerSource=pSource;
			documentManager=pDocumentMgr;
			editContext=pContext;
		}
	};
}

// also need ITextStoreACP to receive edited text
// CHECKED(pDocumentMgr->CreateContext(g_TfClientId, 0, NULL, &pContext, &pecTextStore));
// put it in NULL
// https://github.com/vczh-libraries/GacUI/issues/88
#ifdef COPY_FROM_YANGFF
#include <Windows.h>
#include <msctf.h>
#include <io.h>
#include <fcntl.h>

#include <cstdio>
#include <cassert>

#define RELEASE(x) if (x) { x->Release(); x = NULL; }
#define CHECKED(x) assert(SUCCEEDED(x))
// Thread manager IME
ITfThreadMgr2* g_pThreadMgr;
TfClientId g_TfClientId = TF_CLIENTID_NULL;
ITfInputProcessorProfiles* g_pInputProcessorProfiles;

// ITfUIElementSink 
class CUIElementSink : public ITfUIElementSink, public ITfTextEditSink, public ITfInputProcessorProfileActivationSink {
private:
	ULONG m_cRef = 1;
public:
	/* ITfUIElementSink */
	HRESULT STDMETHODCALLTYPE BeginUIElement(
		/* [in] */ DWORD dwUIElementId,
		/* [out][in] */ BOOL* pbShow) {

		*pbShow = FALSE;

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE UpdateUIElement(
		/* [in] */ DWORD dwUIElementId) {
		ITfUIElementMgr* pUIElementMgr;
		CHECKED(g_pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr));
		ITfUIElement* pUIElement;
		CHECKED(pUIElementMgr->GetUIElement(dwUIElementId, &pUIElement));
		// candidate list
		ITfCandidateListUIElement* pCandidateListUIElement;
		if (SUCCEEDED(pUIElement->QueryInterface(IID_ITfCandidateListUIElement, (void**)&pCandidateListUIElement))) {
			UINT uCount;
			CHECKED(pCandidateListUIElement->GetCount(&uCount));
			wprintf(L"Candidate list: %d\n", uCount);
			for (DWORD i = 0; i < uCount; i++) {
				BSTR bstr;
				CHECKED(pCandidateListUIElement->GetString(i, &bstr));
				wprintf(L"  %ls\t", bstr);
				SysFreeString(bstr);
			}
			wprintf(L"\n");
			RELEASE(pCandidateListUIElement);
		}
		// reading info
		ITfReadingInformationUIElement* pReadingInformationUIElement;
		if (SUCCEEDED(pUIElement->QueryInterface(IID_ITfReadingInformationUIElement, (void**)&pReadingInformationUIElement))) {
			BSTR bstr;
			CHECKED(pReadingInformationUIElement->GetString(&bstr));
			wprintf(L"Reading info: %ls\n", bstr);
			SysFreeString(bstr);
			RELEASE(pReadingInformationUIElement);
		}

		RELEASE(pUIElement);
		RELEASE(pUIElementMgr);

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE EndUIElement(
		/* [in] */ DWORD dwUIElementId) {

		return S_OK;
	}


	/* ITfTextEditSink */
	HRESULT STDMETHODCALLTYPE OnEndEdit(
		/* [in] */ __RPC__in_opt ITfContext* pic,
		/* [in] */ TfEditCookie ecReadOnly,
		/* [in] */ __RPC__in_opt ITfEditRecord* pEditRecord) {
		return S_OK;
	}

	/* ITfInputProcessorProfileActivationSink */
	HRESULT STDMETHODCALLTYPE OnActivated(
		/* [in] */ DWORD dwProfileType,
		/* [in] */ LANGID langid,
		/* [in] */ __RPC__in REFCLSID clsid,
		/* [in] */ __RPC__in REFGUID catid,
		/* [in] */ __RPC__in REFGUID guidProfile,
		/* [in] */ HKL hkl,
		/* [in] */ DWORD dwFlags) {
		wprintf(L"OnActivated\n");
		if (dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR) {
			wprintf(L"  Input processor\n");
			BSTR bstrName;
			g_pInputProcessorProfiles->GetLanguageProfileDescription(clsid, langid, guidProfile, &bstrName);
			wprintf(L"  %ls\n", bstrName);
			SysFreeString(bstrName);
		}
		else if (dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT) {
			wprintf(L"  Keyboard layout\n");
		}
		else {
			wprintf(L"  Unknown\n");
		}

		return S_OK;

	}

	/* IUnknown */
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) {
		if (ppvObj == NULL) {
			return E_INVALIDARG;
		}
		*ppvObj = NULL;
		if (IsEqualCATID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ITfUIElementSink)) {
			*ppvObj = (ITfUIElementSink*)(this);
		}
		else if (IsEqualGUID(riid, IID_ITfTextEditSink)) {
			*ppvObj = (ITfTextEditSink*)(this);
		}
		else if (IsEqualGUID(riid, IID_ITfInputProcessorProfileActivationSink)) {
			*ppvObj = (ITfInputProcessorProfileActivationSink*)(this);
		}
		else {
			return E_NOINTERFACE;
		}
		return S_OK;
	}
	ULONG STDMETHODCALLTYPE AddRef(void) {
		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}

	ULONG STDMETHODCALLTYPE Release(void) {
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}
};

int main() {
	_setmode(_fileno(stdout), _O_U16TEXT);

	CHECKED(CoInitialize(NULL));
	CHECKED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&g_pThreadMgr));
	g_pThreadMgr->ActivateEx(&g_TfClientId, TF_TMAE_UIELEMENTENABLEDONLY);

	CHECKED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, (void**)&g_pInputProcessorProfiles));

	ITfUIElementMgr* pUIElementMgr;
	CHECKED(g_pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr));
	// IID_ITfSource 

	ITfSource* pSource;
	CHECKED(g_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource));

	ITfUIElementSink* pUIElementSink = new CUIElementSink();

	DWORD m_dwUIElementSinkCookie, m_dwInputProcessorProfileActivationSinkCookie;
	pSource->AdviseSink(IID_ITfUIElementSink, pUIElementSink, &m_dwUIElementSinkCookie);
	pSource->AdviseSink(IID_ITfInputProcessorProfileActivationSink, pUIElementSink, &m_dwInputProcessorProfileActivationSinkCookie);

	// send keys to IME

	ITfDocumentMgr* pDocumentMgr;
	CHECKED(g_pThreadMgr->CreateDocumentMgr(&pDocumentMgr));

	ITfContext* pContext;
	// cookie
	TfEditCookie pecTextStore;
	CHECKED(pDocumentMgr->CreateContext(g_TfClientId, 0, NULL, &pContext, &pecTextStore));

	pDocumentMgr->Push(pContext);

	// ITfContextComposition
	ITfContextComposition* pContextComposition;
	CHECKED(pContext->QueryInterface(IID_ITfContextComposition, (void**)&pContextComposition));
	//RELEASE(pContext);
	//RELEASE(pDocumentMgr);

	// Create Window & Message Loop

	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = L"WSLIME";
	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindowEx(0, wcex.lpszClassName, L"WSLIME", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wcex.hInstance, NULL);
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	// End
	pSource->UnadviseSink(m_dwUIElementSinkCookie);
	RELEASE(pSource);
	RELEASE(pUIElementMgr);
	RELEASE(g_pThreadMgr);
	CoUninitialize();

	return 0;
}
#endif