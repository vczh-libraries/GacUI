#define GAC_HEADER_USE_NAMESPACE
#include "..\..\..\Source\GacUI.h"
#include "..\..\..\Source\NativeWindow\Windows\WinNativeWindow.h"
#include "..\..\..\Source\Compiler\InstanceQuery\GuiInstanceQuery.h"
#include "..\..\..\Source\Resources\GuiParserManager.h"
#include <Windows.h>
#include <msctf.h>

using namespace vl::collections;
using namespace vl::stream;
using namespace vl::presentation;

void UnitTestInGuiMain()
{
#define ASSERT(x) do{if(!(x))throw 0;}while(0)
	{
		GuiBoundsComposition* bounds = new GuiBoundsComposition;
		GuiControl* control = new GuiControl(new GuiControl::EmptyStyleController);
		bounds->AddChild(control->GetBoundsComposition());

		volatile vint* rc1 = ReferenceCounterOperator<GuiBoundsComposition>::CreateCounter(bounds);
		volatile vint* rc2 = ReferenceCounterOperator<GuiControl>::CreateCounter(control);

		ASSERT(*rc1 == 0);
		Ptr<GuiBoundsComposition> a1 = bounds;
		ASSERT(*rc1 == 1);
		Ptr<GuiBoundsComposition> a2 = bounds;
		ASSERT(*rc1 == 2);
		Ptr<DescriptableObject> a3 = a1.Cast<DescriptableObject>();
		ASSERT(*rc1 == 3);
		Ptr<DescriptableObject> a4 = a2;
		ASSERT(*rc1 == 4);

		ASSERT(*rc2 == 0);
		Ptr<GuiControl> b1 = control;
		ASSERT(*rc2 == 1);
		Ptr<GuiControl> b2 = control;
		ASSERT(*rc2 == 2);
		Ptr<DescriptableObject> b3 = b1.Cast<DescriptableObject>();
		ASSERT(*rc2 == 3);
		Ptr<DescriptableObject> b4 = b2;
		ASSERT(*rc2 == 4);

		b4 = b3 = b2 = b1 = 0; // not released yet
		ASSERT(*rc2 == 0);
		control->SetText(L"Text");

		a4 = a3 = a2 = a1 = 0; // all released
	}
	{
		auto parser = GetParserManager()->GetParser<GuiIqQuery>(L"INSTANCE-QUERY");
		ASSERT(parser);

		const wchar_t* queries[] = {
			L"/Label",
			L"/Label.labelMessage",
			L"/@:Label",
			L"/@:Label.labelMessage",
			L"/@Items:Label",
			L"/@Items:Label.labelMessage",
			L"/*",
			L"/*.labelMessage",
			L"/@:*",
			L"/@:*.labelMessage",
			L"/@Items:*",
			L"/@Items:*.labelMessage",
			L"//Label",
			L"//Label.labelMessage",
			L"//@:Label",
			L"//@:Label.labelMessage",
			L"//@Items:Label",
			L"//@Items:Label.labelMessage",
			L"//*",
			L"//*.labelMessage",
			L"//@:*",
			L"//@:*.labelMessage",
			L"//@Items:*",
			L"//@Items:*.labelMessage",

			L"//Cell/Label",
			L"//Cell/Label + //Cell/TextList",
			L"//Cell/Label - //Cell/TextList",
			L"//Cell/Label * //Cell/TextList",
			L"//Cell/Label ^ //Cell/TextList",
			L"//Cell(/Label + /TextList)"
			L"//Cell(/Label - /TextList)"
			L"//Cell(/Label * /TextList)"
			L"//Cell(/Label ^ /TextList)"
			L"//Cell(/Label + /TextList)//Items"
			L"//Cell(/Label - /TextList)//Items"
			L"//Cell(/Label * /TextList)//Items"
			L"//Cell(/Label ^ /TextList)//Items"
		};

		for (auto queryCode : queries)
		{
			List<Ptr<parsing::ParsingError>> errors;
			auto query = parser->ParseInternal(queryCode, errors);
			ASSERT(query);
			ASSERT(errors.Count() == 0);
			
			MemoryStream stream;
			WString queryText;
			{
				StreamWriter writer(stream);
				GuiIqPrint(query, writer);
			}
			stream.SeekFromBegin(0);
			{
				StreamReader reader(stream);
				queryText = reader.ReadToEnd();
			}
			ASSERT(queryText == queryCode || queryText == L"(" + WString(queryCode) + L")");
		}
	}
}

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
			:GuiWindow(GetCurrentTheme()->CreateWindowStyle())
			,clientId(0)
		{
			SetText(GetApplication()->GetExecutableFolder());
			GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
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