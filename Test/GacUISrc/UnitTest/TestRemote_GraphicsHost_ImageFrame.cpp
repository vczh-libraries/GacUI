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
			// The first created image is assigned 8 as its id because FakeDialogService has loaded 7 images
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:8, data:30x40})",
				L"Created(<1:ImageFrame>)",
				L"Updated(1, (8:0), Left, Top, <flags:[e]>)",
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
			// The first created image is assigned 8 as its id because FakeDialogService has loaded 7 images
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:8, data:30x40})",
				L"Created(<1:ImageFrame>)",
				L"Updated(1, (8:0), Left, Top, <flags:[e]>)",
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
			// Size (0,0) is updated to the composition because the image is stretched
			AssertEventLogs(
				eventLogs,
				L"Updated(1, (8:0), Left, Top, <flags:[s]>)",
				L"Begin()",
				L"Render(1, {10,10:30,40}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogs(
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
			// Size of the composition becomes (0,0)
			L"Render(1, {10,10:0,0}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:9, data:50x60})",
				L"ImageDestroyed(8)",
				L"Updated(1, (9:0), Left, Top, <flags:[s]>)",
				L"Begin()",
				L"Render(1, {10,10:0,0}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.createdImages);
		});

		AssertRenderingEventLogsNoLayout(
			protocol,
			eventLogs,
			[&]()
			{
				element->SetStretch(false);
			},
			// Size is not changed because it is still stretched
			L"Render(1, {10,10:0,0}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Size (50,60) is updated to the composition because the image is not stretched
			AssertEventLogs(
				eventLogs,
				L"Updated(1, (9:0), Left, Top, <flags:>)",
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
			// The first created image is assigned 8 as its id because FakeDialogService has loaded 7 images
			// Image is created when calling GuiImageFrameElement::SetImage, size already sent back
			// Render for the first time, the size of image is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"ImageCreated({id:8, data:30x40})",
				L"ImageCreated({id:9, data:50x60})",
				L"Created(<1:ImageFrame>, <2:ImageFrame>, <3:ImageFrame>)",
				L"Updated(1, (8:0), Left, Top, <flags:[e]>)",
				L"Updated(2, (8:0), Left, Top, <flags:[e]>)",
				L"Updated(3, (9:0), Left, Top, <flags:[e]>)",
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
				// Reconnect again
				protocol.GetEvents()->OnControllerDisconnect();
				protocol.GetEvents()->OnControllerConnect();
			},
			// Size of the composition becomes (30,40) and (50,60)
			L"Render(1, {10,10:30,40}, {0,0:640,480})",
			L"Render(2, {20,20:30,40}, {0,0:640,480})",
			L"Render(3, {30,30:50,60}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// There will be no ImageCreated
			// Instead metadata will be asked via Updated
			AssertEventLogs(
				eventLogs,
				L"Created(<1:ImageFrame>, <2:ImageFrame>, <3:ImageFrame>)",
				L"Updated(1, (8:0), Left, Top, <flags:[e]>, <imageCreation:{id:8, data:30x40}>)",
				L"Updated(2, (8:0), Left, Top, <flags:[e]>, <imageCreation:{id:8, data:omitted}>)",
				L"Updated(3, (9:0), Left, Top, <flags:[e]>, <imageCreation:{id:9, data:50x60}>)",
				L"Begin()",
				L"Render(1, {10,10:30,40}, {0,0:640,480})",
				L"Render(2, {20,20:30,40}, {0,0:640,480})",
				L"Render(3, {30,30:50,60}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.createdImages);
		});

		protocol.OnNextFrame([&]()
		{
			// Nothing really get updated
			AssertEventLogs(eventLogs);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}