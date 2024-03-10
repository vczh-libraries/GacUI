/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_RENDERING
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_RENDERING

#include "GuiUnitTestProtocol_Shared.h"

namespace vl::presentation::unittest
{

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/
	
	template<typename TProtocol>
	class UnitTestRemoteProtocol_Rendering : public TProtocol
	{
	public:
		template<typename ...TArgs>
		UnitTestRemoteProtocol_Rendering(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering)
***********************************************************************/

		void RequestRendererCreated(const Ptr<collections::List<remoteprotocol::RendererCreation>>& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererDestroyed(const Ptr<collections::List<vint>>& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererBeginRendering() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererEndRendering(vint id) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererEndBoundary() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - SolidLabel)
***********************************************************************/

		void RequestRendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - Image)
***********************************************************************/

		void RequestImageCreated(vint id, const remoteprotocol::ImageCreation& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestImageDestroyed(const vint& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments) override
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	};
}

#endif