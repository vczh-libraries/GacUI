#ifndef VCZH_PRESENTATION_WINDOWS_UIAUTOMATIONPROVIDER
#define VCZH_PRESENTATION_WINDOWS_UIAUTOMATIONPROVIDER

#include "WindowsUIAutomation.Windows.h"
#include "../../../GacUI.h"
#include "../../../Controls/Templates/GuiCommonTemplates.h"

#ifdef VCZH_MSVC
#include <UIAutomation.h>
#include <atomic>

namespace vl::presentation::windows
{
	class WindowsUIAutomationProvider;
	class WindowsUIAutomationTextRange;
	struct WindowsUIAutomationIdleRequest;
	struct WindowsUIAutomationNode;

	struct WindowsUIAutomationMetadata : Object
	{
		Nullable<WString>					name;
		WString								id;
		controls::GuiControl*				label = nullptr;
		Ptr<controls::GuiDisposedFlag>		labelDisposed;
		collections::Dictionary<WString, WString> texts;
	};
	extern Ptr<WindowsUIAutomationMetadata> UiaMetadata(controls::GuiControl* control, bool create = false);
	extern WString UiaLocalizedText(controls::GuiControl* control, const WString& key, const WString& fallback);
	extern WString UiaTooltipText(controls::GuiControl* control);
	extern bool UiaTooltipInteractive(controls::GuiControl* control);
	extern bool UiaDocumentObjectRange(WindowsUIAutomationNode* node, vint& begin, vint& end);
	extern WString UiaDocumentObjectName(WindowsUIAutomationNode* node);
	extern UiaRect UiaDocumentObjectBounds(WindowsUIAutomationNode* node);
	extern void UiaInvokeDocumentObject(WindowsUIAutomationNode* node);
	extern Ptr<WindowsUIAutomationNode> UiaDocumentObjectParent(WindowsUIAutomationNode* node);
	extern Ptr<WindowsUIAutomationNode> UiaRadioGroup(WindowsUIAutomationNode* node);
	extern Size UiaSpatialGrid(controls::GuiControl* control);
	extern bool UiaSpatialColumnMajor(controls::GuiControl* control);
	extern GridPos UiaGridPosition(WindowsUIAutomationNode* node);

	class WindowsUIAutomationDispatcher : public Object
	{
		std::atomic<HWND>					window = nullptr;
		DWORD								threadId;
		SpinLock							lockPosting;
		collections::List<Ptr<WindowsUIAutomationIdleRequest>> idleRequests;
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	public:
		WindowsUIAutomationDispatcher();
		~WindowsUIAutomationDispatcher();
		HRESULT Run(const Func<HRESULT()>& action);
		HRESULT WaitForIdle(int milliseconds, BOOL* result, const Func<HRESULT()>& validate);
		void Queue(const Func<void()>& action);
		void Stop();
	};

	enum class WindowsUIAutomationNodeKind
	{
		Control, Item, Cell, Header, HeaderItem, TreeNode, CalendarDay, CalendarHeader, TabContent, DocumentObject, RadioGroup,
	};

	struct WindowsUIAutomationValue : Object
	{
		VARIANT								value;
		WindowsUIAutomationValue();
		~WindowsUIAutomationValue();
	};

	struct WindowsUIAutomationNode : Object
	{
		WindowsUIAutomationContext*			context = nullptr;
		Ptr<WindowsUIAutomationDispatcher>	dispatcher;
		Ptr<controls::GuiDisposedFlag>		disposed;
		controls::GuiControl*				control = nullptr;
		Ptr<WindowsUIAutomationNode>			owner;
		Ptr<controls::tree::INodeProvider>	treeNode;
		Ptr<DocumentRun>						documentRun;
		WindowsUIAutomationNodeKind			kind = WindowsUIAutomationNodeKind::Control;
		vint								id = 0;
		vint								row = -1;
		vint								column = -1;
		bool								retired = false;
		bool								notificationPending = false;
		bool								structurePending = false;
		collections::SortedList<EVENTID>	pendingEvents;
		collections::List<vint>				selection;
		ComPtr<IRawElementProviderSimple>	provider;
		collections::Dictionary<PROPERTYID, Ptr<WindowsUIAutomationValue>> properties;

		bool IsLive();
		controls::GuiControl* Control();
		controls::GuiWindow* Window();
		HWND Handle();
		NativePoint ScreenOrigin();
		bool IsRoot();
		compositions::GuiGraphicsComposition* Composition();
		WString Name();
		CONTROLTYPEID Role();
		bool Supports(PATTERNID pattern);
		bool IsSelected();
		bool IsFocusable();
		bool IsFocused();
		vint ItemIndex();
		UiaRect Bounds();
		Ptr<WindowsUIAutomationNode> Parent();
		collections::List<Ptr<WindowsUIAutomationNode>> Children();
		HRESULT Property(PROPERTYID property, VARIANT* result);
		IRawElementProviderSimple* Provider();
		void Retire();
	};

	class WindowsUIAutomationLifetime : public Object, public controls::list::IItemProviderCallback, public controls::tree::INodeProviderCallback, public controls::list::IColumnItemViewCallback
	{
	public:
		Ptr<WindowsUIAutomationNode>			node;
		controls::list::IItemProvider*		items = nullptr;
		Ptr<controls::tree::INodeRootProvider> tree;
		controls::list::IColumnItemView*		columns = nullptr;
		~WindowsUIAutomationLifetime();
		void OnAttached(controls::list::IItemProvider* provider)override;
		void OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void OnAttached(controls::tree::INodeRootProvider* provider)override;
		void OnBeforeItemModified(controls::tree::INodeProvider* parent, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void OnAfterItemModified(controls::tree::INodeProvider* parent, vint start, vint count, vint newCount, bool itemReferenceUpdated)override;
		void OnItemExpanded(controls::tree::INodeProvider* value)override;
		void OnItemCollapsed(controls::tree::INodeProvider* value)override;
		void OnColumnRebuilt()override;
		void OnColumnChanged(bool needToRefreshItems)override;
	};

	class WindowsUIAutomationContext : public Object
	{
	public:
		bool								hosted;
		bool								stopped = false;
		vint								nextId = 1;
		Ptr<WindowsUIAutomationDispatcher>	dispatcher;
		collections::Dictionary<INativeWindow*, HWND> windows;
		collections::Dictionary<INativeWindow*, Ptr<WindowsUIAutomationNode>> roots;
		collections::List<Ptr<WindowsUIAutomationNode>> nodes;
		collections::List<Ptr<WindowsUIAutomationNode>> combos;
		collections::List<Ptr<WindowsUIAutomationNode>> openMenus;
		Ptr<WindowsUIAutomationNode>			menuModeOwner;
		WindowsUIAutomationContext(bool isHostedMode);
		void Stop();
		void BindWindows();
		void WindowEvent(Ptr<WindowsUIAutomationNode> node, bool opening);
		void Scan(compositions::GuiGraphicsComposition* composition);
		Ptr<WindowsUIAutomationNode> Control(controls::GuiControl* control);
		Ptr<WindowsUIAutomationNode> Item(Ptr<WindowsUIAutomationNode> owner, WindowsUIAutomationNodeKind kind, vint row = -1, vint column = -1, Ptr<controls::tree::INodeProvider> treeNode = nullptr, Ptr<DocumentRun> documentRun = nullptr);
		void Notify(Ptr<WindowsUIAutomationNode> node, bool structure = false, EVENTID eventId = 0);
		void UpdateProperties(Ptr<WindowsUIAutomationNode> node, bool raiseEvents);
		void UpdateSelection(Ptr<WindowsUIAutomationNode> node, bool raiseEvents);
	};

	class WindowsUIAutomationProvider : public IRawElementProviderSimple, public IRawElementProviderFragment, public IRawElementProviderFragmentRoot,
		public IInvokeProvider, public IToggleProvider, public ISelectionProvider, public ISelectionItemProvider,
		public IExpandCollapseProvider, public IValueProvider, public IRangeValueProvider, public IScrollProvider,
		public IScrollItemProvider, public IGridProvider, public IGridItemProvider, public ITableProvider,
		public ITableItemProvider, public IItemContainerProvider, public IVirtualizedItemProvider,
		public IMultipleViewProvider, public IWindowProvider, public ITransformProvider, public ITextProvider
	{
		std::atomic<ULONG>					references = 1;
		const bool							rootProvider;
	public:
		Ptr<WindowsUIAutomationNode>			node;
		WindowsUIAutomationProvider(Ptr<WindowsUIAutomationNode> value);
		HRESULT Read(const Func<HRESULT()>& action, PATTERNID pattern = 0, bool enabled = false);
		HRESULT Queue(const Func<void()>& action, PATTERNID pattern);
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result)override;
		ULONG STDMETHODCALLTYPE AddRef()override;
		ULONG STDMETHODCALLTYPE Release()override;
		HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions* result)override;
		HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID pattern, IUnknown** result)override;
		HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID property, VARIANT* result)override;
		HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(IRawElementProviderSimple** result)override;
		HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction, IRawElementProviderFragment** result)override;
		HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* result)override;
		HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE SetFocus()override;
		HRESULT STDMETHODCALLTYPE get_FragmentRoot(IRawElementProviderFragmentRoot** result)override;
		HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(double x, double y, IRawElementProviderFragment** result)override;
		HRESULT STDMETHODCALLTYPE GetFocus(IRawElementProviderFragment** result)override;
		HRESULT STDMETHODCALLTYPE Invoke()override;
		HRESULT STDMETHODCALLTYPE Toggle()override;
		HRESULT STDMETHODCALLTYPE get_ToggleState(ToggleState* result)override;
		HRESULT STDMETHODCALLTYPE GetSelection(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE get_CanSelectMultiple(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_IsSelectionRequired(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE Select()override;
		HRESULT STDMETHODCALLTYPE AddToSelection()override;
		HRESULT STDMETHODCALLTYPE RemoveFromSelection()override;
		HRESULT STDMETHODCALLTYPE get_IsSelected(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_SelectionContainer(IRawElementProviderSimple** result)override;
		HRESULT STDMETHODCALLTYPE Expand()override;
		HRESULT STDMETHODCALLTYPE Collapse()override;
		HRESULT STDMETHODCALLTYPE get_ExpandCollapseState(ExpandCollapseState* result)override;
		HRESULT STDMETHODCALLTYPE SetValue(LPCWSTR value)override;
		HRESULT STDMETHODCALLTYPE get_Value(BSTR* result)override;
		HRESULT STDMETHODCALLTYPE get_IsReadOnly(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE SetValue(double value)override;
		HRESULT STDMETHODCALLTYPE get_Value(double* result)override;
		HRESULT STDMETHODCALLTYPE get_Maximum(double* result)override;
		HRESULT STDMETHODCALLTYPE get_Minimum(double* result)override;
		HRESULT STDMETHODCALLTYPE get_LargeChange(double* result)override;
		HRESULT STDMETHODCALLTYPE get_SmallChange(double* result)override;
		HRESULT STDMETHODCALLTYPE Scroll(ScrollAmount horizontal, ScrollAmount vertical)override;
		HRESULT STDMETHODCALLTYPE SetScrollPercent(double horizontal, double vertical)override;
		HRESULT STDMETHODCALLTYPE get_HorizontalScrollPercent(double* result)override;
		HRESULT STDMETHODCALLTYPE get_VerticalScrollPercent(double* result)override;
		HRESULT STDMETHODCALLTYPE get_HorizontalViewSize(double* result)override;
		HRESULT STDMETHODCALLTYPE get_VerticalViewSize(double* result)override;
		HRESULT STDMETHODCALLTYPE get_HorizontallyScrollable(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_VerticallyScrollable(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE ScrollIntoView()override;
		HRESULT STDMETHODCALLTYPE GetItem(int row, int column, IRawElementProviderSimple** result)override;
		HRESULT STDMETHODCALLTYPE get_RowCount(int* result)override;
		HRESULT STDMETHODCALLTYPE get_ColumnCount(int* result)override;
		HRESULT STDMETHODCALLTYPE get_Row(int* result)override;
		HRESULT STDMETHODCALLTYPE get_Column(int* result)override;
		HRESULT STDMETHODCALLTYPE get_RowSpan(int* result)override;
		HRESULT STDMETHODCALLTYPE get_ColumnSpan(int* result)override;
		HRESULT STDMETHODCALLTYPE get_ContainingGrid(IRawElementProviderSimple** result)override;
		HRESULT STDMETHODCALLTYPE GetRowHeaders(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE GetColumnHeaders(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE get_RowOrColumnMajor(RowOrColumnMajor* result)override;
		HRESULT STDMETHODCALLTYPE GetRowHeaderItems(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE GetColumnHeaderItems(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE FindItemByProperty(IRawElementProviderSimple* startAfter, PROPERTYID property, VARIANT value, IRawElementProviderSimple** result)override;
		HRESULT STDMETHODCALLTYPE Realize()override;
		HRESULT STDMETHODCALLTYPE GetViewName(int viewId, BSTR* result)override;
		HRESULT STDMETHODCALLTYPE SetCurrentView(int viewId)override;
		HRESULT STDMETHODCALLTYPE get_CurrentView(int* result)override;
		HRESULT STDMETHODCALLTYPE GetSupportedViews(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE SetVisualState(WindowVisualState state)override;
		HRESULT STDMETHODCALLTYPE Close()override;
		HRESULT STDMETHODCALLTYPE WaitForInputIdle(int milliseconds, BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_CanMaximize(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_CanMinimize(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_IsModal(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_WindowVisualState(WindowVisualState* result)override;
		HRESULT STDMETHODCALLTYPE get_WindowInteractionState(WindowInteractionState* result)override;
		HRESULT STDMETHODCALLTYPE get_IsTopmost(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE Move(double x, double y)override;
		HRESULT STDMETHODCALLTYPE Resize(double width, double height)override;
		HRESULT STDMETHODCALLTYPE Rotate(double degrees)override;
		HRESULT STDMETHODCALLTYPE get_CanMove(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_CanResize(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE get_CanRotate(BOOL* result)override;
		HRESULT STDMETHODCALLTYPE GetVisibleRanges(SAFEARRAY** result)override;
		HRESULT STDMETHODCALLTYPE RangeFromChild(IRawElementProviderSimple* child, ITextRangeProvider** result)override;
		HRESULT STDMETHODCALLTYPE RangeFromPoint(UiaPoint point, ITextRangeProvider** result)override;
		HRESULT STDMETHODCALLTYPE get_DocumentRange(ITextRangeProvider** result)override;
		HRESULT STDMETHODCALLTYPE get_SupportedTextSelection(SupportedTextSelection* result)override;
	};

	extern HRESULT UiaNodeArray(const collections::List<Ptr<WindowsUIAutomationNode>>& nodes, SAFEARRAY** result);
	extern HRESULT UiaString(const WString& value, BSTR* result);
	extern controls::list::IListViewItemView* UiaListView(controls::GuiControl* control);
	extern controls::list::IColumnItemView* UiaColumns(controls::GuiControl* control);
	extern controls::list::IDataGridView* UiaDataGrid(controls::GuiControl* control);
	extern controls::GuiDocumentCommonInterface* UiaDocument(controls::GuiControl* control);
	extern templates::GuiCommonDatePickerLook* UiaCalendar(controls::GuiControl* control);
	extern controls::GuiComboBoxListControl* UiaCombo(WindowsUIAutomationNode* node);
	extern controls::GuiControl* UiaPopupOwner(WindowsUIAutomationNode* node);
	extern void UiaSelectedChildren(Ptr<WindowsUIAutomationNode> node, collections::List<Ptr<WindowsUIAutomationNode>>& selected);
	extern collections::List<Ptr<WindowsUIAutomationNode>> UiaTextChildren(WindowsUIAutomationNode* node, vint begin, vint end);
	extern void UiaCollectChildren(WindowsUIAutomationContext* context, compositions::GuiGraphicsComposition* composition, collections::List<Ptr<WindowsUIAutomationNode>>& children);
}
#endif
#endif
