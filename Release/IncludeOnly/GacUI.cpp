﻿/***********************************************************************
THIS FILE IS AUTOMATICALLY GENERATED. DO NOT MODIFY
DEVELOPER: Zihan Chen(vczh)
***********************************************************************/
#include "GacUI.h"
#ifndef VCZH_DEBUG_NO_REFLECTION
#include "GacUIReflection.h"
#endif

#include "..\..\Source\GacUIReflectionHelper.cpp"
#include "..\..\Source\Application\Controls\GuiApplication.cpp"
#include "..\..\Source\Application\Controls\GuiBasicControls.cpp"
#include "..\..\Source\Application\Controls\GuiInstanceRootObject.cpp"
#include "..\..\Source\Application\Controls\GuiLabelControls.cpp"
#include "..\..\Source\Application\Controls\GuiThemeManager.cpp"
#include "..\..\Source\Application\Controls\GuiWindowControls.cpp"
#include "..\..\Source\Application\GraphicsCompositions\GuiGraphicsBoundsComposition.cpp"
#include "..\..\Source\Application\GraphicsCompositions\GuiGraphicsComposition.cpp"
#include "..\..\Source\Application\GraphicsCompositions\GuiGraphicsComposition_Helpers.cpp"
#include "..\..\Source\Application\GraphicsCompositions\GuiGraphicsComposition_Layout.cpp"
#include "..\..\Source\Application\GraphicsCompositions\GuiGraphicsEventReceiver.cpp"
#include "..\..\Source\Application\GraphicsCompositions\GuiGraphicsWindowComposition.cpp"
#include "..\..\Source\Application\GraphicsHost\GuiGraphicsHost.cpp"
#include "..\..\Source\Application\GraphicsHost\GuiGraphicsHost_Alt.cpp"
#include "..\..\Source\Application\GraphicsHost\GuiGraphicsHost_ShortcutKey.cpp"
#include "..\..\Source\Application\GraphicsHost\GuiGraphicsHost_Tab.cpp"
#include "..\..\Source\Controls\GuiButtonControls.cpp"
#include "..\..\Source\Controls\GuiContainerControls.cpp"
#include "..\..\Source\Controls\GuiDateTimeControls.cpp"
#include "..\..\Source\Controls\GuiDialogs.cpp"
#include "..\..\Source\Controls\GuiScrollControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_ItemProviderBase.cpp"
#include "..\..\Source\Controls\ListControlPackage\DataSourceImpl_IItemProvider_NodeItemProvider.cpp"
#include "..\..\Source\Controls\ListControlPackage\DataSourceImpl_INodeProvider_MemoryNodeProvider.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiBindableDataGrid.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiBindableListControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiComboControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiDataGridControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiDataGridExtensions.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiListControlItemArrangers.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiListControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiListViewControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiListViewItemTemplates.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiTextListControls.cpp"
#include "..\..\Source\Controls\ListControlPackage\GuiTreeViewControls.cpp"
#include "..\..\Source\Controls\Templates\GuiAnimation.cpp"
#include "..\..\Source\Controls\Templates\GuiCommonTemplates.cpp"
#include "..\..\Source\Controls\Templates\GuiControlTemplates.cpp"
#include "..\..\Source\Controls\Templates\GuiThemeStyleFactory.cpp"
#include "..\..\Source\Controls\TextEditorPackage\GuiDocumentViewer.cpp"
#include "..\..\Source\Controls\TextEditorPackage\GuiTextCommonInterface.cpp"
#include "..\..\Source\Controls\TextEditorPackage\GuiTextControls.cpp"
#include "..\..\Source\Controls\TextEditorPackage\EditorCallback\GuiTextAutoComplete.cpp"
#include "..\..\Source\Controls\TextEditorPackage\EditorCallback\GuiTextColorizer.cpp"
#include "..\..\Source\Controls\TextEditorPackage\EditorCallback\GuiTextUndoRedo.cpp"
#include "..\..\Source\Controls\TextEditorPackage\LanguageService\GuiLanguageAutoComplete.cpp"
#include "..\..\Source\Controls\TextEditorPackage\LanguageService\GuiLanguageColorizer.cpp"
#include "..\..\Source\Controls\TextEditorPackage\LanguageService\GuiLanguageOperations.cpp"
#include "..\..\Source\Controls\ToolstripPackage\GuiMenuControls.cpp"
#include "..\..\Source\Controls\ToolstripPackage\GuiRibbonControls.cpp"
#include "..\..\Source\Controls\ToolstripPackage\GuiRibbonGalleryList.cpp"
#include "..\..\Source\Controls\ToolstripPackage\GuiRibbonImpl.cpp"
#include "..\..\Source\Controls\ToolstripPackage\GuiToolstripCommand.cpp"
#include "..\..\Source\Controls\ToolstripPackage\GuiToolstripMenu.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsAxis.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsFlowComposition.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsRepeatComposition.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsRepeatComposition_NonVirtial.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsRepeatComposition_Virtual.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsResponsiveComposition.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsSharedSizeComposition.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsSpecializedComposition.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsStackComposition.cpp"
#include "..\..\Source\GraphicsComposition\GuiGraphicsTableComposition.cpp"
#include "..\..\Source\GraphicsElement\GuiGraphicsDocumentElement.cpp"
#include "..\..\Source\GraphicsElement\GuiGraphicsElement.cpp"
#include "..\..\Source\GraphicsElement\GuiGraphicsElementInterfaces.cpp"
#include "..\..\Source\GraphicsElement\GuiGraphicsResourceManager.cpp"
#include "..\..\Source\GraphicsElement\GuiGraphicsTextElement.cpp"
#include "..\..\Source\NativeWindow\GuiNativeWindow.cpp"
#include "..\..\Source\PlatformProviders\GacGen\GacGenController.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedApplication.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedController.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedGraphics.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedWindow.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedWindowProxy_Main.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedWindowProxy_NonMain.cpp"
#include "..\..\Source\PlatformProviders\Hosted\GuiHostedWindowProxy_Placeholder.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteController.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteControllerSetup.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteEvents.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteGraphics.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteGraphics_BasicElements.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteGraphics_ImageService.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteProtocolSchemaShared.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteProtocol_Channel_Async.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteProtocol_Channel_Json.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteProtocol_DomDiff.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteProtocol_Filter.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteProtocol_FilterVerifier.cpp"
#include "..\..\Source\PlatformProviders\Remote\GuiRemoteWindow.cpp"
#include "..\..\Source\PlatformProviders\Remote\Protocol\FrameOperations\GuiRemoteProtocolSchema_BuildFrame.cpp"
#include "..\..\Source\PlatformProviders\Remote\Protocol\FrameOperations\GuiRemoteProtocolSchema_CopyDom.cpp"
#include "..\..\Source\PlatformProviders\Remote\Protocol\FrameOperations\GuiRemoteProtocolSchema_DiffFrame.cpp"
#include "..\..\Source\PlatformProviders\Remote\Protocol\Generated\GuiRemoteProtocolSchema.cpp"
#include "..\..\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle.cpp"
#include "..\..\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Controller.cpp"
#include "..\..\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_IO.cpp"
#include "..\..\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_MainWindow.cpp"
#include "..\..\Source\PlatformProviders\RemoteRenderer\GuiRemoteRendererSingle_Rendering.cpp"
#include "..\..\Source\Resources\GuiDocument.cpp"
#include "..\..\Source\Resources\GuiDocumentClipboard_Document.cpp"
#include "..\..\Source\Resources\GuiDocumentClipboard_HtmlFormat.cpp"
#include "..\..\Source\Resources\GuiDocumentClipboard_RichTextFormat.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_AddContainer.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_ClearUnnecessaryRun.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_CloneRun.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_CollectStyle.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_CutRun.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_GetRunRange.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_LocaleHyperlink.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_LocaleStyle.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_RemoveContainer.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_RemoveRun.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_ReplaceStyleName.cpp"
#include "..\..\Source\Resources\GuiDocumentEditor_SummerizeStyle.cpp"
#include "..\..\Source\Resources\GuiDocument_Edit.cpp"
#include "..\..\Source\Resources\GuiDocument_Load.cpp"
#include "..\..\Source\Resources\GuiDocument_Save.cpp"
#include "..\..\Source\Resources\GuiParserManager.cpp"
#include "..\..\Source\Resources\GuiPluginManager.cpp"
#include "..\..\Source\Resources\GuiResource.cpp"
#include "..\..\Source\Resources\GuiResourceManager.cpp"
#include "..\..\Source\Resources\GuiResourceTypeResolvers.cpp"
#include "..\..\Source\Utilities\GuiUtilitiesRegistration.cpp"
#include "..\..\Source\Utilities\FakeServices\GuiFakeClipboardService.cpp"
#include "..\..\Source\Utilities\FakeServices\GuiFakeDialogServiceBase.cpp"
#include "..\..\Source\Utilities\FakeServices\GuiFakeDialogServiceBase_ColorDialog.cpp"
#include "..\..\Source\Utilities\FakeServices\GuiFakeDialogServiceBase_FileDialog.cpp"
#include "..\..\Source\Utilities\FakeServices\GuiFakeDialogServiceBase_FontDialog.cpp"
#include "..\..\Source\Utilities\FakeServices\GuiFakeDialogServiceBase_MessageBox.cpp"
#include "..\..\Source\Utilities\FakeServices\Dialogs\GuiFakeDialogService.cpp"
#include "..\..\Source\Utilities\FakeServices\Dialogs\Source\GuiFakeDialogServiceUI.cpp"
#include "..\..\Source\Utilities\FakeServices\Dialogs\Source\GuiFakeDialogServiceUIResource.cpp"
#include "..\..\Source\Utilities\SharedServices\GuiSharedAsyncService.cpp"
#include "..\..\Source\Utilities\SharedServices\GuiSharedCallbackService.cpp"
