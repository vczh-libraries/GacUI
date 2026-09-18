#ifndef VCZH_UIALIST_VIEWMODEL
#define VCZH_UIALIST_VIEWMODEL

#include "../Source/UiaListPartialClasses.h"
#include "UiaSession.Windows.h"
#include "WindowCapture.Windows.h"
#undef GetRoleText

namespace uialist
{
	class UiaListViewModel;
	class PropertyDialogViewModel;

	struct UiLifetime
	{
		UiaListViewModel*				owner = nullptr;
		vl::atomic_vint					generation = 0;
		vl::atomic_vint					processSerial = 0;
		vl::atomic_vint					dialogSerial = 0;
		std::atomic<bool>				closed = false;
	};

	class ProcessNodeViewModel : public vl::Object, public virtual vm::IProcessNodeViewModel
	{
	public:
		vm::ProcessNodeKind				kind = vm::ProcessNodeKind::SyntheticRoot;
		vl::WString						displayText;
		vl::WString						executable;
		DWORD							processId = 0;
		DWORD							parentId = 0;
		vl::Nullable<vl::vuint64_t>		creationTime;
		native::WindowRecord			window;
		vl::collections::ObservableList<vl::Ptr<vm::IProcessNodeViewModel>> children;
		vl::collections::ObservableList<vl::Ptr<vm::IProcessNodeViewModel>> windows;
		bool							expanded = false;

		vm::ProcessNodeKind				GetKind() override;
		vl::WString						GetDisplayText() override;
		vl::vuint						GetProcessId() override;
		vl::vuint						GetParentId() override;
		vl::WString						GetExecutable() override;
		vl::Nullable<vl::vuint64_t>		GetCreationTime() override;
		vl::vuint64_t					GetWindowKey() override;
		vl::WString						GetWindowTitle() override;
		vl::WString						GetWindowClass() override;
		vl::WString						GetWindowHandleText() override;
		vl::Ptr<vl::reflection::description::IValueList> GetChildren() override;
		vl::Ptr<vl::reflection::description::IValueList> GetWindows() override;
		bool							GetIsExpanded() override;
		void							SetIsExpanded(bool value) override;
	};

	class NodeViewModel : public vl::Object, public virtual vm::INodeViewModel
	{
	public:
		native::NodeData					data;
		vl::vint						generation = 0;
		vl::WString						displayText;
		vl::collections::ObservableList<vl::Ptr<vm::INodeViewModel>> children;
		bool							expanded = false;
		bool							selected = false;

		bool							GetIsSyntheticRoot() override;
		vl::vint						GetNodeId() override;
		vl::vint						GetParentId() override;
		vl::WString						GetRuntimeIdText() override;
		vl::Ptr<vl::reflection::description::IValueList> GetChildren() override;
		vl::WString						GetDisplayText() override;
		vl::WString						GetRoleText() override;
		vl::WString						GetClientInterfaceName() override;
		vl::WString						GetProviderNames() override;
		vl::presentation::Rect			GetBounds() override;
		bool							GetIsOffscreen() override;
		bool							GetIsExpanded() override;
		void							SetIsExpanded(bool value) override;
		bool							GetIsSelected() override;
	};

	class PreviewViewModel : public vl::Object, public virtual vm::IPreviewViewModel
	{
	public:
		UiaListViewModel* owner;
		vl::Ptr<native::CaptureSnapshot> capture;
		vl::Ptr<vl::presentation::GuiImageData> image;
		vl::vint hoverKey = 0;
		vl::presentation::Rect hoverBounds;
		PreviewViewModel(UiaListViewModel& root);
		vl::Ptr<vl::presentation::GuiImageData> GetImage() override;
		vl::presentation::Size GetImageSize() override;
		bool GetIsAvailable() override;
		vl::WString GetStatus() override;
		vl::WString GetHoverLabel() override;
		vl::presentation::Rect GetHoverBounds() override;
		bool GetHasHover() override;
		void Hover(vl::vint x, vl::vint y) override;
		void Leave() override;
		void Click(vl::vint x, vl::vint y) override;
		void Publish(vl::Ptr<native::CaptureSnapshot> result);
		vl::vint HitTest(vl::vint x, vl::vint y);
	};

	class UiaListViewModel : public vl::Object, public virtual vm::IUiaListViewModel
	{
	public:
		vl::Ptr<IStringsStrings>			strings;
		vl::WString						localeName;
		vl::vint						activeTab = 0;
		bool							initialized = false;
		bool							processBusy = false;
		bool							treeBusy = false;
		vl::vint						loadedNodes = 0;
		vl::Ptr<UiLifetime>				lifetime;
		native::UiaWorker				worker;
		native::CaptureWorker			captureWorker;
		vl::Ptr<native::CaptureRequest> captureRequest;
		vl::Ptr<native::TreeSnapshot> pendingTree;
		vl::Ptr<native::CaptureSnapshot> pendingCapture;
		vl::Ptr<PreviewViewModel>		preview;
		vl::Ptr<ProcessNodeViewModel>	processRoot;
		vl::Ptr<ProcessNodeViewModel>	selectedProcess;
		vl::collections::Dictionary<DWORD, vl::Ptr<ProcessNodeViewModel>> processes;
		vl::Ptr<ProcessNodeViewModel>	selectedWindow;
		vl::Ptr<NodeViewModel>			nodeRoot;
		vl::Ptr<NodeViewModel>			selectedNode;
		vl::collections::Dictionary<vl::vint, vl::Ptr<NodeViewModel>> nodes;
		vl::Ptr<PropertyDialogViewModel>	propertyDialog;
		vl::Ptr<native::TreeSnapshot>	snapshot;

										UiaListViewModel(vl::Ptr<IStringsStrings> value, const vl::WString& locale);
										~UiaListViewModel();
		vl::Ptr<vm::IProcessNodeViewModel> GetProcessRoot() override;
		vl::Ptr<vm::IProcessNodeViewModel> GetSelectedProcess() override;
		vl::Ptr<vl::reflection::description::IValueList> GetWindows() override;
		vl::Ptr<vm::INodeViewModel>		GetNodeRoot() override;
		vl::Ptr<vm::IProcessNodeViewModel> GetSelectedWindow() override;
		vl::Ptr<vm::INodeViewModel>		GetSelectedNode() override;
		vl::Ptr<vm::IPreviewViewModel>	GetPreview() override;
		bool							GetHasSelection() override;
		vl::vint						GetActiveTab() override;
		void							SetActiveTab(vl::vint value) override;
		bool							GetIsBusy() override;
		vl::WString						GetStatus() override;
		vl::WString						GetLocaleName() override;
		vl::Ptr<vm::IPropertyDialogViewModel> GetPropertyDialog() override;
		void							Initialize() override;
		void							RefreshProcesses() override;
		void							RefreshWindow() override;
		void							RefreshWindowInternal(bool closeDialog);
		void							SelectProcess(vl::Ptr<vm::IProcessNodeViewModel> process) override;
		void							SelectWindow(vl::Ptr<vm::IProcessNodeViewModel> window) override;
		void							SelectNode(vl::Ptr<vm::INodeViewModel> node) override;
		void							InspectNode(vl::Ptr<vm::INodeViewModel> node) override;
		void							RequestClose() override;
		void							ClearSelection();
		void							PublishTree(vl::Ptr<native::TreeSnapshot> result);
		void							PublishSnapshot();
		void							QueueNative(const vl::WString& operation, vl::Func<void()> task);
		static void						Post(vl::Ptr<UiLifetime> lifetime, vl::Func<void(UiaListViewModel&)> completion);
	};
}

#endif
