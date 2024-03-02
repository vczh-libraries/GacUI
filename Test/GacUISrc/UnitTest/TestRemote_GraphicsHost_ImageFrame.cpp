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
			// Render for the first time and send back size of the image
			AssertEventLogs(
				eventLogs,
				L"Created(<1:ImageElement>)",
				L"ImageCreated({id:0, data:30x40})",
				L"Updated(1, (0:0), Left, Top, <flags:>, <imageCreation:{id:0, data:omitted}>)",
				L"Begin()",
				L"Render(1, {0,0:0,0}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.createdImages);
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the second time and the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {10,10:0,0}, {0,0:640,480})",
				L"End()"
				);
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
}