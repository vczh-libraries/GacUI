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
				L"ImageCreated({id:1, data:30x40})",
				L"Created(<1:ImageFrame>)",
				L"Updated(1, (1:0), Left, Top, <flags:[e]>)",
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

	TEST_CATEGORY(L"Change properties")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;
		Ptr<GuiImageFrameElement> element;

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

			element = Ptr(GuiImageFrameElement::Create());
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
				L"ImageCreated({id:1, data:30x40})",
				L"Created(<1:ImageFrame>)",
				L"Updated(1, (1:0), Left, Top, <flags:[e]>)",
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
				element->SetStretch(true);
				element->SetEnabled(false);
			},
			// Size of the composition becomes (30,40)
			L"Render(1, {10,10:30,40}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Size is not changed
			AssertEventLogs(
				eventLogs,
				L"Updated(1, (1:0), Left, Top, <flags:[s]>)",
				L"Begin()",
				L"Render(1, {10,10:30,40}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogsNoLayout(
			protocol,
			eventLogs,
			[&]()
			{
				MemoryStream imageData;
				{
					StreamWriter_<char8_t> writer(imageData);
					writer.WriteString(u8"50x60");
				}
				auto image = GetCurrentController()->ImageService()->CreateImageFromStream(imageData);
				element->SetImage(image);
			},
			L"Render(1, {10,10:30,40}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:2, data:50x60})",
				L"ImageDestroyed(1)",
				L"Updated(1, (2:0), Left, Top, <flags:[s]>)",
				L"Begin()",
				L"Render(1, {10,10:30,40}, {0,0:640,480})",
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
			// Size of the composition becomes (50,60)
			L"Render(1, {10,10:50,60}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Disconnect after caching image metadata to trigger recache")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			Ptr<INativeImage> image1, image2;
			{
				MemoryStream imageData;
				{
					StreamWriter_<char8_t> writer(imageData);
					writer.WriteString(u8"30x40");
				}
				image1 = GetCurrentController()->ImageService()->CreateImageFromStream(imageData);
			}
			{
				MemoryStream imageData;
				{
					StreamWriter_<char8_t> writer(imageData);
					writer.WriteString(u8"50x60");
				}
				image2 = GetCurrentController()->ImageService()->CreateImageFromStream(imageData);
			}
			{
				auto bounds = new GuiBoundsComposition;
				bounds->SetAlignmentToParent(Margin(10, 10, -1, -1));
				bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

				auto element = Ptr(GuiImageFrameElement::Create());
				element->SetImage(image1, 0);
				bounds->SetOwnedElement(element);

				controlHost->GetContainerComposition()->AddChild(bounds);
			}
			{
				auto bounds = new GuiBoundsComposition;
				bounds->SetAlignmentToParent(Margin(20, 20, -1, -1));
				bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

				auto element = Ptr(GuiImageFrameElement::Create());
				element->SetImage(image1, 0);
				bounds->SetOwnedElement(element);

				controlHost->GetContainerComposition()->AddChild(bounds);
			}
			{
				auto bounds = new GuiBoundsComposition;
				bounds->SetAlignmentToParent(Margin(30, 30, -1, -1));
				bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

				auto element = Ptr(GuiImageFrameElement::Create());
				element->SetImage(image2, 0);
				bounds->SetOwnedElement(element);

				controlHost->GetContainerComposition()->AddChild(bounds);
			}
		});

		protocol.OnNextFrame([&]()
		{
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:1, data:30x40})",
				L"ImageCreated({id:2, data:50x60})",
				L"Created(<1:ImageFrame>, <2:ImageFrame>, <3:ImageFrame>)",
				L"Updated(1, (1:0), Left, Top, <flags:[e]>)",
				L"Updated(2, (1:0), Left, Top, <flags:[e]>)",
				L"Updated(3, (2:0), Left, Top, <flags:[e]>)",
				L"Begin()",
				L"Render(1, {0,0:0,0}, {0,0:640,480})",
				L"Render(2, {0,0:0,0}, {0,0:640,480})",
				L"Render(3, {0,0:0,0}, {0,0:640,480})",
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
			// Size of the composition becomes (30,40) and (50,60)
			L"Render(1, {10,10:30,40}, {0,0:640,480})",
			L"Render(2, {20,20:30,40}, {0,0:640,480})",
			L"Render(3, {30,30:50,60}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}