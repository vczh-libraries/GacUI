#include "TestRemote_GraphicsHost_Shared.h"

using namespace vl::stream;

TEST_FILE
{
	TEST_CATEGORY(L"Simple ImageFrame")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			MemoryStream imageData;
			{
				StreamWriter_<char8_t> writer(imageData);
				writer.WriteString(u8"30x40");
			}
			auto image = GetCurrentController()->ImageService()->CreateImageFromStream(imageData);

			auto bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, -1, -1));
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

			auto element = Ptr(GuiImageFrameElement::Create());
			element->SetImage(image, 0);
			bounds->SetOwnedElement(element);

			controlHost->GetContainerComposition()->AddChild(bounds);
		});

		protocol.OnNextFrame([&]()
		{
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:0, data:30x40})",
				L"Created(<1:ImageFrame>)",
				L"Updated(1, (0:0), Left, Top, <flags:[e]>)",
				L"Begin()",
				L"Render(1, {0,0:0,0}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.createdImages);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				controlHost->Hide();
			},
			// Size of the composition becomes (30,40)
			L"Render(1, {10,10:30,40}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Stretch and Disabled")
	{
	});

	TEST_CATEGORY(L"Change image and delete old one")
	{
	});

	TEST_CATEGORY(L"Disconnect after caching image metadata to trigger recache")
	{
	});
}