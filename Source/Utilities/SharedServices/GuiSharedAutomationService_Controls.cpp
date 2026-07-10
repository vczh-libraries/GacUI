#include "GuiSharedAutomationService_Controls.h"
#include "../../PlatformProviders/Hosted/GuiHostedController.h"
#include "../../PlatformProviders/Remote/GuiRemoteController.h"
#include "../../GraphicsComposition/IncludeAll.h"
#include "../../GraphicsElement/GuiGraphicsDocumentElement.h"
#include "../../Resources/GuiDocument.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace controls;
		using namespace compositions;
		using namespace elements;
		using namespace glr::xml;
		using namespace remoteprotocol;
		using namespace stream;

/***********************************************************************
AutomationService
***********************************************************************/

		WString AutomationService::DumpControlTreeInternal()
		{
			auto app = GetApplication();
			auto mainWindow = app->GetMainWindow();

			auto dumpRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(dumpRoot, L"WindowManagement", WString::Unmanaged(L"MultiWindow"));
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"MainWindow";
				field->value = DumpWindowClientArea(mainWindow, GetNativeWindowId(mainWindow->GetNativeWindow()), { 0,0 });
			}
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"SubWindows";

				auto subWindows = Ptr(new glr::json::JsonArray);
				field->value = subWindows;

				for (auto subWindow : app->windows)
				{
					if (subWindow != mainWindow && subWindow->GetVisible() && !dynamic_cast<GuiPopup*>(subWindow))
					{
						subWindows->items.Add(DumpWindowClientArea(subWindow, GetNativeWindowId(subWindow->GetNativeWindow()), { 0,0 }));
					}
				}
			}
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"Popups";

				auto popups = Ptr(new glr::json::JsonArray);
				field->value = popups;

				for (auto popup : app->openingPopups)
				{
					popups->items.Add(DumpWindowClientArea(popup, GetNativeWindowId(popup->GetNativeWindow()), { 0,0 }));
				}
			}
			return DumpJsonToString(dumpRoot);
		}

		AutomationService::AutomationService()
		{
		}

		AutomationService::~AutomationService()
		{
		}

		bool AutomationService::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
AutomationServiceHosted
***********************************************************************/

		Nullable<WString> AutomationServiceHosted::GetNativeWindowId(INativeWindow* window)
		{
			return {};
		}

		INativeWindow* AutomationServiceHosted::GetNativeWindow(Nullable<WString> windowId)
		{
			return GetHostedApplication()->GetNativeWindowHost();
		}

		WString AutomationServiceHosted::DumpControlTreeInternal()
		{
			auto app = GetApplication();
			auto mainWindow = app->GetMainWindow();

			auto dumpRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(dumpRoot, L"WindowManagement", windowManagement);
			{
				auto field = Ptr(new glr::json::JsonObjectField);
				dumpRoot->fields.Add(field);
				field->name.value = L"MainWindow";
				field->value = DumpWindowClientArea(mainWindow, {}, { 0,0 });
				{
					auto subWindowsField = Ptr(new glr::json::JsonObjectField);
					field->value.Cast<glr::json::JsonObject>()->fields.Add(subWindowsField);
					subWindowsField->name.value = L"subWindowsInZOrder";

					auto subWindows = Ptr(new glr::json::JsonArray);
					subWindowsField->value = subWindows;

					auto hostedController = static_cast<GuiHostedController*>(GetHostedApplication());
					for (auto wmWindow : From(hostedController->wmManager->topMostedWindowsInOrder).Concat(hostedController->wmManager->ordinaryWindowsInOrder).Reverse())
					{
						if (wmWindow->id == mainWindow->GetNativeWindow()) continue;
						auto subWindow = app->windowMap[wmWindow->id];
						auto offset = mainWindow->GetNativeWindow()->Convert(wmWindow->bounds.LeftTop());
						subWindows->items.Add(DumpWindowClientArea(subWindow, {}, offset));
					}
				}
			}
			return DumpJsonToString(dumpRoot);
		}

		AutomationServiceHosted::AutomationServiceHosted()
		{
		}

		AutomationServiceHosted::~AutomationServiceHosted()
		{
		}

		bool AutomationServiceHosted::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
RemoteProtocolAutomationService
***********************************************************************/

		WString RemoteProtocolAutomationService::RunIOCommandInternal(Nullable<WString> windowId, const WString& ioCommand)
		{
			auto window = dynamic_cast<GuiRemoteWindow*>(this->GetNativeWindow(windowId));
			if (!window)
			{
				return L"!Invalid window.";
			}

			return RunIOCommandOnNativeWindow(&ioCommandState, GetHostedApplication()->GetNativeController(), window, window->listeners, ioCommand);
		}

		RemoteProtocolAutomationService::RemoteProtocolAutomationService()
		{
			windowManagement = WString::Unmanaged(L"HostedRemoteProtocol");
		}

		RemoteProtocolAutomationService::~RemoteProtocolAutomationService()
		{
		}

		INativeAutomationService::IOCommandAvailability RemoteProtocolAutomationService::CanRunIOCommands()
		{
			return INativeAutomationService::IOCommandAvailability::Enabled;
		}

/***********************************************************************
DumpWindowClientArea
***********************************************************************/

		WString PrintControlThemeName(theme::ThemeName theme)
		{
			switch (theme)
			{
			case theme::ThemeName::Window: return WString::Unmanaged(L"Window");
#define GUI_DEFINE_THEME_NAME(TEMPLATE, CONTROL) case theme::ThemeName::CONTROL: return WString::Unmanaged(L ## #CONTROL);
				GUI_CONTROL_TEMPLATE_TYPES(GUI_DEFINE_THEME_NAME)
#undef GUI_DEFINE_THEME_NAME
			default: return WString::Unmanaged(L"Unknown");
			}
		}

		void AddJsonField(Ptr<glr::json::JsonObject> dump, const WString& name, Ptr<glr::json::JsonNode> value)
		{
			auto field = Ptr(new glr::json::JsonObjectField);
			dump->fields.Add(field);
			field->name.value = name;
			field->value = value;
		}

		Rect OffsetRect(Rect bounds, Point offset)
		{
			bounds.Move(offset.x, offset.y);
			return bounds;
		}

		WString HexByte(unsigned char value)
		{
			const wchar_t* code = L"0123456789ABCDEF";
			return WString::FromChar(code[value / 16]) + WString::FromChar(code[value % 16]);
		}

		WString PrintColor(Color color)
		{
			return WString::Unmanaged(L"#") + HexByte(color.r) + HexByte(color.g) + HexByte(color.b) + HexByte(color.a);
		}

		WString PrintElementShape(ElementShape shape)
		{
			switch (shape.shapeType)
			{
			case ElementShapeType::Rectangle:
				return WString::Unmanaged(L"Rectangle");
			case ElementShapeType::Ellipse:
				return WString::Unmanaged(L"Ellipse");
			case ElementShapeType::RoundRect:
				return L"RoundRect(" + itow(shape.radiusX) + L"," + itow(shape.radiusY) + L")";
			default:
				return WString::Unmanaged(L"Unknown");
			}
		}

		WString PrintSplitterDirection(Gui3DSplitterElement::Direction direction)
		{
			switch (direction)
			{
			case Gui3DSplitterElement::Horizontal:
				return WString::Unmanaged(L"Horizontal");
			case Gui3DSplitterElement::Vertical:
				return WString::Unmanaged(L"Vertical");
			default:
				return WString::Unmanaged(L"Unknown");
			}
		}

		WString PrintGradientDirection(GuiGradientBackgroundElement::Direction direction)
		{
			switch (direction)
			{
			case GuiGradientBackgroundElement::Horizontal:
				return WString::Unmanaged(L"Horizontal");
			case GuiGradientBackgroundElement::Vertical:
				return WString::Unmanaged(L"Vertical");
			case GuiGradientBackgroundElement::Slash:
				return WString::Unmanaged(L"Slash");
			case GuiGradientBackgroundElement::Backslash:
				return WString::Unmanaged(L"Backslash");
			default:
				return WString::Unmanaged(L"Unknown");
			}
		}

		WString PrintTextPos(TextPos position)
		{
			return L"(" + itow(position.row) + L"," + itow(position.column) + L")";
		}

		WString PrintFontStyle(const FontProperties& fontProperties)
		{
			return fontProperties.fontFamily + L"," + itow(fontProperties.size);
		}

		WString PrintLayout(GuiGraphicsComposition* composition)
		{
			if (auto table = dynamic_cast<GuiTableComposition*>(composition))
			{
				return L"Table:" + itow(table->GetRows()) + L"*" + itow(table->GetColumns());
			}
			else if (auto cell = dynamic_cast<GuiCellComposition*>(composition))
			{
				if (dynamic_cast<GuiTableComposition*>(cell->GetParent()))
				{
					return L"Cell:(" + itow(cell->GetRow()) + L"," + itow(cell->GetColumn()) + L")*(" + itow(cell->GetRowSpan()) + L"," + itow(cell->GetColumnSpan()) + L")";
				}
			}
			else if (auto rowSplitter = dynamic_cast<GuiRowSplitterComposition*>(composition))
			{
				return L"RowSplitter:" + itow(rowSplitter->GetRowsToTheTop());
			}
			else if (auto columnSplitter = dynamic_cast<GuiColumnSplitterComposition*>(composition))
			{
				return L"ColumnSplitter:" + itow(columnSplitter->GetColumnsToTheLeft());
			}
			else if (dynamic_cast<GuiStackComposition*>(composition))
			{
				return WString::Unmanaged(L"Stack");
			}
			else if (auto stackItem = dynamic_cast<GuiStackItemComposition*>(composition))
			{
				if (auto stack = dynamic_cast<GuiStackComposition*>(stackItem->GetParent()))
				{
					auto index = stack->GetStackItems().IndexOf(stackItem);
					if (index != -1)
					{
						return L"StackItem:" + itow(index);
					}
				}
			}
			else if (dynamic_cast<GuiFlowComposition*>(composition))
			{
				return WString::Unmanaged(L"Flow");
			}
			else if (auto flowItem = dynamic_cast<GuiFlowItemComposition*>(composition))
			{
				if (auto flow = dynamic_cast<GuiFlowComposition*>(flowItem->GetParent()))
				{
					auto index = flow->GetFlowItems().IndexOf(flowItem);
					if (index != -1)
					{
						return L"FlowItem:" + itow(index);
					}
				}
			}
			return WString::Empty;
		}

		WString PrintCursor(INativeCursor* cursor)
		{
			if (!cursor) return WString::Empty;
			if (!cursor->IsSystemCursor()) return WString::Unmanaged(L"Custom");

			switch (cursor->GetSystemCursorType())
			{
			case INativeCursor::SmallWaiting: return WString::Unmanaged(L"SmallWaiting");
			case INativeCursor::LargeWaiting: return WString::Unmanaged(L"LargeWaiting");
			case INativeCursor::Arrow: return WString::Unmanaged(L"Arrow");
			case INativeCursor::Cross: return WString::Unmanaged(L"Cross");
			case INativeCursor::Hand: return WString::Unmanaged(L"Hand");
			case INativeCursor::Help: return WString::Unmanaged(L"Help");
			case INativeCursor::IBeam: return WString::Unmanaged(L"IBeam");
			case INativeCursor::SizeAll: return WString::Unmanaged(L"SizeAll");
			case INativeCursor::SizeNESW: return WString::Unmanaged(L"SizeNESW");
			case INativeCursor::SizeNS: return WString::Unmanaged(L"SizeNS");
			case INativeCursor::SizeNWSE: return WString::Unmanaged(L"SizeNWSE");
			case INativeCursor::SizeWE: return WString::Unmanaged(L"SizeWE");
			default: return WString::Unmanaged(L"Unknown");
			}
		}

		WString PrintDocument(Ptr<DocumentModel> document)
		{
			if (!document) return WString::Empty;
			return GenerateToStream([&](StreamWriter& writer)
			{
				XmlPrint(document->SaveToXml(), writer);
			});
		}

		void DumpElement(Ptr<IGuiGraphicsElement> element, Ptr<glr::json::JsonObject> compositionDump)
		{
			if (!element) return;
			auto rawElement = element.Obj();
			WString elementDescription;

			if (dynamic_cast<GuiFocusRectangleElement*>(rawElement))
			{
				elementDescription = WString::Unmanaged(L"FocusRectangle");
			}
			else if (auto border = dynamic_cast<GuiSolidBorderElement*>(rawElement))
			{
				elementDescription = L"Border:" + PrintColor(border->GetColor()) + L"," + PrintElementShape(border->GetShape());
			}
			else if (auto border3D = dynamic_cast<Gui3DBorderElement*>(rawElement))
			{
				elementDescription = L"3DBorder:" + PrintColor(border3D->GetColor1()) + L"," + PrintColor(border3D->GetColor2());
			}
			else if (auto splitter3D = dynamic_cast<Gui3DSplitterElement*>(rawElement))
			{
				elementDescription = L"3DSplitter:" + PrintColor(splitter3D->GetColor1()) + L"," + PrintColor(splitter3D->GetColor2()) + L"," + PrintSplitterDirection(splitter3D->GetDirection());
			}
			else if (auto background = dynamic_cast<GuiSolidBackgroundElement*>(rawElement))
			{
				elementDescription = L"Background:" + PrintColor(background->GetColor()) + L"," + PrintElementShape(background->GetShape());
			}
			else if (auto gradient = dynamic_cast<GuiGradientBackgroundElement*>(rawElement))
			{
				elementDescription = L"Gradient:" + PrintColor(gradient->GetColor1()) + L"," + PrintColor(gradient->GetColor2()) + L"," + PrintGradientDirection(gradient->GetDirection()) + L"," + PrintElementShape(gradient->GetShape());
			}
			else if (auto shadow = dynamic_cast<GuiInnerShadowElement*>(rawElement))
			{
				elementDescription = L"InnerShadow:" + PrintColor(shadow->GetColor()) + L"," + itow(shadow->GetThickness());
			}
			else if (auto label = dynamic_cast<GuiSolidLabelElement*>(rawElement))
			{
				elementDescription = L"Label:" + PrintColor(label->GetColor()) + L"," + PrintFontStyle(label->GetFont());
				if (label->GetWrapLine()) elementDescription += L",WrapLine";
				if (label->GetEllipse()) elementDescription += L",Ellipse";
				if (label->GetMultiline()) elementDescription += L",Multiline";
				ConvertCustomTypeToJsonField(compositionDump, L"elementText", label->GetText());
			}
			else if (dynamic_cast<GuiImageFrameElement*>(rawElement))
			{
				elementDescription = WString::Unmanaged(L"Image");
			}
			else if (dynamic_cast<GuiPolygonElement*>(rawElement))
			{
				elementDescription = WString::Unmanaged(L"Polygon");
			}
			else if (auto document = dynamic_cast<GuiDocumentElement*>(rawElement))
			{
				elementDescription = L"Document:Selection" + PrintTextPos(document->GetCaretBegin()) + L"-" + PrintTextPos(document->GetCaretEnd());
				if (auto passwordChar = document->GetPasswordChar())
				{
					elementDescription += L",PasswordChar=" + WString::FromChar(passwordChar);
				}
				if (document->GetWrapLine())
				{
					elementDescription += L",WrapLine";
				}
				ConvertCustomTypeToJsonField(compositionDump, L"elementDocument", PrintDocument(document->GetDocument()));
			}

			if (elementDescription != WString::Empty)
			{
				ConvertCustomTypeToJsonField(compositionDump, L"element", elementDescription);
			}
		}

		Ptr<glr::json::JsonNode> DumpComposition(GuiGraphicsComposition* composition, Point offset)
		{
			auto compositionDump = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(compositionDump, L"bounds", OffsetRect(composition->GetGlobalBounds(), offset));

			auto layout = PrintLayout(composition);
			if (layout != WString::Empty)
			{
				ConvertCustomTypeToJsonField(compositionDump, L"layout", layout);
			}

			auto cursor = PrintCursor(composition->GetAssociatedCursor());
			if (cursor != WString::Empty)
			{
				ConvertCustomTypeToJsonField(compositionDump, L"cursor", cursor);
			}

			DumpElement(composition->GetOwnedElement(), compositionDump);

			if (auto control = composition->GetAssociatedControl())
			{
				ConvertCustomTypeToJsonField(compositionDump, L"control", PrintControlThemeName(control->GetControlThemeName()));
			}

			if (composition->Children().Count() > 0)
			{
				auto children = Ptr(new glr::json::JsonArray);
				for (auto child : composition->Children())
				{
					if (child->GetEventuallyVisible())
					{
						children->items.Add(DumpComposition(child, offset));
					}
				}
				if (children->items.Count() > 0)
				{
					AddJsonField(compositionDump, L"children", children);
				}
			}

			return compositionDump;
		}

		Ptr<glr::json::JsonNode> DumpWindowClientArea(controls::GuiWindow* window, Nullable<WString> windowId, Point offset)
		{
			auto windowDump = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(windowDump, L"title", window->GetText());
			if (windowId)
			{
				ConvertCustomTypeToJsonField(windowDump, L"windowId", windowId.Value());
			}

			{
				Size size = window->GetBoundsComposition()->GetCachedBounds().GetSize();
				ConvertCustomTypeToJsonField(windowDump, L"bounds", Rect(offset, size));
			}
			AddJsonField(windowDump, L"composition", DumpComposition(window->GetBoundsComposition(), offset));
			return windowDump;
		}
	}
}
