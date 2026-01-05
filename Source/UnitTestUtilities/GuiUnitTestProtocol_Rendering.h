/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_RENDERING
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_RENDERING

#include "GuiUnitTestProtocol_Shared.h"
#include "../PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"
#include "../PlatformProviders/Remote/GuiRemoteGraphics_Document.h"

namespace vl::presentation::unittest
{

/***********************************************************************
DocumentParagraphState
***********************************************************************/

	struct DocumentParagraphCharLayout
	{
		double									x;               // X position of character start
		double									width;           // Character width
		vint									lineIndex;       // Which line this character belongs to
		vint									height;          // Character height (font size or inline object height)
		vint									baseline;        // Distance from top to baseline (for alignment)
		bool									isInlineObject;  // True if this position is part of an inline object
	};

	struct DocumentParagraphLineInfo
	{
		vint									startPos;        // Text position of line start (inclusive)
		vint									endPos;          // Text position of line end (exclusive)
		vint									y;               // Y coordinate of line top
		vint									height;          // Line height (computed from baseline alignment)
		vint									baseline;        // Line baseline position from line top
		vint									width;           // Line width (for alignment calculation)
	};

	struct DocumentParagraphState
	{
		WString											text;
		bool											wrapLine = false;
		vint											maxWidth = -1;
		remoteprotocol::ElementHorizontalAlignment		alignment = remoteprotocol::ElementHorizontalAlignment::Left;

		elements::DocumentTextRunPropertyMap			textRuns;
		elements::DocumentInlineObjectRunPropertyMap	inlineObjectRuns;
		elements::DocumentRunPropertyMap				mergedRuns;

		collections::List<DocumentParagraphCharLayout>	characterLayouts;
		collections::List<DocumentParagraphLineInfo>	lines;
		Size											cachedSize;
	};

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/

	using ElementDescVariant = remoteprotocol::UnitTest_ElementDescVariant;
	using UnitTestRenderingDom = remoteprotocol::RenderingDom;

	struct UnitTestLoggedFrame
	{
		vint													frameId;
		collections::List<WString>								renderingCommandsLog;
		Nullable<remoteprotocol::RenderingDom_DiffsInOrder>		renderingDiffs;
		Ptr<UnitTestRenderingDom>								renderingDom;
	};

	using UnitTestLoggedFrameList = collections::List<Ptr<UnitTestLoggedFrame>>;
	
	class UnitTestRemoteProtocol_Rendering : public virtual UnitTestRemoteProtocolBase
	{
		using IdSet = collections::SortedList<vint>;
		using Base64ToImageMetadataMap = collections::Dictionary<WString, remoteprotocol::ImageMetadata>;
		using ElementDescMap = collections::Dictionary<vint, ElementDescVariant>;
		using ImageMetadataMap = collections::Dictionary<vint, remoteprotocol::ImageMetadata>;

	protected:
		remoteprotocol::RenderingDomBuilder				renderingDomBuilder;
		remoteprotocol::UnitTest_RenderingTrace			loggedTrace;
		UnitTestLoggedFrameList							loggedFrames;
		bool											lastRenderingCommandsOpening = false;

		Ptr<remoteprotocol::RenderingDom>				receivedDom;
		remoteprotocol::DomIndex						receivedDomIndex;
		bool											receivedDomDiffMessage = false;
		bool											receivedElementMessage = false;

		ElementDescMap									lastElementDescs;
		IdSet											removedElementIds;
		IdSet											removedImageIds;
		Ptr<Base64ToImageMetadataMap>					cachedImageMetadatas;

		remoteprotocol::ElementMeasurings				measuringForNextRendering;
		regex::Regex									regexCrLf{ L"/n|/r(/n)?" };

		collections::Dictionary<vint, Ptr<DocumentParagraphState>> paragraphStates;

		void ResetCreatedObjects()
		{
			loggedTrace.createdElements = Ptr(new collections::Dictionary<vint, remoteprotocol::RendererType>);
			loggedTrace.imageCreations = Ptr(new remoteprotocol::ArrayMap<vint, remoteprotocol::ImageCreation, &remoteprotocol::ImageCreation::id>);
			loggedTrace.imageMetadatas = Ptr(new remoteprotocol::ArrayMap<vint, remoteprotocol::ImageMetadata, &remoteprotocol::ImageMetadata::id>);
			lastElementDescs.Clear();
		}
	public:

		UnitTestRemoteProtocol_Rendering(const UnitTestScreenConfig& _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
		{
			ResetCreatedObjects();
			loggedTrace.frames = Ptr(new collections::List<remoteprotocol::UnitTest_RenderingFrame>);
		}

	protected:

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering)
***********************************************************************/

		Ptr<UnitTestLoggedFrame>			GetLastRenderingFrame();
		Ptr<UnitTestLoggedFrame>			TryGetLastRenderingFrameAndReset();
		void								Impl_RendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments);
		void								Impl_RendererEndRendering(vint id);

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - Element)
***********************************************************************/

		void								Impl_RendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments);
		void								Impl_RendererEndBoundary();
		void								EnsureRenderedElement(vint id, Rect bounds);
		void								Impl_RendererRenderElement(const remoteprotocol::ElementRendering& arguments);

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - Dom)
***********************************************************************/

		void								CalculateSolidLabelSizesIfNecessary(Ptr<remoteprotocol::RenderingDom> dom);
		void								Impl_RendererRenderDom(const Ptr<remoteprotocol::RenderingDom>& arguments);
		void								Impl_RendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments);

/***********************************************************************
IGuiRemoteProtocolMessages (Elements)
***********************************************************************/

		void								Impl_RendererCreated(const Ptr<collections::List<remoteprotocol::RendererCreation>>& arguments);
		void								Impl_RendererDestroyed(const Ptr<collections::List<vint>>& arguments);

		template<remoteprotocol::RendererType RendererType, typename TElementDesc>
		void RequestRendererUpdateElement(const TElementDesc& arguments, const wchar_t* emWrongId, const wchar_t* emWrongType)
		{
			vint index = loggedTrace.createdElements->Keys().IndexOf(arguments.id);
			CHECK_ERROR(index != -1, emWrongId);
			CHECK_ERROR(loggedTrace.createdElements->Values()[index] == RendererType, emWrongType);
			lastElementDescs.Set(arguments.id, arguments);
		}

		void								Impl_RendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments);
		void								Impl_RendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments);
		void								Impl_RendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments);
		void								Impl_RendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments);
		void								Impl_RendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments);
		void								Impl_RendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments);
		void								Impl_RendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments);

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - SolidLabel)
***********************************************************************/

		void								CalculateSolidLabelSizeIfNecessary(vint width, vint height, const remoteprotocol::ElementDesc_SolidLabel& arguments);
		void								Impl_RendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments);

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - Image)
***********************************************************************/

		WString								GetBinaryKeyFromBinary(stream::IStream& binary);
		WString								GetBinaryKeyFromImage(Ptr<INativeImage> image);
		remoteprotocol::ImageMetadata		MakeImageMetadata(const remoteprotocol::ImageCreation& arguments);
		void								Impl_ImageCreated(vint id, const remoteprotocol::ImageCreation& arguments);
		void								Impl_ImageDestroyed(const vint& arguments);
		void								Impl_RendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments);

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - Document)
***********************************************************************/

		void								Impl_RendererUpdateElement_DocumentParagraph(vint id, const remoteprotocol::ElementDesc_DocumentParagraph& arguments);
		void								Impl_DocumentParagraph_GetCaret(vint id, const remoteprotocol::GetCaretRequest& arguments);
		void								Impl_DocumentParagraph_GetCaretBounds(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments);
		void								Impl_DocumentParagraph_GetInlineObjectFromPoint(vint id, const remoteprotocol::GetInlineObjectFromPointRequest & arguments);
		void								Impl_DocumentParagraph_GetNearestCaretFromTextPos(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments);
		void								Impl_DocumentParagraph_IsValidCaret(vint id, const remoteprotocol::IsValidCaretRequest& arguments);
		void								Impl_DocumentParagraph_OpenCaret(const remoteprotocol::OpenCaretRequest& arguments);
		void								Impl_DocumentParagraph_CloseCaret(const vint& arguments);
	};
}

#endif