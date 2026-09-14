#include "Synthetic.h"
#include <UIAutomation.h>
#include <OleAcc.h>
#include <atomic>
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>
#include <cwctype>

#pragma comment(lib, "Oleacc.lib")

namespace
{
	class Provider;
	class TextRange;

	struct Context
	{
		std::atomic<ULONG> references = 1;
		HWND window = nullptr;
		HANDLE log = INVALID_HANDLE_VALUE;
		CRITICAL_SECTION lock;
		bool stress;
		std::vector<Provider*> nodes;
		std::wstring text = L"Alpha 日本語\r\n\r\nBeta 中文\r\n";
		std::atomic<int> invokes = 0, toggle = 0, view = 7, dock = DockPosition_None, visual = WindowVisualState_Normal;
		std::atomic<int> expanded = ExpandCollapseState_Collapsed, selected = 1, input = 0;
		std::atomic<double> value = 25, horizontal = 0, vertical = 0, zoom = 100, x = 10, y = 20, width = 100, height = 50, angle = 0;
		Context(bool large) : stress(large), nodes(large ? 11047 : 45, nullptr)
		{
			InitializeCriticalSection(&lock);
			wchar_t path[100]; swprintf_s(path, L"UiaFixture-%lu.%s.txt", GetCurrentProcessId(), large ? L"stress" : L"synthetic");
			log = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		}
		~Context() { CloseHandle(log); DeleteCriticalSection(&lock); }
		void AddRef() { ++references; }
		void Release() { if (!--references) delete this; }
		HRESULT Record(int key, const wchar_t* method, double a = 0, double b = 0, const wchar_t* textValue = L"")
		{
			EnterCriticalSection(&lock);
			wchar_t header[256]; swprintf_s(header, L"target=%d\t%s\ta=%.17g\tb=%.17g\tutf16=%zu\t", key, method, a, b, wcslen(textValue));
			std::wstring line = header;
			// Escape every UTF-16 unit so blank/final lines and Unicode have an exact oracle.
			for (auto p = textValue; *p; ++p) { wchar_t unit[8]; swprintf_s(unit, L"%04X ", static_cast<unsigned>(*p)); line += unit; }
			line += L"\r\n"; DWORD written;
			auto ok = WriteFile(log, line.data(), static_cast<DWORD>(line.size() * sizeof(wchar_t)), &written, nullptr);
			FlushFileBuffers(log); LeaveCriticalSection(&lock);
			return ok ? S_OK : HRESULT_FROM_WIN32(GetLastError());
		}
	};

	SAFEARRAY* Ints(std::initializer_list<LONG> values)
	{
		auto result = SafeArrayCreateVector(VT_I4, 3, static_cast<ULONG>(values.size()));
		LONG index = 3; for (auto value : values) { SafeArrayPutElement(result, &index, &value); ++index; } return result;
	}
	SAFEARRAY* Objects(IUnknown* value)
	{
		auto result = SafeArrayCreateVector(VT_UNKNOWN, 0, value ? 1 : 0);
		LONG index = 0; if (value) SafeArrayPutElement(result, &index, value); return result;
	}
	HRESULT String(const wchar_t* value, BSTR* output) { *output = SysAllocString(value); return *output ? S_OK : E_OUTOFMEMORY; }
	template<typename T> HRESULT Return(T* value, T** output) { *output = value; if (value) value->AddRef(); return S_OK; }

	// Deliberately artificial combinations exercise adapters; the other fixture window
	// supplies standards-conforming native controls. No inspector implementation is used.
	class Provider : public IRawElementProviderSimple, public IRawElementProviderFragment, public IRawElementProviderFragmentRoot,
		public IInvokeProvider, public ISelectionProvider2, public IValueProvider, public IRangeValueProvider,
		public IScrollProvider, public IExpandCollapseProvider, public IGridProvider, public IGridItemProvider,
		public IMultipleViewProvider, public IWindowProvider, public ISelectionItemProvider, public IDockProvider,
		public ITableProvider, public ITableItemProvider, public ITextProvider2, public IToggleProvider,
		public ITransformProvider2, public IScrollItemProvider, public ILegacyIAccessibleProvider,
		public IItemContainerProvider, public IVirtualizedItemProvider, public ISynchronizedInputProvider,
		public IObjectModelProvider, public IAnnotationProvider, public IStylesProvider, public ISpreadsheetProvider,
		public ISpreadsheetItemProvider, public ITextChildProvider, public IDragProvider, public IDropTargetProvider,
		public ITextEditProvider, public ICustomNavigationProvider
	{
	public:
		std::atomic<ULONG> references = 1;
		Context* context;
		int key;
		Provider(Context* owner, int id) : context(owner), key(id) { context->AddRef(); }
		~Provider() { context->nodes[key] = nullptr; context->Release(); }
		ULONG STDMETHODCALLTYPE AddRef() override { return ++references; }
		ULONG STDMETHODCALLTYPE Release() override { auto count = --references; if (!count) delete this; return count; }
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** output) override
		{
			*output = nullptr;
			if (iid == IID_IUnknown) *output = static_cast<IRawElementProviderSimple*>(this);
#define Q(T) else if (iid == __uuidof(T)) *output = static_cast<T*>(this)
			Q(IRawElementProviderSimple); Q(IRawElementProviderFragment); Q(IRawElementProviderFragmentRoot);
			Q(IInvokeProvider); Q(ISelectionProvider); Q(ISelectionProvider2); Q(IValueProvider); Q(IRangeValueProvider);
			Q(IScrollProvider); Q(IExpandCollapseProvider); Q(IGridProvider); Q(IGridItemProvider); Q(IMultipleViewProvider);
			Q(IWindowProvider); Q(ISelectionItemProvider); Q(IDockProvider); Q(ITableProvider); Q(ITableItemProvider);
			else if (iid == __uuidof(ITextProvider)) *output = static_cast<ITextProvider2*>(this);
			Q(ITextProvider2); Q(ITextEditProvider); Q(IToggleProvider); Q(ITransformProvider); Q(ITransformProvider2);
			Q(IScrollItemProvider); Q(ILegacyIAccessibleProvider); Q(IItemContainerProvider); Q(IVirtualizedItemProvider);
			Q(ISynchronizedInputProvider); Q(IObjectModelProvider); Q(IAnnotationProvider); Q(IStylesProvider);
			Q(ISpreadsheetProvider); Q(ISpreadsheetItemProvider); Q(ITextChildProvider); Q(IDragProvider); Q(IDropTargetProvider); Q(ICustomNavigationProvider);
#undef Q
			if (!*output) return E_NOINTERFACE; AddRef(); return S_OK;
		}
		HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions* output) override { *output = ProviderOptions_ServerSideProvider; return S_OK; }
		HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(IRawElementProviderSimple** output) override { *output = nullptr; return key ? S_OK : UiaHostProviderFromHwnd(context->window, output); }
		HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID id, IUnknown** output) override
		{
			*output = nullptr;
			if (key == 2)
			{
				if (id == UIA_TextPatternId || id == UIA_TextPattern2Id) *output = static_cast<ITextProvider2*>(this);
				else if (id == UIA_TextEditPatternId) *output = static_cast<ITextEditProvider*>(this);
			}
			else if (key == 1)
			{
				switch (id)
				{
#define P(ID, T) case UIA_##ID##PatternId: *output = static_cast<T*>(this); break
				P(Invoke, IInvokeProvider); P(Selection, ISelectionProvider2);
				case UIA_SelectionPattern2Id: *output = static_cast<ISelectionProvider2*>(this); break;
				P(Value, IValueProvider); P(RangeValue, IRangeValueProvider); P(Scroll, IScrollProvider); P(ExpandCollapse, IExpandCollapseProvider);
				P(Grid, IGridProvider); P(GridItem, IGridItemProvider); P(MultipleView, IMultipleViewProvider); P(Window, IWindowProvider);
				P(SelectionItem, ISelectionItemProvider); P(Dock, IDockProvider); P(Table, ITableProvider); P(TableItem, ITableItemProvider);
				P(Toggle, IToggleProvider); P(Transform, ITransformProvider2); P(ScrollItem, IScrollItemProvider);
				case UIA_TransformPattern2Id: *output = static_cast<ITransformProvider2*>(this); break;
				P(LegacyIAccessible, ILegacyIAccessibleProvider); P(ItemContainer, IItemContainerProvider); P(VirtualizedItem, IVirtualizedItemProvider);
				P(SynchronizedInput, ISynchronizedInputProvider); P(ObjectModel, IObjectModelProvider); P(Annotation, IAnnotationProvider);
				P(Styles, IStylesProvider); P(Spreadsheet, ISpreadsheetProvider); P(SpreadsheetItem, ISpreadsheetItemProvider);
				P(TextChild, ITextChildProvider); P(Drag, IDragProvider); P(DropTarget, IDropTargetProvider); P(CustomNavigation, ICustomNavigationProvider);
#undef P
				}
			}
			if (*output) AddRef(); return S_OK;
		}
		HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID id, VARIANT* output) override
		{
			VariantInit(output);
			switch (id)
			{
			case UIA_NamePropertyId:
				{
					wchar_t name[100]; swprintf_s(name, key >= 47 ? L"Stress node %d — 日本語 中文" : L"Role %d — 日本語 中文", key >= 47 ? key : key - 3);
					if (key == 45) wcscpy_s(name, L"10,000 siblings");
					if (key == 46) wcscpy_s(name, L"1,000 levels");
					output->vt = VT_BSTR; return String(key == 0 ? L"Synthetic adapter coverage" : key == 1 ? L"Synthetic capabilities" : key == 2 ? L"Synthetic document" : name, &output->bstrVal);
				}
			case UIA_AutomationIdPropertyId: output->vt = VT_BSTR; return String(L"deliberately-duplicated", &output->bstrVal);
			case UIA_ControlTypePropertyId: output->vt = VT_I4; output->lVal = key == 2 ? UIA_DocumentControlTypeId : key == 44 ? 59999 : key >= 3 && key <= 43 ? 50000 + key - 3 : UIA_CustomControlTypeId; break;
			case UIA_IsControlElementPropertyId: case UIA_IsContentElementPropertyId: output->vt = VT_BOOL; output->boolVal = key >= 3 ? VARIANT_FALSE : VARIANT_TRUE; break;
			case UIA_IsKeyboardFocusablePropertyId: case UIA_IsEnabledPropertyId: output->vt = VT_BOOL; output->boolVal = VARIANT_TRUE; break;
			case UIA_ValueValuePropertyId: output->vt = VT_BSTR; return get_Value(&output->bstrVal);
			case UIA_RangeValueValuePropertyId: output->vt = VT_R8; output->dblVal = context->value; break;
			case UIA_ToggleToggleStatePropertyId: output->vt = VT_I4; output->lVal = context->toggle; break;
			case UIA_HelpTextPropertyId: output->vt = VT_BSTR; return String(L"Synthetic combinations; see independent typed call log.", &output->bstrVal);
			}
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, IRawElementProviderFragment** output) override
		{
			int targetKey = -1, rootLast = context->stress ? 46 : 44;
			if (key == 0 && direction == NavigateDirection_FirstChild) targetKey = 1;
			if (key == 0 && direction == NavigateDirection_LastChild) targetKey = rootLast;
			if (key > 0 && direction == NavigateDirection_Parent) targetKey = key <= 46 ? 0 : key <= 10046 ? 45 : key == 10047 ? 46 : key - 1;
			if (key > 0 && key < rootLast && direction == NavigateDirection_NextSibling) targetKey = key + 1;
			if (key > 1 && key <= rootLast && direction == NavigateDirection_PreviousSibling) targetKey = key - 1;
			if (key == 45 && direction == NavigateDirection_FirstChild) targetKey = 47;
			if (key == 45 && direction == NavigateDirection_LastChild) targetKey = 10046;
			if (key >= 47 && key < 10046 && direction == NavigateDirection_NextSibling) targetKey = key + 1;
			if (key > 47 && key <= 10046 && direction == NavigateDirection_PreviousSibling) targetKey = key - 1;
			if ((direction == NavigateDirection_FirstChild || direction == NavigateDirection_LastChild) && (key == 46 || (key >= 10047 && key < 11046))) targetKey = key == 46 ? 10047 : key + 1;
			auto target = targetKey < 0 ? nullptr : context->nodes[targetKey];
			return Return<IRawElementProviderFragment>(target, output);
		}
		HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** output) override { *output = key ? Ints({ UiaAppendRuntimeId, key }) : nullptr; return S_OK; }
		HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* output) override
		{
			RECT bounds; GetWindowRect(context->window, &bounds);
			*output = { static_cast<double>(bounds.left + 15), static_cast<double>(bounds.top + 40 + key * 10), 400, 30 }; return S_OK;
		}
		HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(SAFEARRAY** output) override { *output = nullptr; return S_OK; }
		HRESULT STDMETHODCALLTYPE SetFocus() override { return context->Record(key, L"SetFocus"); }
		HRESULT STDMETHODCALLTYPE get_FragmentRoot(IRawElementProviderFragmentRoot** output) override { return Return<IRawElementProviderFragmentRoot>(context->nodes[0], output); }
		HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(double, double, IRawElementProviderFragment** output) override { return Return<IRawElementProviderFragment>(this, output); }
		HRESULT STDMETHODCALLTYPE GetFocus(IRawElementProviderFragment** output) override { return Return<IRawElementProviderFragment>(context->nodes[1], output); }
#define GET(T, M, V) HRESULT STDMETHODCALLTYPE get_##M(T* output) override { *output = (V); return S_OK; }
#define STR(M, V) HRESULT STDMETHODCALLTYPE get_##M(BSTR* output) override { return String(V, output); }
#define ELEMENT(M, K) HRESULT STDMETHODCALLTYPE get_##M(IRawElementProviderSimple** output) override { return Return<IRawElementProviderSimple>(context->nodes[K], output); }
#define ARRAY(M) HRESULT STDMETHODCALLTYPE M(SAFEARRAY** output) override { *output = Objects(static_cast<IRawElementProviderSimple*>(context->nodes[1])); return S_OK; }
		HRESULT STDMETHODCALLTYPE Invoke() override { ++context->invokes; return context->Record(key, L"Invoke", context->invokes); }
		HRESULT STDMETHODCALLTYPE SetValue(LPCWSTR value) override { EnterCriticalSection(&context->lock); context->text = value; LeaveCriticalSection(&context->lock); return context->Record(key, L"SetValue(BSTR)", 0, 0, value); }
		HRESULT STDMETHODCALLTYPE get_Value(BSTR* output) override { EnterCriticalSection(&context->lock); auto hr = String(context->text.c_str(), output); LeaveCriticalSection(&context->lock); return hr; }
		HRESULT STDMETHODCALLTYPE SetValue(double value) override { if (value < 0 || value > 100) return E_INVALIDARG; context->value = value; return context->Record(key, L"SetValue(R8)", value); }
		GET(double, Value, context->value); GET(BOOL, IsReadOnly, FALSE); GET(double, Minimum, 0); GET(double, Maximum, 100); GET(double, SmallChange, 1); GET(double, LargeChange, 10);
		GET(BOOL, CanSelectMultiple, TRUE); GET(BOOL, IsSelectionRequired, FALSE); GET(int, ItemCount, context->selected);
		ELEMENT(FirstSelectedItem, 1); ELEMENT(LastSelectedItem, 1); ELEMENT(CurrentSelectedItem, 1);
		HRESULT STDMETHODCALLTYPE GetSelection(SAFEARRAY** output) override;
		HRESULT STDMETHODCALLTYPE Scroll(ScrollAmount h, ScrollAmount v) override { context->horizontal = std::clamp(context->horizontal + (h == ScrollAmount_NoAmount ? 0 : h > 2 ? 10 : -10), 0.0, 100.0); context->vertical = std::clamp(context->vertical + (v == ScrollAmount_NoAmount ? 0 : v > 2 ? 10 : -10), 0.0, 100.0); return context->Record(key, L"Scroll", h, v); }
		HRESULT STDMETHODCALLTYPE SetScrollPercent(double h, double v) override { if (h != -1) context->horizontal = h; if (v != -1) context->vertical = v; return context->Record(key, L"SetScrollPercent", h, v); }
		GET(double, HorizontalScrollPercent, context->horizontal); GET(double, VerticalScrollPercent, context->vertical); GET(double, HorizontalViewSize, 25); GET(double, VerticalViewSize, 25); GET(BOOL, HorizontallyScrollable, TRUE); GET(BOOL, VerticallyScrollable, TRUE);
		HRESULT STDMETHODCALLTYPE Expand() override { context->expanded = ExpandCollapseState_Expanded; return context->Record(key, L"Expand"); }
		HRESULT STDMETHODCALLTYPE Collapse() override { context->expanded = ExpandCollapseState_Collapsed; return context->Record(key, L"Collapse"); }
		GET(ExpandCollapseState, ExpandCollapseState, static_cast<::ExpandCollapseState>(context->expanded.load()));
		GET(int, RowCount, 1); GET(int, ColumnCount, 1); GET(int, Row, 0); GET(int, Column, 0); GET(int, RowSpan, 1); GET(int, ColumnSpan, 1); ELEMENT(ContainingGrid, 1);
		HRESULT STDMETHODCALLTYPE GetItem(int row, int column, IRawElementProviderSimple** output) override { context->Record(key, L"GetItem", row, column); if (row || column) return E_INVALIDARG; return Return<IRawElementProviderSimple>(this, output); }
		HRESULT STDMETHODCALLTYPE GetViewName(int id, BSTR* output) override { return String(id == 7 ? L"View seven" : L"View forty-two", output); }
		HRESULT STDMETHODCALLTYPE SetCurrentView(int id) override { if (id != 7 && id != 42) return E_INVALIDARG; context->view = id; return context->Record(key, L"SetCurrentView", id); }
		GET(int, CurrentView, context->view); HRESULT STDMETHODCALLTYPE GetSupportedViews(SAFEARRAY** output) override { *output = Ints({ 7, 42 }); return S_OK; }
		HRESULT STDMETHODCALLTYPE SetVisualState(WindowVisualState value) override { context->visual = value; return context->Record(key, L"SetVisualState", value); }
		HRESULT STDMETHODCALLTYPE Close() override { context->Record(key, L"Close"); PostMessageW(context->window, WM_CLOSE, 0, 0); return S_OK; }
		HRESULT STDMETHODCALLTYPE WaitForInputIdle(int milliseconds, BOOL* output) override { *output = TRUE; return context->Record(key, L"WaitForInputIdle", milliseconds); }
		GET(BOOL, CanMaximize, TRUE); GET(BOOL, CanMinimize, TRUE); GET(BOOL, IsModal, FALSE); GET(BOOL, IsTopmost, FALSE);
		GET(WindowVisualState, WindowVisualState, static_cast<::WindowVisualState>(context->visual.load())); GET(WindowInteractionState, WindowInteractionState, WindowInteractionState_Running);
		HRESULT STDMETHODCALLTYPE Select() override { context->selected = 1; return context->Record(key, L"Select"); }
		HRESULT STDMETHODCALLTYPE AddToSelection() override { context->selected = 1; return context->Record(key, L"AddToSelection"); }
		HRESULT STDMETHODCALLTYPE RemoveFromSelection() override { context->selected = 0; return context->Record(key, L"RemoveFromSelection"); }
		GET(BOOL, IsSelected, context->selected != 0); ELEMENT(SelectionContainer, 1);
		HRESULT STDMETHODCALLTYPE SetDockPosition(DockPosition value) override { context->dock = value; return context->Record(key, L"SetDockPosition", value); }
		GET(DockPosition, DockPosition, static_cast<::DockPosition>(context->dock.load())); GET(RowOrColumnMajor, RowOrColumnMajor, RowOrColumnMajor_RowMajor);
		ARRAY(GetRowHeaders); ARRAY(GetColumnHeaders); ARRAY(GetRowHeaderItems); ARRAY(GetColumnHeaderItems);
		HRESULT STDMETHODCALLTYPE Toggle() override { context->toggle = (context->toggle + 1) % 3; return context->Record(key, L"Toggle", context->toggle); }
		GET(ToggleState, ToggleState, static_cast<::ToggleState>(context->toggle.load()));
		HRESULT STDMETHODCALLTYPE Move(double x, double y) override { context->x = x; context->y = y; return context->Record(key, L"Move", x, y); }
		HRESULT STDMETHODCALLTYPE Resize(double width, double height) override { context->width = width; context->height = height; return context->Record(key, L"Resize", width, height); }
		HRESULT STDMETHODCALLTYPE Rotate(double degrees) override { context->angle = degrees; return context->Record(key, L"Rotate", degrees); }
		GET(BOOL, CanMove, TRUE); GET(BOOL, CanResize, TRUE); GET(BOOL, CanRotate, TRUE); GET(BOOL, CanZoom, TRUE); GET(double, ZoomLevel, context->zoom); GET(double, ZoomMinimum, 25); GET(double, ZoomMaximum, 400);
		HRESULT STDMETHODCALLTYPE Zoom(double value) override { context->zoom = value; return context->Record(key, L"Zoom", value); }
		HRESULT STDMETHODCALLTYPE ZoomByUnit(ZoomUnit value) override { context->zoom = std::clamp(context->zoom + (value == ZoomUnit_NoAmount ? 0 : value > 2 ? 25 : -25), 25.0, 400.0); return context->Record(key, L"ZoomByUnit", value); }
		HRESULT STDMETHODCALLTYPE ScrollIntoView() override { return context->Record(key, L"ScrollIntoView"); }
		HRESULT STDMETHODCALLTYPE Select(long flags) override { return context->Record(key, L"Legacy.Select", flags); }
		HRESULT STDMETHODCALLTYPE DoDefaultAction() override { return context->Record(key, L"DoDefaultAction"); }
		HRESULT STDMETHODCALLTYPE GetIAccessible(IAccessible** output) override { return AccessibleObjectFromWindow(context->window, OBJID_CLIENT, IID_IAccessible, reinterpret_cast<void**>(output)); }
		GET(int, ChildId, 0); GET(DWORD, Role, ROLE_SYSTEM_CLIENT); GET(DWORD, State, 0); STR(Name, L"Legacy fixture"); STR(Description, L"Synthetic"); STR(Help, L"Help"); STR(KeyboardShortcut, L"Alt+F"); STR(DefaultAction, L"Invoke");
		HRESULT STDMETHODCALLTYPE FindItemByProperty(IRawElementProviderSimple* after, PROPERTYID id, VARIANT value, IRawElementProviderSimple** output) override { context->Record(key, L"FindItemByProperty", id, value.vt); return Return<IRawElementProviderSimple>(after ? nullptr : this, output); }
		HRESULT STDMETHODCALLTYPE Realize() override { return context->Record(key, L"Realize"); }
		HRESULT STDMETHODCALLTYPE StartListening(SynchronizedInputType value) override { context->input = value; return context->Record(key, L"StartListening", value); }
		HRESULT STDMETHODCALLTYPE Cancel() override { context->input = 0; return context->Record(key, L"Cancel"); }
		HRESULT STDMETHODCALLTYPE GetUnderlyingObjectModel(IUnknown** output) override { context->Record(key, L"GetUnderlyingObjectModel"); return Return<IUnknown>(static_cast<IRawElementProviderSimple*>(this), output); }
		GET(int, AnnotationTypeId, AnnotationType_Comment); STR(AnnotationTypeName, L"Comment"); STR(Author, L"Fixture 日本語"); STR(DateTime, L"2026-09-13"); ELEMENT(Target, 2);
		GET(int, StyleId, StyleId_Normal); STR(StyleName, L"Normal"); GET(int, FillColor, 0x112233); STR(FillPatternStyle, L"Solid"); STR(Shape, L"Rectangle"); GET(int, FillPatternColor, 0x445566); STR(ExtendedProperties, L"fixture=independent;");
		HRESULT STDMETHODCALLTYPE GetItemByName(LPCWSTR name, IRawElementProviderSimple** output) override { context->Record(key, L"GetItemByName", 0, 0, name); return Return<IRawElementProviderSimple>(wcscmp(name, L"A1") == 0 ? this : nullptr, output); }
		STR(Formula, L"=1+2"); ARRAY(GetAnnotationObjects); HRESULT STDMETHODCALLTYPE GetAnnotationTypes(SAFEARRAY** output) override { *output = Ints({ AnnotationType_Comment }); return S_OK; }
		ELEMENT(TextContainer, 2); HRESULT STDMETHODCALLTYPE get_TextRange(ITextRangeProvider** output) override;
		GET(BOOL, IsGrabbed, FALSE); STR(DropEffect, L"copy"); STR(DropTargetEffect, L"copy"); ARRAY(GetGrabbedItems);
		HRESULT STDMETHODCALLTYPE get_DropEffects(SAFEARRAY** output) override { *output = SafeArrayCreateVector(VT_BSTR, 0, 2); LONG index = 0; auto a = SysAllocString(L"copy"); SafeArrayPutElement(*output, &index, a); SysFreeString(a); index++; a = SysAllocString(L"move"); SafeArrayPutElement(*output, &index, a); SysFreeString(a); return S_OK; }
		HRESULT STDMETHODCALLTYPE get_DropTargetEffects(SAFEARRAY** output) override { return get_DropEffects(output); }
		HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, IRawElementProviderSimple** output) override { context->Record(key, L"Custom.Navigate", direction); return Return<IRawElementProviderSimple>(direction == NavigateDirection_Parent ? context->nodes[0] : context->nodes[2], output); }
		GET(SupportedTextSelection, SupportedTextSelection, SupportedTextSelection_Multiple);
		HRESULT STDMETHODCALLTYPE GetVisibleRanges(SAFEARRAY** output) override;
		HRESULT STDMETHODCALLTYPE RangeFromChild(IRawElementProviderSimple*, ITextRangeProvider** output) override;
		HRESULT STDMETHODCALLTYPE RangeFromPoint(UiaPoint, ITextRangeProvider** output) override;
		HRESULT STDMETHODCALLTYPE get_DocumentRange(ITextRangeProvider** output) override;
		HRESULT STDMETHODCALLTYPE RangeFromAnnotation(IRawElementProviderSimple*, ITextRangeProvider** output) override;
		HRESULT STDMETHODCALLTYPE GetCaretRange(BOOL* active, ITextRangeProvider** output) override;
		HRESULT STDMETHODCALLTYPE GetActiveComposition(ITextRangeProvider** output) override;
		HRESULT STDMETHODCALLTYPE GetConversionTarget(ITextRangeProvider** output) override;
#undef GET
#undef STR
#undef ELEMENT
#undef ARRAY
	};

	class TextRange : public ITextRangeProvider2
	{
	public:
		std::atomic<ULONG> references = 1;
		Provider* document;
		std::wstring text;
		int start, end;
		TextRange(Provider* owner, int first = 0, int last = -1) : document(owner), start(first)
		{
			document->AddRef(); EnterCriticalSection(&document->context->lock);
			text = document->context->text; LeaveCriticalSection(&document->context->lock);
			end = last < 0 ? static_cast<int>(text.size()) : last;
		}
		~TextRange() { document->Release(); }
		ULONG STDMETHODCALLTYPE AddRef() override { return ++references; }
		ULONG STDMETHODCALLTYPE Release() override { auto count = --references; if (!count) delete this; return count; }
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** output) override
		{
			*output = nullptr;
			if (iid != IID_IUnknown && iid != __uuidof(ITextRangeProvider) && iid != __uuidof(ITextRangeProvider2)) return E_NOINTERFACE;
			*output = static_cast<ITextRangeProvider2*>(this); AddRef(); return S_OK;
		}
		HRESULT Record(const wchar_t* method, double a = 0, double b = 0) { return document->context->Record(2, method, a, b); }
		TextRange* Other(ITextRangeProvider* value) { auto range = dynamic_cast<TextRange*>(value); return range && range->document == document ? range : nullptr; }
		int Endpoint(TextPatternRangeEndpoint endpoint) { return endpoint == TextPatternRangeEndpoint_Start ? start : end; }
		void SetEndpoint(TextPatternRangeEndpoint endpoint, int value)
		{
			value = std::clamp(value, 0, static_cast<int>(text.size()));
			if (endpoint == TextPatternRangeEndpoint_Start) { start = value; if (end < start) end = start; }
			else { end = value; if (start > end) start = end; }
		}
		std::vector<int> Units(TextUnit unit)
		{
			std::vector<int> boundaries = { 0 };
			for (int i = 1; i < static_cast<int>(text.size()); i++)
			{
				if (unit == TextUnit_Character || (unit == TextUnit_Word && iswspace(text[i - 1]) && !iswspace(text[i])) || ((unit == TextUnit_Line || unit == TextUnit_Paragraph) && text[i - 1] == L'\n')) boundaries.push_back(i);
			}
			boundaries.push_back(static_cast<int>(text.size())); return boundaries;
		}
		HRESULT STDMETHODCALLTYPE Clone(ITextRangeProvider** output) override { auto range = new TextRange(document, start, end); range->text = text; *output = range; return Record(L"Range.Clone", start, end); }
		HRESULT STDMETHODCALLTYPE Compare(ITextRangeProvider* other, BOOL* output) override { auto range = Other(other); if (!range) return E_INVALIDARG; *output = start == range->start && end == range->end; return Record(L"Range.Compare", *output); }
		HRESULT STDMETHODCALLTYPE CompareEndpoints(TextPatternRangeEndpoint endpoint, ITextRangeProvider* other, TextPatternRangeEndpoint target, int* output) override { auto range = Other(other); if (!range) return E_INVALIDARG; *output = Endpoint(endpoint) - range->Endpoint(target); return Record(L"Range.CompareEndpoints", endpoint, target); }
		HRESULT STDMETHODCALLTYPE ExpandToEnclosingUnit(TextUnit unit) override
		{
			auto boundaries = Units(unit); auto it = std::upper_bound(boundaries.begin(), boundaries.end(), start);
			if (it == boundaries.end()) --it;
			end = *it; start = it == boundaries.begin() ? 0 : *(it - 1); return Record(L"Range.ExpandToEnclosingUnit", unit);
		}
		HRESULT STDMETHODCALLTYPE FindAttribute(TEXTATTRIBUTEID id, VARIANT value, BOOL backward, ITextRangeProvider** output) override
		{
			*output = nullptr; VARIANT actual; VariantInit(&actual); GetAttributeValue(id, &actual);
			if (actual.vt == value.vt && VarCmp(&actual, &value, LOCALE_INVARIANT, 0) == VARCMP_EQ) Clone(output);
			VariantClear(&actual); return Record(L"Range.FindAttribute", id, value.vt + backward * 65536);
		}
		HRESULT STDMETHODCALLTYPE FindText(BSTR value, BOOL backward, BOOL ignoreCase, ITextRangeProvider** output) override
		{
			*output = nullptr; auto haystack = text.substr(start, end - start); std::wstring needle(value, SysStringLen(value));
			if (ignoreCase) { for (auto& c : haystack) c = static_cast<wchar_t>(towupper(c)); for (auto& c : needle) c = static_cast<wchar_t>(towupper(c)); }
			auto index = backward ? haystack.rfind(needle) : haystack.find(needle);
			if (index != std::wstring::npos) *output = new TextRange(document, start + static_cast<int>(index), start + static_cast<int>(index + needle.size()));
			return document->context->Record(2, L"Range.FindText", backward, ignoreCase, value);
		}
		HRESULT STDMETHODCALLTYPE GetAttributeValue(TEXTATTRIBUTEID id, VARIANT* output) override
		{
			VariantInit(output); Record(L"Range.GetAttributeValue", id);
			switch (id)
			{
			case UIA_FontNameAttributeId: case UIA_StyleNameAttributeId: case UIA_LineSpacingAttributeId:
				output->vt = VT_BSTR; return String(id == UIA_FontNameAttributeId ? L"Segoe UI" : id == UIA_StyleNameAttributeId ? L"Normal" : L"1.0", &output->bstrVal);
			case UIA_FontSizeAttributeId: case UIA_IndentationFirstLineAttributeId: case UIA_IndentationLeadingAttributeId: case UIA_IndentationTrailingAttributeId: case UIA_MarginBottomAttributeId: case UIA_MarginLeadingAttributeId: case UIA_MarginTopAttributeId: case UIA_MarginTrailingAttributeId: case UIA_AfterParagraphSpacingAttributeId: case UIA_BeforeParagraphSpacingAttributeId:
				output->vt = VT_R8; output->dblVal = id == UIA_FontSizeAttributeId ? 14.5 : 0; break;
			case UIA_IsHiddenAttributeId: case UIA_IsItalicAttributeId: case UIA_IsReadOnlyAttributeId: case UIA_IsSubscriptAttributeId: case UIA_IsSuperscriptAttributeId: case UIA_IsActiveAttributeId:
				output->vt = VT_BOOL; output->boolVal = id == UIA_IsActiveAttributeId ? VARIANT_TRUE : VARIANT_FALSE; break;
			case UIA_TabsAttributeId:
				output->vt = VT_ARRAY | VT_R8; output->parray = SafeArrayCreateVector(VT_R8, 2, 2);
				{ LONG index = 2; double value = 10; SafeArrayPutElement(output->parray, &index, &value); index++; value = 20; SafeArrayPutElement(output->parray, &index, &value); } break;
			case UIA_AnnotationTypesAttributeId: output->vt = VT_ARRAY | VT_I4; output->parray = Ints({ AnnotationType_Comment }); break;
			case UIA_AnnotationObjectsAttributeId: output->vt = VT_ARRAY | VT_UNKNOWN; output->parray = Objects(static_cast<IRawElementProviderSimple*>(document->context->nodes[1])); break;
			case UIA_LinkAttributeId: output->vt = VT_UNKNOWN; return UiaGetReservedNotSupportedValue(&output->punkVal);
			case UIA_FontWeightAttributeId:
				if (start == 0 && end > 5) { output->vt = VT_UNKNOWN; return UiaGetReservedMixedAttributeValue(&output->punkVal); }
				output->vt = VT_I4; output->lVal = 400; break;
			default:
				if (id >= 40000 && id <= 40043) { output->vt = VT_I4; output->lVal = id == UIA_CultureAttributeId ? 1033 : 0; }
				else { output->vt = VT_UNKNOWN; return UiaGetReservedNotSupportedValue(&output->punkVal); }
			}
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE GetBoundingRectangles(SAFEARRAY** output) override
		{
			UiaRect bounds; document->get_BoundingRectangle(&bounds);
			*output = SafeArrayCreateVector(VT_R8, 0, start == end ? 0 : 4);
			double values[] = { bounds.left, bounds.top, static_cast<double>((end - start) * 8), 20 };
			if (start != end) for (LONG i = 0; i < 4; i++) SafeArrayPutElement(*output, &i, values + i);
			return Record(L"Range.GetBoundingRectangles", start, end);
		}
		HRESULT STDMETHODCALLTYPE GetEnclosingElement(IRawElementProviderSimple** output) override { return Return<IRawElementProviderSimple>(document, output); }
		HRESULT STDMETHODCALLTYPE GetText(int maximum, BSTR* output) override
		{
			if (maximum < -1) return E_INVALIDARG;
			auto count = maximum == -1 ? end - start : (std::min)(end - start, maximum);
			*output = SysAllocStringLen(text.data() + start, count); return Record(L"Range.GetText", maximum, count);
		}
		HRESULT STDMETHODCALLTYPE Move(TextUnit unit, int count, int* output) override
		{
			auto boundaries = Units(unit); auto current = std::upper_bound(boundaries.begin(), boundaries.end(), start) - boundaries.begin() - 1;
			current = (std::min)(current, static_cast<ptrdiff_t>(boundaries.size()) - 2);
			auto next = std::clamp<ptrdiff_t>(current + static_cast<long long>(count), 0, boundaries.size() - 2);
			*output = static_cast<int>(next - current); if (count) { start = boundaries[next]; end = boundaries[next + 1]; }
			return Record(L"Range.Move", unit, count);
		}
		HRESULT STDMETHODCALLTYPE MoveEndpointByUnit(TextPatternRangeEndpoint endpoint, TextUnit unit, int count, int* output) override
		{
			auto boundaries = Units(unit); auto current = std::lower_bound(boundaries.begin(), boundaries.end(), Endpoint(endpoint)) - boundaries.begin();
			auto next = std::clamp<ptrdiff_t>(current + static_cast<long long>(count), 0, boundaries.size() - 1);
			*output = static_cast<int>(next - current); if (count) SetEndpoint(endpoint, boundaries[next]);
			return Record(L"Range.MoveEndpointByUnit", endpoint * 10 + unit, count);
		}
		HRESULT STDMETHODCALLTYPE MoveEndpointByRange(TextPatternRangeEndpoint endpoint, ITextRangeProvider* other, TextPatternRangeEndpoint target) override { auto range = Other(other); if (!range) return E_INVALIDARG; SetEndpoint(endpoint, range->Endpoint(target)); return Record(L"Range.MoveEndpointByRange", endpoint, target); }
		HRESULT STDMETHODCALLTYPE Select() override { return Record(L"Range.Select", start, end); }
		HRESULT STDMETHODCALLTYPE AddToSelection() override { return Record(L"Range.AddToSelection", start, end); }
		HRESULT STDMETHODCALLTYPE RemoveFromSelection() override { return Record(L"Range.RemoveFromSelection", start, end); }
		HRESULT STDMETHODCALLTYPE ScrollIntoView(BOOL align) override { return Record(L"Range.ScrollIntoView", align); }
		HRESULT STDMETHODCALLTYPE GetChildren(SAFEARRAY** output) override { *output = Objects(static_cast<IRawElementProviderSimple*>(document->context->nodes[1])); return S_OK; }
		HRESULT STDMETHODCALLTYPE ShowContextMenu() override { return Record(L"Range.ShowContextMenu", start, end); }
	};

	HRESULT Provider::GetSelection(SAFEARRAY** output)
	{
		if (key != 2) { *output = Objects(context->selected ? static_cast<IRawElementProviderSimple*>(this) : nullptr); return S_OK; }
		auto range = new TextRange(this, 0, 5); *output = Objects(range); range->Release(); return context->Record(key, L"Text.GetSelection");
	}
	HRESULT Provider::GetVisibleRanges(SAFEARRAY** output) { auto range = new TextRange(this); *output = Objects(range); range->Release(); return context->Record(key, L"Text.GetVisibleRanges"); }
	HRESULT Provider::get_DocumentRange(ITextRangeProvider** output) { *output = new TextRange(this); return context->Record(key, L"Text.DocumentRange"); }
	HRESULT Provider::get_TextRange(ITextRangeProvider** output) { *output = new TextRange(context->nodes[2], 0, 5); return context->Record(key, L"TextChild.TextRange"); }
	HRESULT Provider::RangeFromChild(IRawElementProviderSimple*, ITextRangeProvider** output) { *output = new TextRange(this, 0, 5); return context->Record(key, L"Text.RangeFromChild"); }
	HRESULT Provider::RangeFromAnnotation(IRawElementProviderSimple*, ITextRangeProvider** output) { *output = new TextRange(this, 6, 9); return context->Record(key, L"Text.RangeFromAnnotation"); }
	HRESULT Provider::RangeFromPoint(UiaPoint point, ITextRangeProvider** output) { *output = new TextRange(this, 0, 0); return context->Record(key, L"Text.RangeFromPoint", point.x, point.y); }
	HRESULT Provider::GetCaretRange(BOOL* active, ITextRangeProvider** output) { *active = TRUE; *output = new TextRange(this, 5, 5); return context->Record(key, L"Text.GetCaretRange"); }
	HRESULT Provider::GetActiveComposition(ITextRangeProvider** output) { *output = new TextRange(this, 6, 9); return context->Record(key, L"Text.GetActiveComposition"); }
	HRESULT Provider::GetConversionTarget(ITextRangeProvider** output) { *output = new TextRange(this, 6, 9); return context->Record(key, L"Text.GetConversionTarget"); }

	LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto context = reinterpret_cast<Context*>(GetWindowLongPtrW(window, GWLP_USERDATA));
		if (message == WM_NCCREATE) { context = static_cast<Context*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams); context->window = window; SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(context)); }
		if (!context) return DefWindowProcW(window, message, wParam, lParam);
		if (message == WM_GETOBJECT && static_cast<LONG>(lParam) == UiaRootObjectId) return UiaReturnRawElementProvider(window, wParam, lParam, context->nodes[0]);
		if (message == WM_PAINT)
		{
			PAINTSTRUCT paint; auto dc = BeginPaint(window, &paint);
			const wchar_t* lines[] = { L"Synthetic UIA adapter coverage", L"One capability node, one document, 41 raw-only roles.", L"Combinations are deliberately artificial.", L"Exact calls are recorded in UiaFixture-PID.synthetic.txt." };
			for (int i = 0; i < 4; i++) TextOutW(dc, 20, 30 + i * 30, lines[i], static_cast<int>(wcslen(lines[i])));
			EndPaint(window, &paint); return 0;
		}
		if (message == WM_DESTROY)
		{
			SetWindowLongPtrW(window, GWLP_USERDATA, 0);
			for (auto provider : context->nodes) if (provider) UiaDisconnectProvider(provider);
			for (auto provider : context->nodes) if (provider) provider->Release();
			context->Release(); return 0;
		}
		return DefWindowProcW(window, message, wParam, lParam);
	}
}

HWND CreateSyntheticFixture(HINSTANCE instance, bool stress)
{
	WNDCLASSW type = {}; type.hInstance = instance; type.lpszClassName = L"UiaSyntheticFixture"; type.lpfnWndProc = WindowProc; type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); type.hCursor = LoadCursor(nullptr, IDC_ARROW);
	if (!RegisterClassW(&type) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return nullptr;
	auto context = new Context(stress);
	for (int i = 0; i < static_cast<int>(context->nodes.size()); i++) context->nodes[i] = new Provider(context, i);
	auto window = CreateWindowExW(0, type.lpszClassName, stress ? L"UiaList verification — 10,000 siblings and 1,000 levels" : L"UiaList verification — synthetic providers", WS_OVERLAPPEDWINDOW, 820, 80, 650, 650, nullptr, nullptr, instance, context);
	if (window) ShowWindow(window, SW_SHOW);
	return window;
}

void DestroySyntheticFixture(HWND window) { if (IsWindow(window)) DestroyWindow(window); }
