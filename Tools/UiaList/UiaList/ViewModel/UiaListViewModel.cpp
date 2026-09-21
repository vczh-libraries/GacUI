#include "UiaListViewModel.h"
#include "PropertyViewModel.h"
#include "ActionViewModel.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::controls;

namespace uialist
{
	template<typename T>
	void ReleaseTreeChildren(Ptr<T> root)
	{
		// Retain every node while severing ownership from the leaves upward. Raw
		// providers can be arbitrarily deep; recursive Ptr destruction exhausts
		// the UI thread stack even when traversal itself is iterative.
		List<Ptr<T>> retained;
		if (root) retained.Add(root);
		for (vint i = 0; i < retained.Count(); i++)
		{
			for (auto&& child : retained[i]->children) retained.Add(child.template Cast<T>());
		}
		for (vint i = retained.Count(); i > 0; i--) retained[i - 1]->children.Clear();
	}

	PreviewViewModel::PreviewViewModel(UiaListViewModel& root) : owner(&root) {}
	Ptr<GuiImageData> PreviewViewModel::GetImage() { return image; }
	Size PreviewViewModel::GetImageSize() { return capture && image ? Size(capture->bounds.right - capture->bounds.left, capture->bounds.bottom - capture->bounds.top) : Size(); }
	bool PreviewViewModel::GetIsAvailable() { return image && !owner->treeBusy; }
	WString PreviewViewModel::GetStatus()
	{
		if (owner->treeBusy) return owner->strings->Loading();
		if (!owner->selectedWindow) return owner->strings->ChooseWindow();
		if (!capture || capture->status == native::CaptureStatus::Unavailable || capture->status == native::CaptureStatus::Canceled) return owner->strings->PreviewUnavailable();
		if (capture->status == native::CaptureStatus::GeometryChanged) return owner->strings->PreviewGeometryChanged();
		return owner->strings->Captured() + L": " + capture->timestamp + L" (" + utow(capture->dpi) + L" DPI)";
	}
	WString PreviewViewModel::GetHoverLabel() { return hoverKey && owner->nodes.Keys().Contains(hoverKey) ? owner->nodes[hoverKey]->displayText : WString(); }
	Rect PreviewViewModel::GetHoverBounds() { return hoverBounds; }
	bool PreviewViewModel::GetHasHover() { return hoverKey != 0 && GetIsAvailable(); }
	void PreviewViewModel::Publish(Ptr<native::CaptureSnapshot> result)
	{
		capture = result;
		image = nullptr;
		if (capture && capture->status == native::CaptureStatus::Ready)
		{
			auto nativeImage = GetCurrentController()->ImageService()->CreateImageFromMemory(&capture->bitmap[0], capture->bitmap.Count());
			CHECK_ERROR(nativeImage, L"Unable to decode captured window bitmap.");
			image = Ptr(new GuiImageData(nativeImage, 0));
		}
		Leave();
		ImageChanged(); ImageSizeChanged(); IsAvailableChanged(); StatusChanged();
	}
	vint PreviewViewModel::HitTest(vint x, vint y)
	{
		auto size = GetImageSize();
		if (!GetIsAvailable() || x < 0 || y < 0 || x >= size.x || y >= size.y) return 0;
		auto px = x + capture->bounds.left, py = y + capture->bounds.top;
		auto contains = [&](const native::NodeData& node)
		{
			auto b = node.bounds;
			return !node.offscreen && b.right > b.left && b.bottom > b.top && px >= b.left && py >= b.top && px < b.right && py < b.bottom;
		};
		auto&& nodes = owner->snapshot->nodes;
		vint begin = 0, end = nodes.Count(), windowDepth = -1;
		for (vint i = 0; i < nodes.Count(); i++)
		{
			auto&& node = nodes[i];
			if (node.controlType == UIA_WindowControlTypeId && contains(node) && node.depth > windowDepth)
			{
				begin = i;
				windowDepth = node.depth;
			}
		}
		// Hosted windows can be shallower than the page behind them. Resolve the
		// window branch before comparing descendant depths. Raw nodes are preorder.
		if (windowDepth >= 0)
		{
			end = begin + 1;
			while (end < nodes.Count() && nodes[end].depth > windowDepth) end++;
		}
		vint key = 0, depth = -1;
		for (vint i = begin; i < end; i++)
		{
			auto&& node = nodes[i];
			if (contains(node) && node.depth > depth)
			{
				key = node.key;
				depth = node.depth;
			}
		}
		return key;
	}
	void PreviewViewModel::Hover(vint x, vint y)
	{
		auto key = HitTest(x, y);
		if (key == hoverKey) return;
		hoverKey = key;
		hoverBounds = {};
		if (key)
		{
			auto b = owner->nodes[key]->data.bounds;
			auto size = GetImageSize();
			hoverBounds = Rect((std::max)(0L, b.left - capture->bounds.left), (std::max)(0L, b.top - capture->bounds.top), (std::min)(static_cast<LONG>(size.x), b.right - capture->bounds.left), (std::min)(static_cast<LONG>(size.y), b.bottom - capture->bounds.top));
		}
		HoverBoundsChanged(); HoverLabelChanged(); HasHoverChanged();
	}
	void PreviewViewModel::Leave() { hoverKey = 0; hoverBounds = {}; HoverBoundsChanged(); HoverLabelChanged(); HasHoverChanged(); }
	void PreviewViewModel::Click(vint x, vint y)
	{
		auto key = HitTest(x, y);
		if (!key) return;
		for (auto current = owner->nodes[key]->data.parent; current; current = owner->nodes[current]->data.parent) owner->nodes[current]->SetIsExpanded(true);
		owner->SelectNode(owner->nodes[key]);
		owner->SetActiveTab(2);
	}

	vm::ProcessNodeKind ProcessNodeViewModel::GetKind() { return kind; }
	WString ProcessNodeViewModel::GetDisplayText() { return displayText; }
	vuint ProcessNodeViewModel::GetProcessId() { return processId; }
	vuint ProcessNodeViewModel::GetParentId() { return parentId; }
	WString ProcessNodeViewModel::GetExecutable() { return executable; }
	Nullable<vuint64_t> ProcessNodeViewModel::GetCreationTime() { return creationTime; }
	vuint64_t ProcessNodeViewModel::GetWindowKey() { return reinterpret_cast<UINT_PTR>(window.identity.handle); }
	WString ProcessNodeViewModel::GetWindowTitle() { return displayText; }
	WString ProcessNodeViewModel::GetWindowClass() { return window.className; }
	WString ProcessNodeViewModel::GetWindowHandleText() { return native::Hex(GetWindowKey()); }
	Ptr<IValueList> ProcessNodeViewModel::GetChildren() { return UnboxValue<Ptr<IValueList>>(BoxParameter(children)); }
	Ptr<IValueList> ProcessNodeViewModel::GetWindows() { return UnboxValue<Ptr<IValueList>>(BoxParameter(windows)); }
	bool ProcessNodeViewModel::GetIsExpanded() { return expanded; }
	void ProcessNodeViewModel::SetIsExpanded(bool value) { if (expanded != value) { expanded = value; IsExpandedChanged(); } }

	bool NodeViewModel::GetIsSyntheticRoot() { return data.key == 0; }
	vint NodeViewModel::GetNodeId() { return data.key; }
	vint NodeViewModel::GetParentId() { return data.parent; }
	WString NodeViewModel::GetRuntimeIdText() { return data.runtimeId; }
	Ptr<IValueList> NodeViewModel::GetChildren() { return UnboxValue<Ptr<IValueList>>(BoxParameter(children)); }
	WString NodeViewModel::GetDisplayText() { return displayText; }
	WString NodeViewModel::GetRoleText() { return data.role; }
	WString NodeViewModel::GetClientInterfaceName() { return data.client; }
	WString NodeViewModel::GetProviderNames() { return data.providers; }
	Rect NodeViewModel::GetBounds() { return Rect(data.bounds.left, data.bounds.top, data.bounds.right, data.bounds.bottom); }
	bool NodeViewModel::GetIsOffscreen() { return data.offscreen; }
	bool NodeViewModel::GetIsExpanded() { return expanded; }
	void NodeViewModel::SetIsExpanded(bool value) { if (expanded != value) { expanded = value; IsExpandedChanged(); } }
	bool NodeViewModel::GetIsSelected() { return selected; }

	WString SingleLine(const WString& text)
	{
		Array<wchar_t> buffer(text.Length());
		for (vint i = 0; i < text.Length(); i++)
		{
			auto c = text[i];
			buffer[i] = c == L'\r' || c == L'\n' || c == L'\t' ? L' ' : c;
		}
		return buffer.Count() ? WString::CopyFrom(&buffer[0], buffer.Count()) : WString();
	}

	UiaListViewModel::UiaListViewModel(Ptr<IStringsStrings> value, const WString& locale)
		: strings(value)
		, localeName(locale)
		, lifetime(Ptr(new UiLifetime))
		, processRoot(Ptr(new ProcessNodeViewModel))
		, nodeRoot(Ptr(new NodeViewModel))
	{
		lifetime->owner = this;
		preview = Ptr(new PreviewViewModel(*this));
	}

	UiaListViewModel::~UiaListViewModel()
	{
		RequestClose();
	}
	Ptr<vm::IProcessNodeViewModel> UiaListViewModel::GetProcessRoot() { return processRoot; }
	Ptr<vm::IProcessNodeViewModel> UiaListViewModel::GetSelectedProcess() { return selectedProcess; }
	Ptr<IValueList> UiaListViewModel::GetWindows() { return selectedProcess ? selectedProcess->GetWindows() : nullptr; }
	Ptr<vm::INodeViewModel> UiaListViewModel::GetNodeRoot() { return nodeRoot; }
	Ptr<vm::IProcessNodeViewModel> UiaListViewModel::GetSelectedWindow() { return selectedWindow; }
	Ptr<vm::INodeViewModel> UiaListViewModel::GetSelectedNode() { return selectedNode; }
	Ptr<vm::IPreviewViewModel> UiaListViewModel::GetPreview() { return preview; }
	bool UiaListViewModel::GetHasSelection() { return selectedWindow != nullptr; }
	vint UiaListViewModel::GetActiveTab() { return activeTab; }
	void UiaListViewModel::SetActiveTab(vint value)
	{
		if (activeTab != value) { activeTab = value; ActiveTabChanged(); }
	}
	bool UiaListViewModel::GetIsBusy() { return processBusy || treeBusy; }
	WString UiaListViewModel::GetStatus() { return treeBusy ? strings->Loading() + L" (" + itow(loadedNodes) + L")" : processBusy ? strings->Loading() : selectedWindow ? strings->Ready() + L" (" + itow(nodes.Count()) + L")" : strings->ChooseWindow(); }
	WString UiaListViewModel::GetLocaleName() { return localeName; }
	Ptr<vm::IPropertyDialogViewModel> UiaListViewModel::GetPropertyDialog() { return propertyDialog; }

	void UiaListViewModel::Post(Ptr<UiLifetime> gate, Func<void(UiaListViewModel&)> completion)
	{
		GetApplication()->InvokeInMainThread(nullptr, [gate, completion]()
		{
			if (!gate->closed && gate->owner) completion(*gate->owner);
		});
	}

	void UiaListViewModel::QueueNative(const WString& operation, Func<void()> task)
	{
		auto context = operation + L"; generation=" + itow(lifetime->generation);
		if (selectedWindow)
		{
			context += L"; HWND=" + native::Hex(selectedWindow->GetWindowKey()) + L"; PID=" + utow(selectedWindow->processId);
		}
		auto gate = lifetime;
		auto epoch = lifetime->generation.load();
		worker.queue.QueueTask([task, context, gate, epoch]()
		{
			try
			{
				try { task(); }
				catch (const native::UiaFailure& error)
				{
					if (!error.IsExpected()) throw;
					auto message = context + L"\r\n" + error.Message();
					Post(gate, [message, epoch](UiaListViewModel& root)
					{
						if (root.lifetime->generation != epoch) return;
						if (root.propertyDialog && root.propertyDialog->open)
						{
							auto dialog = root.propertyDialog;
							dialog->busy = false;
							dialog->status = message;
							dialog->StatusChanged();
							dialog->NotifyAvailability();
						}
					});
				}
			}
			catch (const Exception& error)
			{
				auto message = context + L"\r\n" + error.Message();
				GetApplication()->InvokeInMainThread(nullptr, [message]()
				{
					OutputDebugStringW(message.Buffer());
					MessageBoxW(nullptr, message.Buffer(), L"UiaList", MB_OK | MB_ICONERROR);
					ExitProcess(1);
				});
			}
		});
	}

	void UiaListViewModel::Initialize()
	{
		if (initialized) return;
		initialized = true;
		worker.Start();
		captureWorker.Start();
		RefreshProcesses();
	}

	void UiaListViewModel::RefreshProcesses()
	{
		if (lifetime->closed) return;
		auto serial = ++lifetime->processSerial;
		auto gate = lifetime;
		processBusy = true;
		IsBusyChanged();
		StatusChanged();
		QueueNative(L"DiscoverProcesses", [gate, serial]()
		{
			if (gate->closed || gate->processSerial != serial) return;
			auto result = native::DiscoverProcesses();
			Post(gate, [result, serial](UiaListViewModel& root)
			{
				if (root.lifetime->processSerial != serial) return;
				List<Ptr<ProcessNodeViewModel>> processNodes;
				Dictionary<WString, bool> expansion;
				List<Ptr<vm::IProcessNodeViewModel>> previous; previous.Add(root.processRoot);
				for (vint i = 0; i < previous.Count(); i++)
				{
					auto p = previous[i].Cast<ProcessNodeViewModel>();
					if (p->kind == vm::ProcessNodeKind::Process) expansion.Set(utow(p->processId) + L"/" + (p->creationTime ? u64tow(p->creationTime.Value()) : WString()), p->expanded);
					for (auto&& child : p->children) previous.Add(child);
				}
				Ptr<ProcessNodeViewModel> survivingSelection;
				Ptr<ProcessNodeViewModel> survivingProcess;
				root.processes.Clear();
				for (auto&& record : result->processes)
				{
					auto node = Ptr(new ProcessNodeViewModel);
					node->kind = vm::ProcessNodeKind::Process;
					node->processId = record.processId;
					node->parentId = record.parentId;
					node->executable = record.executable;
					if (record.creationTime) node->creationTime = record.creationTime.Value();
					node->displayText = record.executable + L" [" + utow(record.processId) + L"]";
					auto processKey = utow(record.processId) + L"/" + (record.creationTime ? u64tow(record.creationTime.Value()) : WString());
					node->expanded = expansion.Keys().Contains(processKey) ? expansion[processKey] : true;
					if (record.hasVisibleUI)
					{
						root.processes.Add(record.processId, node);
						if (root.selectedProcess && root.selectedProcess->processId == record.processId
							&& (!root.selectedProcess->creationTime || !node->creationTime || root.selectedProcess->creationTime == node->creationTime)) survivingProcess = node;
					}
					for (auto&& window : record.windows)
					{
						if (!window.qualifies) continue;
						auto child = Ptr(new ProcessNodeViewModel);
						child->kind = vm::ProcessNodeKind::Window;
						child->processId = record.processId;
						child->executable = record.executable;
						child->window = window;
						child->displayText = SingleLine(window.title.Length() ? window.title : root.strings->Untitled());
						node->windows.Add(child);
						if (root.selectedWindow && root.selectedWindow->window.identity == window.identity) survivingSelection = child;
					}
					processNodes.Add(node);
				}
				for (vint i = 0; i < result->processes.Count(); i++)
				{
					for (auto index : result->processes[i].children) processNodes[i]->children.Add(processNodes[index]);
				}
				auto synthetic = Ptr(new ProcessNodeViewModel);
				for (auto index : result->roots) synthetic->children.Add(processNodes[index]);
				ReleaseTreeChildren(root.processRoot);
				root.processRoot = synthetic;
				root.selectedProcess = survivingProcess;
				root.ProcessRootChanged();
				root.SelectedProcessChanged();
				root.WindowsChanged();
				if (root.selectedWindow && !survivingSelection) root.ClearSelection();
				else if (survivingSelection) { root.selectedWindow = survivingSelection; root.SelectedWindowChanged(); }
				root.processBusy = false;
				root.IsBusyChanged();
				root.StatusChanged();
			});
		});
	}

	void UiaListViewModel::SelectProcess(Ptr<vm::IProcessNodeViewModel> value)
	{
		auto process = value.Cast<ProcessNodeViewModel>();
		if (!process || !processes.Keys().Contains(process->processId) || processes[process->processId] != process || lifetime->closed) return;
		if (selectedProcess == process) return;
		selectedProcess = process;
		SelectedProcessChanged();
		WindowsChanged();
	}

	void UiaListViewModel::ClearSelection()
	{
		if (captureRequest) captureRequest->Cancel();
		pendingTree = nullptr;
		pendingCapture = nullptr;
		preview->Publish(nullptr);
		++lifetime->generation;
		++lifetime->dialogSerial;
		if (propertyDialog) propertyDialog->Close();
		propertyDialog = nullptr;
		PropertyDialogChanged();
		selectedWindow = nullptr;
		selectedNode = nullptr;
		snapshot = nullptr;
		ReleaseTreeChildren(nodeRoot);
		nodes.Clear();
		nodeRoot = Ptr(new NodeViewModel);
		treeBusy = false;
		SelectedWindowChanged();
		SelectedNodeChanged();
		HasSelectionChanged();
		NodeRootChanged();
		IsBusyChanged();
		StatusChanged();
		SetActiveTab(0);
	}

	void UiaListViewModel::SelectWindow(Ptr<vm::IProcessNodeViewModel> value)
	{
		auto node = value.Cast<ProcessNodeViewModel>();
		if (!node || !selectedProcess || !selectedProcess->windows.Contains(node.Obj()) || !node->window.qualifies || lifetime->closed) return;
		ClearSelection();
		selectedWindow = node;
		SelectedWindowChanged();
		HasSelectionChanged();
		auto target = node->window.identity;
		auto gate = lifetime;
		QueueNative(L"Create UIA session", [this, target, gate]()
		{
			if (!gate->closed) worker.session = Ptr(new native::UiaSession(target));
		});
		RefreshWindow();
		SetActiveTab(1);
	}

	void UiaListViewModel::RefreshWindow()
	{
		RefreshWindowInternal(true);
	}

	void UiaListViewModel::RefreshWindowInternal(bool closeDialog)
	{
		if (!selectedWindow || lifetime->closed) return;
		auto generation = ++lifetime->generation;
		auto gate = lifetime;
		if (closeDialog)
		{
			if (propertyDialog) propertyDialog->Close();
			propertyDialog = nullptr;
			PropertyDialogChanged();
		}
		else if (propertyDialog)
		{
			propertyDialog->generation = generation;
			for (auto&& reference : propertyDialog->references)
				reference.Cast<ReferenceViewModel>()->generation = generation;
		}
		treeBusy = true;
		loadedNodes = 0;
		if (propertyDialog)
		{
			propertyDialog->busy = true;
			propertyDialog->textRanges.Clear();
			propertyDialog->NotifyAvailability();
		}
		pendingTree = nullptr;
		pendingCapture = nullptr;
		if (captureRequest) captureRequest->Cancel();
		captureRequest = Ptr(new native::CaptureRequest(selectedWindow->window.identity, generation));
		auto request = captureRequest;
		preview->Leave();
		preview->IsAvailableChanged();
		preview->StatusChanged();
		IsBusyChanged();
		StatusChanged();
		QueueNative(L"Read Raw View", [this, gate, generation, closeDialog]()
		{
			auto canceled = [gate, generation]() { return gate->closed || gate->generation != generation; };
			if (canceled()) return;
			if (closeDialog) worker.session->DiscardRanges();
			auto result = worker.session->ReadTree(generation, canceled, [gate, generation](vint count)
			{
				Post(gate, [generation, count](UiaListViewModel& root)
				{
					if (root.lifetime->generation != generation || !root.treeBusy) return;
					root.loadedNodes = count; root.StatusChanged();
				});
			});
			if (result) Post(gate, [result](UiaListViewModel& root)
			{
				if (root.lifetime->generation != result->generation) return;
				root.pendingTree = result;
				root.PublishSnapshot();
			});
		});
		captureWorker.queue.QueueTask([gate, request]()
		{
			try
			{
				if (gate->closed || gate->generation != request->generation) return;
				auto result = native::CaptureWindow(request);
				Post(gate, [request, result](UiaListViewModel& root)
				{
					if (root.lifetime->generation != request->generation) return;
					root.pendingCapture = result;
					root.PublishSnapshot();
				});
			}
			catch (const Exception& error)
			{
				auto message = L"Window capture; HWND=" + native::Hex(reinterpret_cast<UINT_PTR>(request->target.handle)) + L"; PID=" + utow(request->target.processId) + L"; generation=" + itow(request->generation) + L"\r\n" + error.Message();
				GetApplication()->InvokeInMainThread(nullptr, [message]()
				{
					OutputDebugStringW(message.Buffer());
					MessageBoxW(nullptr, message.Buffer(), L"UiaList", MB_OK | MB_ICONERROR);
					ExitProcess(1);
				});
			}
		});
	}

	void UiaListViewModel::PublishSnapshot()
	{
		if (!pendingTree || !pendingCapture) return;
		PublishTree(pendingTree);
		preview->Publish(pendingCapture);
		pendingTree = nullptr;
		pendingCapture = nullptr;
	}

	void UiaListViewModel::PublishTree(Ptr<native::TreeSnapshot> result)
	{
		if (lifetime->generation != result->generation) return;
		Dictionary<vint, Ptr<NodeViewModel>> newNodes;
		auto synthetic = Ptr(new NodeViewModel);
		auto selectedKey = selectedNode ? selectedNode->data.key : 0;
		for (auto&& data : result->nodes)
		{
			auto node = Ptr(new NodeViewModel);
			node->data = data;
			node->generation = result->generation;
			node->displayText = (data.name.Length() ? SingleLine(data.name) + L" " : WString()) + L"(" + data.client + (data.providers.Length() ? L", " + data.providers : WString()) + L")";
			node->expanded = nodes.Keys().Contains(data.key) ? nodes[data.key]->expanded : data.parent == 0;
			newNodes.Add(data.key, node);
		}
		for (auto&& data : result->nodes)
		{
			auto parent = data.parent ? newNodes[data.parent] : synthetic;
			parent->children.Add(newNodes[data.key]);
		}
		while (selectedKey && !newNodes.Keys().Contains(selectedKey)) selectedKey = nodes.Keys().Contains(selectedKey) ? nodes[selectedKey]->data.parent : 0;
		if (propertyDialog && propertyDialog->open && nodes.Keys().Contains(propertyDialog->nodeKey) && !newNodes.Keys().Contains(propertyDialog->nodeKey)) propertyDialog->Close();
		ReleaseTreeChildren(nodeRoot);
		CopyFrom(nodes, newNodes);
		nodeRoot = synthetic;
		snapshot = result;
		selectedNode = nullptr;
		NodeRootChanged();
		if (selectedKey) SelectNode(nodes[selectedKey]);
		else if (result->nodes.Count()) SelectNode(nodes[result->nodes[0].key]);
		treeBusy = false;
		if (propertyDialog && propertyDialog->open)
		{
			propertyDialog->busy = false;
			propertyDialog->NotifyAvailability();
		}
		IsBusyChanged();
		StatusChanged();
	}

	void UiaListViewModel::SelectNode(Ptr<vm::INodeViewModel> value)
	{
		auto node = value.Cast<NodeViewModel>();
		if (!node || node->GetIsSyntheticRoot() || node->generation != lifetime->generation) return;
		if (node == selectedNode) return;
		if (selectedNode) { selectedNode->selected = false; selectedNode->IsSelectedChanged(); }
		selectedNode = node;
		node->selected = true;
		node->IsSelectedChanged();
		SelectedNodeChanged();
	}

	void UiaListViewModel::InspectNode(Ptr<vm::INodeViewModel> value)
	{
		auto node = value.Cast<NodeViewModel>();
		if (!node || node->GetIsSyntheticRoot() || node->generation != lifetime->generation || treeBusy || lifetime->closed) return;
		if (!nodes.Keys().Contains(node->data.key) || nodes[node->data.key] != node) return;
		if (propertyDialog && propertyDialog->open) return;
		SelectNode(node);
		propertyDialog = Ptr(new PropertyDialogViewModel(*this, node->data.key, node->displayText));
		PropertyDialogChanged();
		propertyDialog->Refresh();
	}

	void UiaListViewModel::RequestClose()
	{
		if (lifetime->closed.exchange(true)) return;
		if (captureRequest) captureRequest->Cancel();
		lifetime->owner = nullptr;
		++lifetime->generation;
		++lifetime->dialogSerial;
		if (propertyDialog) propertyDialog->Close();
		// Detach while observers are alive, before the window destroys its bound
		// tree. This also releases their cached nodes without recursive teardown.
		ReleaseTreeChildren(nodeRoot);
		ReleaseTreeChildren(processRoot);
		if (initialized)
		{
			worker.queue.QueueTask([this]() { worker.session = nullptr; });
			worker.queue.QueueExitTask();
			captureWorker.queue.QueueExitTask();
			worker.Wait();
			captureWorker.Wait();
		}
	}
}
