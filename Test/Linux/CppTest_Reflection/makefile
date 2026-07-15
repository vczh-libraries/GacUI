.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-I ../../../Import -I ../../GacUISrc/Generated_DarkSkin/Source_x64 -I ../../GacUISrc/Generated_FullControlTest/Source_x64
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi

all:pre-build ./Bin/UnitTest

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppGlrParser.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/VlppWorkflowLibrary.o ./Obj/VlppWorkflowCompiler.o ./Obj/VlppWorkflowRuntime.o ./Obj/GuiGraphicsDocumentElement.o ./Obj/GuiGraphicsDocumentRenderer_GuiDocumentImageCache.o ./Obj/GuiGraphicsDocumentRenderer_SetPropertiesVisitor.o ./Obj/GuiGraphicsDocumentRenderer_GuiDocumentParagraphCache.o ./Obj/GuiGraphicsDocumentRenderer_GuiDocumentElementRenderer.o ./Obj/GuiGraphicsElement.o ./Obj/GuiGraphicsElementInterfaces.o ./Obj/GuiGraphicsResourceManager.o ./Obj/GuiNativeWindow.o ./Obj/GacGenController.o ./Obj/GuiHostedApplication.o ./Obj/GuiHostedController.o ./Obj/GuiHostedGraphics.o ./Obj/GuiHostedWindow.o ./Obj/GuiHostedWindowProxy_Main.o ./Obj/GuiHostedWindowProxy_NonMain.o ./Obj/GuiHostedWindowProxy_Placeholder.o ./Obj/GuiRemoteRendererSingle.o ./Obj/GuiRemoteRendererSingle_Controller.o ./Obj/GuiRemoteRendererSingle_IO.o ./Obj/GuiRemoteRendererSingle_MainWindow.o ./Obj/GuiRemoteRendererSingle_Rendering.o ./Obj/GuiRemoteRendererSingle_Rendering_Document.o ./Obj/GuiRemoteRendererSingle_Rendering_Elements.o ./Obj/GuiRemoteRendererSingle_Rendering_Image.o ./Obj/GuiRemoteRendererSingle_Rendering_Label.o ./Obj/GuiRemoteController.o ./Obj/GuiRemoteControllerSetup.o ./Obj/GuiRemoteEvents.o ./Obj/GuiRemoteGraphics.o ./Obj/GuiRemoteGraphics_BasicElements.o ./Obj/GuiRemoteGraphics_Document.o ./Obj/GuiRemoteGraphics_Document_Helpers.o ./Obj/GuiRemoteGraphics_ImageService.o ./Obj/GuiRemoteProtocolSchemaShared.o ./Obj/GuiRemoteProtocol_Channel_Async.o ./Obj/GuiRemoteProtocol_Channel_AsyncRenderer.o ./Obj/GuiRemoteProtocol_Channel_Json.o ./Obj/GuiRemoteProtocol_DomDiff.o ./Obj/GuiRemoteProtocol_Filter.o ./Obj/GuiRemoteProtocol_FilterVerifier.o ./Obj/GuiRemoteWindow.o ./Obj/GuiRemoteProtocolSchema_BuildFrame.o ./Obj/GuiRemoteProtocolSchema_CopyDom.o ./Obj/GuiRemoteProtocolSchema_DiffFrame.o ./Obj/GuiRemoteProtocolSchema.o ./Obj/GuiDocument.o ./Obj/GuiDocumentClipboard_Document.o ./Obj/GuiDocumentClipboard_HtmlFormat.o ./Obj/GuiDocumentClipboard_RichTextFormat.o ./Obj/GuiDocumentEditor_AddContainer.o ./Obj/GuiDocumentEditor_ClearUnnecessaryRun.o ./Obj/GuiDocumentEditor_CloneRun.o ./Obj/GuiDocumentEditor_CollectStyle.o ./Obj/GuiDocumentEditor_CutRun.o ./Obj/GuiDocumentEditor_GetRunRange.o ./Obj/GuiDocumentEditor_LocateHyperlink.o ./Obj/GuiDocumentEditor_LocateStyle.o ./Obj/GuiDocumentEditor_RemoveContainer.o ./Obj/GuiDocumentEditor_RemoveRun.o ./Obj/GuiDocumentEditor_ReplaceStyleName.o ./Obj/GuiDocumentEditor_SummerizeStyle.o ./Obj/GuiDocument_Edit.o ./Obj/GuiDocument_Load.o ./Obj/GuiDocument_Save.o ./Obj/GuiParserManager.o ./Obj/GuiPluginManager.o ./Obj/GuiResource.o ./Obj/GuiResourceManager.o ./Obj/GuiResourceTypeResolvers.o ./Obj/GuiSharedAsyncService.o ./Obj/GuiSharedAutomationService.o ./Obj/GuiSharedCallbackService.o ./Obj/GuiApplication.o ./Obj/GuiBasicControls.o ./Obj/GuiInstanceRootObject.o ./Obj/GuiLabelControls.o ./Obj/GuiThemeManager.o ./Obj/GuiWindowControls.o ./Obj/GuiGraphicsBoundsComposition.o ./Obj/GuiGraphicsComposition.o ./Obj/GuiGraphicsComposition_Helpers.o ./Obj/GuiGraphicsComposition_Layout.o ./Obj/GuiGraphicsEventReceiver.o ./Obj/GuiGraphicsWindowComposition.o ./Obj/GuiGraphicsHost.o ./Obj/GuiGraphicsHost_Alt.o ./Obj/GuiGraphicsHost_ShortcutKey.o ./Obj/GuiGraphicsHost_Tab.o ./Obj/GuiButtonControls.o ./Obj/GuiContainerControls.o ./Obj/GuiDateTimeControls.o ./Obj/GuiDialogs.o ./Obj/GuiScrollControls.o ./Obj/DataSourceImpl_IItemProvider_ItemProviderBase.o ./Obj/DataSourceImpl_IItemProvider_NodeItemProvider.o ./Obj/DataSourceImpl_INodeProvider_MemoryNodeProvider.o ./Obj/GuiBindableDataGrid.o ./Obj/GuiBindableListControls.o ./Obj/GuiComboControls.o ./Obj/GuiDataGridControls.o ./Obj/GuiDataGridExtensions.o ./Obj/GuiListControlItemArrangers.o ./Obj/GuiListControls.o ./Obj/GuiListViewControls.o ./Obj/ItemProvider_Binding.o ./Obj/ItemTemplate_IListViewItemView.o ./Obj/GuiTextListControls.o ./Obj/GuiTreeViewControls.o ./Obj/ItemProvider_IListViewItemView.o ./Obj/ItemProvider_ITextItemView.o ./Obj/ItemProvider_ITreeViewItemView.o ./Obj/ItemTemplate_ITextItemView.o ./Obj/ItemTemplate_ITreeViewItemView.o ./Obj/GuiAnimation.o ./Obj/GuiCommonTemplates.o ./Obj/GuiControlTemplates.o ./Obj/GuiThemeStyleFactory.o ./Obj/GuiTextUndoRedo.o ./Obj/GuiDocumentCommonInterface.o ./Obj/GuiDocumentConfig.o ./Obj/GuiDocumentViewer.o ./Obj/GuiMenuControls.o ./Obj/GuiRibbonControls.o ./Obj/GuiRibbonGalleryList.o ./Obj/GuiRibbonImpl.o ./Obj/GuiToolstripCommand.o ./Obj/GuiToolstripMenu.o ./Obj/GacUIReflectionHelper.o ./Obj/GuiGraphicsAxis.o ./Obj/GuiGraphicsFlowComposition.o ./Obj/GuiGraphicsRepeatComposition.o ./Obj/GuiGraphicsRepeatComposition_NonVirtial.o ./Obj/GuiGraphicsRepeatComposition_Virtual.o ./Obj/GuiGraphicsResponsiveComposition.o ./Obj/GuiGraphicsSharedSizeComposition.o ./Obj/GuiGraphicsSpecializedComposition.o ./Obj/GuiGraphicsStackComposition.o ./Obj/GuiGraphicsTableComposition.o ./Obj/GuiInstanceCompiledWorkflow.o ./Obj/GuiReflectionBasic.o ./Obj/GuiReflectionCompositions.o ./Obj/GuiReflectionControls.o ./Obj/GuiReflectionElements.o ./Obj/GuiReflectionEvents.o ./Obj/GuiReflectionPlugin.o ./Obj/GuiReflectionTemplates.o ./Obj/GuiFakeClipboardService.o ./Obj/GuiFakeDialogServiceBase.o ./Obj/GuiFakeDialogServiceBase_ColorDialog.o ./Obj/GuiFakeDialogServiceBase_FileDialog.o ./Obj/GuiFakeDialogServiceBase_FontDialog.o ./Obj/GuiFakeDialogServiceBase_MessageBox.o ./Obj/GuiFakeDialogService.o ./Obj/GuiFakeDialogServiceUI.o ./Obj/GuiFakeDialogServiceUIResource.o ./Obj/GuiUtilitiesRegistration.o ./Obj/GuiSharedAutomationService_Controls.o ./Obj/GuiFakeDialogServiceUIReflection.o ./Obj/DarkSkin.o ./Obj/DarkSkinReflection.o ./Obj/DarkSkinResource.o ./Obj/DocumentEditorBase.o ./Obj/FullControlTest.o ./Obj/FullControlTestReflection.o ./Obj/FullControlTestResource.o ./Obj/MainWindow.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppGlrParser.o: ../../../Import/VlppGlrParser.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../../Import/VlppReflection.cpp
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../../Import/VlppRegex.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowLibrary.o: ../../../Import/VlppWorkflowLibrary.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowCompiler.o: ../../../Import/VlppWorkflowCompiler.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowRuntime.o: ../../../Import/VlppWorkflowRuntime.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsDocumentElement.o: ../../../Source/GraphicsElement/GuiGraphicsDocumentElement.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsDocumentRenderer_GuiDocumentImageCache.o: ../../../Source/GraphicsElement/GuiGraphicsDocumentRenderer_GuiDocumentImageCache.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsDocumentRenderer_SetPropertiesVisitor.o: ../../../Source/GraphicsElement/GuiGraphicsDocumentRenderer_SetPropertiesVisitor.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsDocumentRenderer_GuiDocumentParagraphCache.o: ../../../Source/GraphicsElement/GuiGraphicsDocumentRenderer_GuiDocumentParagraphCache.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsDocumentRenderer_GuiDocumentElementRenderer.o: ../../../Source/GraphicsElement/GuiGraphicsDocumentRenderer_GuiDocumentElementRenderer.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsElement.o: ../../../Source/GraphicsElement/GuiGraphicsElement.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsElementInterfaces.o: ../../../Source/GraphicsElement/GuiGraphicsElementInterfaces.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsResourceManager.o: ../../../Source/GraphicsElement/GuiGraphicsResourceManager.cpp
	$(CPP_COMPILE)

./Obj/GuiNativeWindow.o: ../../../Source/NativeWindow/GuiNativeWindow.cpp
	$(CPP_COMPILE)

./Obj/GacGenController.o: ../../../Source/PlatformProviders/GacGen/GacGenController.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedApplication.o: ../../../Source/PlatformProviders/Hosted/GuiHostedApplication.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedController.o: ../../../Source/PlatformProviders/Hosted/GuiHostedController.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedGraphics.o: ../../../Source/PlatformProviders/Hosted/GuiHostedGraphics.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedWindow.o: ../../../Source/PlatformProviders/Hosted/GuiHostedWindow.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedWindowProxy_Main.o: ../../../Source/PlatformProviders/Hosted/GuiHostedWindowProxy_Main.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedWindowProxy_NonMain.o: ../../../Source/PlatformProviders/Hosted/GuiHostedWindowProxy_NonMain.cpp
	$(CPP_COMPILE)

./Obj/GuiHostedWindowProxy_Placeholder.o: ../../../Source/PlatformProviders/Hosted/GuiHostedWindowProxy_Placeholder.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_Controller.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Controller.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_IO.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_IO.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_MainWindow.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_MainWindow.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_Rendering.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_Rendering_Document.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Document.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_Rendering_Elements.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Elements.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_Rendering_Image.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Image.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteRendererSingle_Rendering_Label.o: ../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle_Rendering_Label.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteController.o: ../../../Source/PlatformProviders/Remote/GuiRemoteController.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteControllerSetup.o: ../../../Source/PlatformProviders/Remote/GuiRemoteControllerSetup.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteEvents.o: ../../../Source/PlatformProviders/Remote/GuiRemoteEvents.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteGraphics.o: ../../../Source/PlatformProviders/Remote/GuiRemoteGraphics.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteGraphics_BasicElements.o: ../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_BasicElements.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteGraphics_Document.o: ../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteGraphics_Document_Helpers.o: ../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_Document_Helpers.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteGraphics_ImageService.o: ../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_ImageService.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocolSchemaShared.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocolSchemaShared.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocol_Channel_Async.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_Channel_Async.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocol_Channel_AsyncRenderer.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_Channel_AsyncRenderer.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocol_Channel_Json.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_Channel_Json.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocol_DomDiff.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_DomDiff.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocol_Filter.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_Filter.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocol_FilterVerifier.o: ../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_FilterVerifier.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteWindow.o: ../../../Source/PlatformProviders/Remote/GuiRemoteWindow.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocolSchema_BuildFrame.o: ../../../Source/PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_BuildFrame.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocolSchema_CopyDom.o: ../../../Source/PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_CopyDom.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocolSchema_DiffFrame.o: ../../../Source/PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_DiffFrame.cpp
	$(CPP_COMPILE)

./Obj/GuiRemoteProtocolSchema.o: ../../../Source/PlatformProviders/Remote/Protocol/Generated/GuiRemoteProtocolSchema.cpp
	$(CPP_COMPILE)

./Obj/GuiDocument.o: ../../../Source/Resources/GuiDocument.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentClipboard_Document.o: ../../../Source/Resources/GuiDocumentClipboard_Document.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentClipboard_HtmlFormat.o: ../../../Source/Resources/GuiDocumentClipboard_HtmlFormat.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentClipboard_RichTextFormat.o: ../../../Source/Resources/GuiDocumentClipboard_RichTextFormat.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_AddContainer.o: ../../../Source/Resources/GuiDocumentEditor_AddContainer.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_ClearUnnecessaryRun.o: ../../../Source/Resources/GuiDocumentEditor_ClearUnnecessaryRun.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_CloneRun.o: ../../../Source/Resources/GuiDocumentEditor_CloneRun.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_CollectStyle.o: ../../../Source/Resources/GuiDocumentEditor_CollectStyle.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_CutRun.o: ../../../Source/Resources/GuiDocumentEditor_CutRun.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_GetRunRange.o: ../../../Source/Resources/GuiDocumentEditor_GetRunRange.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_LocateHyperlink.o: ../../../Source/Resources/GuiDocumentEditor_LocateHyperlink.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_LocateStyle.o: ../../../Source/Resources/GuiDocumentEditor_LocateStyle.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_RemoveContainer.o: ../../../Source/Resources/GuiDocumentEditor_RemoveContainer.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_RemoveRun.o: ../../../Source/Resources/GuiDocumentEditor_RemoveRun.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_ReplaceStyleName.o: ../../../Source/Resources/GuiDocumentEditor_ReplaceStyleName.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentEditor_SummerizeStyle.o: ../../../Source/Resources/GuiDocumentEditor_SummerizeStyle.cpp
	$(CPP_COMPILE)

./Obj/GuiDocument_Edit.o: ../../../Source/Resources/GuiDocument_Edit.cpp
	$(CPP_COMPILE)

./Obj/GuiDocument_Load.o: ../../../Source/Resources/GuiDocument_Load.cpp
	$(CPP_COMPILE)

./Obj/GuiDocument_Save.o: ../../../Source/Resources/GuiDocument_Save.cpp
	$(CPP_COMPILE)

./Obj/GuiParserManager.o: ../../../Source/Resources/GuiParserManager.cpp
	$(CPP_COMPILE)

./Obj/GuiPluginManager.o: ../../../Source/Resources/GuiPluginManager.cpp
	$(CPP_COMPILE)

./Obj/GuiResource.o: ../../../Source/Resources/GuiResource.cpp
	$(CPP_COMPILE)

./Obj/GuiResourceManager.o: ../../../Source/Resources/GuiResourceManager.cpp
	$(CPP_COMPILE)

./Obj/GuiResourceTypeResolvers.o: ../../../Source/Resources/GuiResourceTypeResolvers.cpp
	$(CPP_COMPILE)

./Obj/GuiSharedAsyncService.o: ../../../Source/Utilities/SharedServices/GuiSharedAsyncService.cpp
	$(CPP_COMPILE)

./Obj/GuiSharedAutomationService.o: ../../../Source/Utilities/SharedServices/GuiSharedAutomationService.cpp
	$(CPP_COMPILE)

./Obj/GuiSharedCallbackService.o: ../../../Source/Utilities/SharedServices/GuiSharedCallbackService.cpp
	$(CPP_COMPILE)

./Obj/GuiApplication.o: ../../../Source/Application/Controls/GuiApplication.cpp
	$(CPP_COMPILE)

./Obj/GuiBasicControls.o: ../../../Source/Application/Controls/GuiBasicControls.cpp
	$(CPP_COMPILE)

./Obj/GuiInstanceRootObject.o: ../../../Source/Application/Controls/GuiInstanceRootObject.cpp
	$(CPP_COMPILE)

./Obj/GuiLabelControls.o: ../../../Source/Application/Controls/GuiLabelControls.cpp
	$(CPP_COMPILE)

./Obj/GuiThemeManager.o: ../../../Source/Application/Controls/GuiThemeManager.cpp
	$(CPP_COMPILE)

./Obj/GuiWindowControls.o: ../../../Source/Application/Controls/GuiWindowControls.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsBoundsComposition.o: ../../../Source/Application/GraphicsCompositions/GuiGraphicsBoundsComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsComposition.o: ../../../Source/Application/GraphicsCompositions/GuiGraphicsComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsComposition_Helpers.o: ../../../Source/Application/GraphicsCompositions/GuiGraphicsComposition_Helpers.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsComposition_Layout.o: ../../../Source/Application/GraphicsCompositions/GuiGraphicsComposition_Layout.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsEventReceiver.o: ../../../Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsWindowComposition.o: ../../../Source/Application/GraphicsCompositions/GuiGraphicsWindowComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsHost.o: ../../../Source/Application/GraphicsHost/GuiGraphicsHost.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsHost_Alt.o: ../../../Source/Application/GraphicsHost/GuiGraphicsHost_Alt.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsHost_ShortcutKey.o: ../../../Source/Application/GraphicsHost/GuiGraphicsHost_ShortcutKey.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsHost_Tab.o: ../../../Source/Application/GraphicsHost/GuiGraphicsHost_Tab.cpp
	$(CPP_COMPILE)

./Obj/GuiButtonControls.o: ../../../Source/Controls/GuiButtonControls.cpp
	$(CPP_COMPILE)

./Obj/GuiContainerControls.o: ../../../Source/Controls/GuiContainerControls.cpp
	$(CPP_COMPILE)

./Obj/GuiDateTimeControls.o: ../../../Source/Controls/GuiDateTimeControls.cpp
	$(CPP_COMPILE)

./Obj/GuiDialogs.o: ../../../Source/Controls/GuiDialogs.cpp
	$(CPP_COMPILE)

./Obj/GuiScrollControls.o: ../../../Source/Controls/GuiScrollControls.cpp
	$(CPP_COMPILE)

./Obj/DataSourceImpl_IItemProvider_ItemProviderBase.o: ../../../Source/Controls/ListControlPackage/DataSourceImpl_IItemProvider_ItemProviderBase.cpp
	$(CPP_COMPILE)

./Obj/DataSourceImpl_IItemProvider_NodeItemProvider.o: ../../../Source/Controls/ListControlPackage/DataSourceImpl_IItemProvider_NodeItemProvider.cpp
	$(CPP_COMPILE)

./Obj/DataSourceImpl_INodeProvider_MemoryNodeProvider.o: ../../../Source/Controls/ListControlPackage/DataSourceImpl_INodeProvider_MemoryNodeProvider.cpp
	$(CPP_COMPILE)

./Obj/GuiBindableDataGrid.o: ../../../Source/Controls/ListControlPackage/GuiBindableDataGrid.cpp
	$(CPP_COMPILE)

./Obj/GuiBindableListControls.o: ../../../Source/Controls/ListControlPackage/GuiBindableListControls.cpp
	$(CPP_COMPILE)

./Obj/GuiComboControls.o: ../../../Source/Controls/ListControlPackage/GuiComboControls.cpp
	$(CPP_COMPILE)

./Obj/GuiDataGridControls.o: ../../../Source/Controls/ListControlPackage/GuiDataGridControls.cpp
	$(CPP_COMPILE)

./Obj/GuiDataGridExtensions.o: ../../../Source/Controls/ListControlPackage/GuiDataGridExtensions.cpp
	$(CPP_COMPILE)

./Obj/GuiListControlItemArrangers.o: ../../../Source/Controls/ListControlPackage/GuiListControlItemArrangers.cpp
	$(CPP_COMPILE)

./Obj/GuiListControls.o: ../../../Source/Controls/ListControlPackage/GuiListControls.cpp
	$(CPP_COMPILE)

./Obj/GuiListViewControls.o: ../../../Source/Controls/ListControlPackage/GuiListViewControls.cpp
	$(CPP_COMPILE)

./Obj/ItemProvider_Binding.o: ../../../Source/Controls/ListControlPackage/ItemProvider_Binding.cpp
	$(CPP_COMPILE)

./Obj/ItemTemplate_IListViewItemView.o: ../../../Source/Controls/ListControlPackage/ItemTemplate_IListViewItemView.cpp
	$(CPP_COMPILE)

./Obj/GuiTextListControls.o: ../../../Source/Controls/ListControlPackage/GuiTextListControls.cpp
	$(CPP_COMPILE)

./Obj/GuiTreeViewControls.o: ../../../Source/Controls/ListControlPackage/GuiTreeViewControls.cpp
	$(CPP_COMPILE)

./Obj/ItemProvider_IListViewItemView.o: ../../../Source/Controls/ListControlPackage/ItemProvider_IListViewItemView.cpp
	$(CPP_COMPILE)

./Obj/ItemProvider_ITextItemView.o: ../../../Source/Controls/ListControlPackage/ItemProvider_ITextItemView.cpp
	$(CPP_COMPILE)

./Obj/ItemProvider_ITreeViewItemView.o: ../../../Source/Controls/ListControlPackage/ItemProvider_ITreeViewItemView.cpp
	$(CPP_COMPILE)

./Obj/ItemTemplate_ITextItemView.o: ../../../Source/Controls/ListControlPackage/ItemTemplate_ITextItemView.cpp
	$(CPP_COMPILE)

./Obj/ItemTemplate_ITreeViewItemView.o: ../../../Source/Controls/ListControlPackage/ItemTemplate_ITreeViewItemView.cpp
	$(CPP_COMPILE)

./Obj/GuiAnimation.o: ../../../Source/Controls/Templates/GuiAnimation.cpp
	$(CPP_COMPILE)

./Obj/GuiCommonTemplates.o: ../../../Source/Controls/Templates/GuiCommonTemplates.cpp
	$(CPP_COMPILE)

./Obj/GuiControlTemplates.o: ../../../Source/Controls/Templates/GuiControlTemplates.cpp
	$(CPP_COMPILE)

./Obj/GuiThemeStyleFactory.o: ../../../Source/Controls/Templates/GuiThemeStyleFactory.cpp
	$(CPP_COMPILE)

./Obj/GuiTextUndoRedo.o: ../../../Source/Controls/TextEditorPackage/EditorCallback/GuiTextUndoRedo.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentCommonInterface.o: ../../../Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentConfig.o: ../../../Source/Controls/TextEditorPackage/GuiDocumentConfig.cpp
	$(CPP_COMPILE)

./Obj/GuiDocumentViewer.o: ../../../Source/Controls/TextEditorPackage/GuiDocumentViewer.cpp
	$(CPP_COMPILE)

./Obj/GuiMenuControls.o: ../../../Source/Controls/ToolstripPackage/GuiMenuControls.cpp
	$(CPP_COMPILE)

./Obj/GuiRibbonControls.o: ../../../Source/Controls/ToolstripPackage/GuiRibbonControls.cpp
	$(CPP_COMPILE)

./Obj/GuiRibbonGalleryList.o: ../../../Source/Controls/ToolstripPackage/GuiRibbonGalleryList.cpp
	$(CPP_COMPILE)

./Obj/GuiRibbonImpl.o: ../../../Source/Controls/ToolstripPackage/GuiRibbonImpl.cpp
	$(CPP_COMPILE)

./Obj/GuiToolstripCommand.o: ../../../Source/Controls/ToolstripPackage/GuiToolstripCommand.cpp
	$(CPP_COMPILE)

./Obj/GuiToolstripMenu.o: ../../../Source/Controls/ToolstripPackage/GuiToolstripMenu.cpp
	$(CPP_COMPILE)

./Obj/GacUIReflectionHelper.o: ../../../Source/GacUIReflectionHelper.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsAxis.o: ../../../Source/GraphicsComposition/GuiGraphicsAxis.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsFlowComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsFlowComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsRepeatComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsRepeatComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsRepeatComposition_NonVirtial.o: ../../../Source/GraphicsComposition/GuiGraphicsRepeatComposition_NonVirtial.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsRepeatComposition_Virtual.o: ../../../Source/GraphicsComposition/GuiGraphicsRepeatComposition_Virtual.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsResponsiveComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsResponsiveComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsSharedSizeComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsSharedSizeComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsSpecializedComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsSpecializedComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsStackComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsStackComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiGraphicsTableComposition.o: ../../../Source/GraphicsComposition/GuiGraphicsTableComposition.cpp
	$(CPP_COMPILE)

./Obj/GuiInstanceCompiledWorkflow.o: ../../../Source/Reflection/GuiInstanceCompiledWorkflow.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionBasic.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionCompositions.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionCompositions.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionControls.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionElements.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionElements.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionEvents.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionPlugin.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.cpp
	$(CPP_COMPILE)

./Obj/GuiReflectionTemplates.o: ../../../Source/Reflection/TypeDescriptors/GuiReflectionTemplates.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeClipboardService.o: ../../../Source/Utilities/FakeServices/GuiFakeClipboardService.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceBase.o: ../../../Source/Utilities/FakeServices/GuiFakeDialogServiceBase.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceBase_ColorDialog.o: ../../../Source/Utilities/FakeServices/GuiFakeDialogServiceBase_ColorDialog.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceBase_FileDialog.o: ../../../Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FileDialog.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceBase_FontDialog.o: ../../../Source/Utilities/FakeServices/GuiFakeDialogServiceBase_FontDialog.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceBase_MessageBox.o: ../../../Source/Utilities/FakeServices/GuiFakeDialogServiceBase_MessageBox.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogService.o: ../../../Source/Utilities/FakeServices/Dialogs/GuiFakeDialogService.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceUI.o: ../../../Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUI.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceUIResource.o: ../../../Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUIResource.cpp
	$(CPP_COMPILE)

./Obj/GuiUtilitiesRegistration.o: ../../../Source/Utilities/GuiUtilitiesRegistration.cpp
	$(CPP_COMPILE)

./Obj/GuiSharedAutomationService_Controls.o: ../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.cpp
	$(CPP_COMPILE)

./Obj/GuiFakeDialogServiceUIReflection.o: ../../../Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUIReflection.cpp
	$(CPP_COMPILE)

./Obj/DarkSkin.o: ../../GacUISrc/Generated_DarkSkin/Source_x64/DarkSkin.cpp
	$(CPP_COMPILE)

./Obj/DarkSkinReflection.o: ../../GacUISrc/Generated_DarkSkin/Source_x64/DarkSkinReflection.cpp
	$(CPP_COMPILE)

./Obj/DarkSkinResource.o: ../../GacUISrc/Generated_DarkSkin/Source_x64/DarkSkinResource.cpp
	$(CPP_COMPILE)

./Obj/DocumentEditorBase.o: ../../GacUISrc/Generated_FullControlTest/Source_x64/DocumentEditorBase.cpp
	$(CPP_COMPILE)

./Obj/FullControlTest.o: ../../GacUISrc/Generated_FullControlTest/Source_x64/FullControlTest.cpp
	$(CPP_COMPILE)

./Obj/FullControlTestReflection.o: ../../GacUISrc/Generated_FullControlTest/Source_x64/FullControlTestReflection.cpp
	$(CPP_COMPILE)

./Obj/FullControlTestResource.o: ../../GacUISrc/Generated_FullControlTest/Source_x64/FullControlTestResource.cpp
	$(CPP_COMPILE)

./Obj/MainWindow.o: ../../GacUISrc/Generated_FullControlTest/Source_x64/MainWindow.cpp
	$(CPP_COMPILE)

./Obj/Main.o: ../Main.cpp
	$(CPP_COMPILE)
