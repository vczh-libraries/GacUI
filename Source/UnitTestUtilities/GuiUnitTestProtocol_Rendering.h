/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_RENDERING
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_RENDERING

#include "GuiUnitTestProtocol_Shared.h"
#include "../PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::unittest
{

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
	
	template<typename TProtocol>
	class UnitTestRemoteProtocol_Rendering : public TProtocol
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

		void ResetCreatedObjects()
		{
			loggedTrace.createdElements = Ptr(new collections::Dictionary<vint, remoteprotocol::RendererType>);
			loggedTrace.imageCreations = Ptr(new remoteprotocol::ArrayMap<vint, remoteprotocol::ImageCreation, &remoteprotocol::ImageCreation::id>);
			loggedTrace.imageMetadatas = Ptr(new remoteprotocol::ArrayMap<vint, remoteprotocol::ImageMetadata, &remoteprotocol::ImageMetadata::id>);
			lastElementDescs.Clear();
		}
	public:

		template<typename ...TArgs>
		UnitTestRemoteProtocol_Rendering(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
			ResetCreatedObjects();
			loggedTrace.frames = Ptr(new collections::List<remoteprotocol::UnitTest_RenderingFrame>);
		}

	protected:

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering)
***********************************************************************/

		Ptr<UnitTestLoggedFrame> GetLastRenderingFrame()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::GetLastRenderingCommands()#"
			CHECK_ERROR(lastRenderingCommandsOpening, ERROR_MESSAGE_PREFIX L"The latest frame of commands is not accepting new commands.");
			return loggedFrames[loggedFrames.Count() - 1];
#undef ERROR_MESSAGE_PREFIX
		}

		Ptr<UnitTestLoggedFrame> TryGetLastRenderingFrameAndReset()
		{
			if (loggedFrames.Count() == 0) return nullptr;
			if (!lastRenderingCommandsOpening) return nullptr;
			lastRenderingCommandsOpening = false;
			return loggedFrames[loggedFrames.Count() - 1];
		}

		void RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments) override
		{
			receivedDomDiffMessage = false;
			receivedElementMessage = false;
			lastRenderingCommandsOpening = true;
			auto frame = Ptr(new UnitTestLoggedFrame);
			frame->frameId = arguments.frameId;
			loggedFrames.Add(frame);
		}

		void RequestRendererEndRendering(vint id) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererEndRendering(vint)#"
			CHECK_ERROR(receivedElementMessage || receivedDomDiffMessage, ERROR_MESSAGE_PREFIX L"Either dom-diff or element message should be sent before this message.");

			auto lastFrame = GetLastRenderingFrame();
			if (receivedElementMessage)
			{
				lastFrame->renderingDom = renderingDomBuilder.RequestRendererEndRendering();
			}
			if (receivedDomDiffMessage)
			{
				// receivedDom will be updated in RequestRendererRenderDomDiff
				// store a copy to log
				lastFrame->renderingDom = CopyDom(receivedDom);
			}
			this->GetEvents()->RespondRendererEndRendering(id, measuringForNextRendering);
			measuringForNextRendering = {};
#undef ERROR_MESSAGE_PREFIX
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - Element)
***********************************************************************/

		void RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererBeginBoundary(const ElementBoundary&)#"
			CHECK_ERROR(!receivedDomDiffMessage, ERROR_MESSAGE_PREFIX L"This message could not be used with dom-diff messages in the same rendering cycle.");
			if (!receivedElementMessage)
			{
				renderingDomBuilder.RequestRendererBeginRendering();
				receivedElementMessage = true;
			}

			renderingDomBuilder.RequestRendererBeginBoundary(arguments);

			glr::json::JsonFormatting formatting;
			formatting.spaceAfterColon = true;
			formatting.spaceAfterComma = true;
			formatting.crlf = false;
			formatting.compact = true;

			GetLastRenderingFrame()->renderingCommandsLog.Add(L"RequestRendererBeginBoundary: " + stream::GenerateToStream([&](stream::TextWriter& writer)
			{
				auto jsonLog = remoteprotocol::ConvertCustomTypeToJson(arguments);
				writer.WriteString(glr::json::JsonToString(jsonLog, formatting));
			}));
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererEndBoundary() override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererEndBoundary()#"
			CHECK_ERROR(!receivedDomDiffMessage, ERROR_MESSAGE_PREFIX L"This message could not be used with dom-diff messages in the same rendering cycle.");
			if (!receivedElementMessage)
			{
				renderingDomBuilder.RequestRendererBeginRendering();
				receivedElementMessage = true;
			}

			renderingDomBuilder.RequestRendererEndBoundary();
			GetLastRenderingFrame()->renderingCommandsLog.Add(L"RequestRendererEndBoundary");
#undef ERROR_MESSAGE_PREFIX
		}

		void EnsureRenderedElement(vint id, Rect bounds)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::EnsureRenderedElement(id&)#"

			vint index = loggedTrace.createdElements->Keys().IndexOf(id);
			CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");
			auto rendererType = loggedTrace.createdElements->Values()[index];
			if (rendererType == remoteprotocol::RendererType::FocusRectangle || rendererType == remoteprotocol::RendererType::Raw)
			{
				// FocusRectangle or Raw does not have a ElementDesc
				return;
			}

			index = lastElementDescs.Keys().IndexOf(id);
			CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been updated after created.");
			lastElementDescs.Values()[index].Apply(Overloading(
				[](remoteprotocol::RendererType)
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been updated after created.");
				},
				[&](const remoteprotocol::ElementDesc_SolidLabel& solidLabel)
				{
					CalculateSolidLabelSizeIfNecessary(bounds.Width(), bounds.Height(), solidLabel);
				},
				[&](const auto& element)
				{
				}));

#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererRenderElement(const ElementRendering&)#"
			CHECK_ERROR(!receivedDomDiffMessage, ERROR_MESSAGE_PREFIX L"This message could not be used with dom-diff messages in the same rendering cycle.");
			if (!receivedElementMessage)
			{
				renderingDomBuilder.RequestRendererBeginRendering();
				receivedElementMessage = true;
			}

			{
				renderingDomBuilder.RequestRendererRenderElement(arguments);

				glr::json::JsonFormatting formatting;
				formatting.spaceAfterColon = true;
				formatting.spaceAfterComma = true;
				formatting.crlf = false;
				formatting.compact = true;
				GetLastRenderingFrame()->renderingCommandsLog.Add(L"RequestRendererRenderElement: " + stream::GenerateToStream([&](stream::TextWriter& writer)
				{
					auto jsonLog = remoteprotocol::ConvertCustomTypeToJson(arguments);
					writer.WriteString(glr::json::JsonToString(jsonLog, formatting));
				}));
			}

			EnsureRenderedElement(arguments.id, arguments.bounds);
#undef ERROR_MESSAGE_PREFIX
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering - Dom)
***********************************************************************/

		void CalculateSolidLabelSizesIfNecessary(Ptr<remoteprotocol::RenderingDom> dom)
		{
			if (dom->content.element)
			{
				EnsureRenderedElement(dom->content.element.Value(), dom->content.bounds);
			}
			if (dom->children)
			{
				for (auto child : *dom->children.Obj())
				{
					CalculateSolidLabelSizesIfNecessary(child);
				}
			}
		}

		void RequestRendererRenderDom(const Ptr<remoteprotocol::RenderingDom>& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererRenderElement(const RenderingDom&)#"
			CHECK_ERROR(!receivedElementMessage, ERROR_MESSAGE_PREFIX L"This message could not be used with element messages in the same rendering cycle.");
			if (!receivedDomDiffMessage)
			{
				receivedDomDiffMessage = true;
			}

			receivedDom = arguments;
			remoteprotocol::BuildDomIndex(receivedDom, receivedDomIndex);
			CalculateSolidLabelSizesIfNecessary(receivedDom);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererRenderElement(const RenderingDom_DiffsInOrder&)#"
			CHECK_ERROR(!receivedElementMessage, ERROR_MESSAGE_PREFIX L"This message could not be used with element messages in the same rendering cycle.");
			if (!receivedDomDiffMessage)
			{
				receivedDomDiffMessage = true;
			}
			
			remoteprotocol::UpdateDomInplace(receivedDom, receivedDomIndex, arguments);
			GetLastRenderingFrame()->renderingDiffs = arguments;
			CalculateSolidLabelSizesIfNecessary(receivedDom);
#undef ERROR_MESSAGE_PREFIX
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Elements)
***********************************************************************/

		void RequestRendererCreated(const Ptr<collections::List<remoteprotocol::RendererCreation>>& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererCreated(const Ptr<List<RendererCreation>>&)#"
			if (arguments)
			{
				for (auto creation : *arguments.Obj())
				{
					CHECK_ERROR(!loggedTrace.createdElements->Keys().Contains(creation.id), ERROR_MESSAGE_PREFIX L"Renderer with the specified id has been created or used.");
					loggedTrace.createdElements->Add(creation.id, creation.type);
				}
			}
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererDestroyed(const Ptr<collections::List<vint>>& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestRendererDestroyed(const Ptr<List<vint>>&)#"
			if (arguments)
			{
				for (auto id : *arguments.Obj())
				{
					CHECK_ERROR(loggedTrace.createdElements->Keys().Contains(id), ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");
					CHECK_ERROR(!removedElementIds.Contains(id), ERROR_MESSAGE_PREFIX L"Renderer with the specified id has been destroyed.");
					removedElementIds.Add(id);
					lastElementDescs.Remove(id);
				}
			}
#undef ERROR_MESSAGE_PREFIX
		}

		template<remoteprotocol::RendererType RendererType, typename TElementDesc>
		void RequestRendererUpdateElement(const TElementDesc& arguments, const wchar_t* emWrongId, const wchar_t* emWrongType)
		{
			vint index = loggedTrace.createdElements->Keys().IndexOf(arguments.id);
			CHECK_ERROR(index != -1, emWrongId);
			CHECK_ERROR(loggedTrace.createdElements->Values()[index] == RendererType, emWrongType);
			lastElementDescs.Set(arguments.id, arguments);
		}

#define REQUEST_RENDERER_UPDATE_ELEMENT2(ARGUMENTS, RENDERER_TYPE)\
			RequestRendererUpdateElement<RENDERER_TYPE>(\
				ARGUMENTS,\
				ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.",\
				ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type."\
				)

#define REQUEST_RENDERER_UPDATE_ELEMENT(RENDERER_TYPE) REQUEST_RENDERER_UPDATE_ELEMENT2(arguments, RENDERER_TYPE)

		void RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_SolidBorder<TProtocol>::RequestRendererCreated(const ElementDesc_SolidBorder&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::SolidBorder);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_SinkBorder<TProtocol>::RequestRendererCreated(const ElementDesc_SinkBorder&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::SinkBorder);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_SinkSplitter<TProtocol>::RequestRendererCreated(const ElementDesc_SinkSplitter&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::SinkSplitter);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_SolidBackground<TProtocol>::RequestRendererCreated(const ElementDesc_SolidBackground&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::SolidBackground);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_GradientBackground<TProtocol>::RequestRendererCreated(const ElementDesc_GradientBackground&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::GradientBackground);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_InnerShadow<TProtocol>::RequestRendererCreated(const ElementDesc_InnerShadow&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::InnerShadow);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_Polygon<TProtocol>::RequestRendererCreated(const ElementDesc_Polygon&)#"
			REQUEST_RENDERER_UPDATE_ELEMENT(remoteprotocol::RendererType::Polygon);
#undef ERROR_MESSAGE_PREFIX
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - SolidLabel)
***********************************************************************/

		void CalculateSolidLabelSizeIfNecessary(vint width, vint height, const remoteprotocol::ElementDesc_SolidLabel& arguments)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_SolidLabel<TProtocol>::CalculateSolidLabelSizeIfNecessary(vint, vint, const ElementDesc_SolidLabel&)#"

			if (arguments.measuringRequest)
			{
				switch (arguments.measuringRequest.Value())
				{
				case remoteprotocol::ElementSolidLabelMeasuringRequest::FontHeight:
					CHECK_ERROR(arguments.font, ERROR_MESSAGE_PREFIX L"Font is missing for calculating font height.");
					if (!measuringForNextRendering.fontHeights)
					{
						measuringForNextRendering.fontHeights = Ptr(new collections::List<remoteprotocol::ElementMeasuring_FontHeight>);
					}
					{
						remoteprotocol::ElementMeasuring_FontHeight measuring;
						measuring.fontFamily = arguments.font.Value().fontFamily;
						measuring.fontSize = arguments.font.Value().size;
						measuring.height = measuring.fontSize + 4;
						measuringForNextRendering.fontHeights->Add(measuring);
					}
					break;
				case remoteprotocol::ElementSolidLabelMeasuringRequest::TotalSize:
					{
						// font and text has already been verified exist in RequestRendererUpdateElement_SolidLabel
						vint size = arguments.font.Value().size;
						auto text = arguments.text.Value();
						vint textWidth = 0;
						vint textHeight = 0;

						collections::List<WString> lines;
						{
							collections::List<Ptr<regex::RegexMatch>> matches;
							regexCrLf.Split(text, true, matches);

							if (matches.Count() == 0)
							{
								// when there is no text, measure a space
								lines.Add(WString::Unmanaged(L" "));
							}
							else if (arguments.multiline)
							{
								// add all lines, and if any line is empty, measure a space
								for (auto match : matches)
								{
									auto line = match->Result().Value();
									lines.Add(line.Length() ? line : WString::Unmanaged(L" "));
								}
							}
							else
							{
								lines.Add(stream::GenerateToStream([&](stream::TextWriter& writer)
								{
									for (auto [match, index] : indexed(matches))
									{
										if (index > 0) writer.WriteChar(L' ');
										auto line = match->Result().Value();
										writer.WriteString(line);
									}
								}));
								if(lines[0].Length() == 0)
								{
									// when there is no text, measure a space
									lines[0] = WString::Unmanaged(L" ");
								}
							}
						}

						if (arguments.wrapLine)
						{
							// width of the text is 0
							// insert a line break when there is no space horizontally
							vint totalLines = 0;
							for (auto&& line : lines)
							{
								if (line.Length() == 0)
								{
									totalLines++;
									continue;
								}

								double accumulatedWidth = 0;
								for (vint i = 0; i < line.Length(); i++)
								{
									auto c = line[i];
									auto w = (c < 128 ? 0.6 : 1) * size;
									if (accumulatedWidth + w > width)
									{
										if (accumulatedWidth == 0)
										{
											totalLines++;
										}
										else
										{
											totalLines++;
											accumulatedWidth = w;
										}
									}
									else
									{
										accumulatedWidth += w;
									}
								}
								if (accumulatedWidth > 0)
								{
									totalLines++;
								}
							}
							textHeight = 4 + size * totalLines;
						}
						else
						{
							// width of the text is width of the longest line
							textWidth = (vint)(size * From(lines)
								.Select([](const WString& line)
								{
									double sum = 0;
									for (vint i = 0; i < line.Length(); i++)
									{
										auto c = line[i];
										sum += (c < 128 ? 0.6 : 1);
									}
									return sum;
								})
								.Max());
							textHeight = 4 + size * lines.Count();
						}

						if (!measuringForNextRendering.minSizes)
						{
							measuringForNextRendering.minSizes = Ptr(new collections::List<remoteprotocol::ElementMeasuring_ElementMinSize>);
						}
						{
							remoteprotocol::ElementMeasuring_ElementMinSize measuring;
							measuring.id = arguments.id;
							measuring.minSize = { textWidth,textHeight };
							measuringForNextRendering.minSizes->Add(measuring);
						}
					}
					break;
				default:
					CHECK_FAIL(L"Unknown value of ElementSolidLabelMeasuringRequest.");
				}
			}
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_SolidLabel(const remoteprotocol::ElementDesc_SolidLabel& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_SolidLabel<TProtocol>::RequestRendererCreated(const ElementDesc_SolidLabel&)#"
			auto element = arguments;
			if (!element.font || !element.text)
			{
				vint index = loggedTrace.createdElements->Keys().IndexOf(element.id);
				CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"Renderer with the specified id has not been created.");

				auto rendererType = loggedTrace.createdElements->Values()[index];
				CHECK_ERROR(rendererType == remoteprotocol::RendererType::SolidLabel, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");

				index = lastElementDescs.Keys().IndexOf(arguments.id);
				if (index != -1)
				{
					auto solidLabel = lastElementDescs.Values()[index].TryGet<remoteprotocol::ElementDesc_SolidLabel>();
					CHECK_ERROR(solidLabel, ERROR_MESSAGE_PREFIX L"Renderer with the specified id is not of the expected type.");
					if (!element.font) element.font = solidLabel->font;
					if (!element.text) element.text = solidLabel->text;
				}
				else
				{
					if (!element.font) element.font = FontProperties();
					if (!element.text) element.text = WString::Empty;
				}
			}
			REQUEST_RENDERER_UPDATE_ELEMENT2(element, remoteprotocol::RendererType::SolidLabel);
#undef ERROR_MESSAGE_PREFIX
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - Image)
***********************************************************************/

		WString GetBinaryKeyFromBinary(stream::IStream& binary)
		{
			stream::MemoryStream base64WStringStream;
			{
				stream::UtfGeneralEncoder<wchar_t, char8_t> utf8ToWCharEncoder;
				stream::EncoderStream utf8ToWCharStream(base64WStringStream, utf8ToWCharEncoder);
				stream::Utf8Base64Encoder binaryToBase64Utf8Encoder;
				stream::EncoderStream binaryToBase64Utf8Stream(utf8ToWCharStream, binaryToBase64Utf8Encoder);
				binary.SeekFromBegin(0);
				stream::CopyStream(binary, binaryToBase64Utf8Stream);
			}
			{
				base64WStringStream.SeekFromBegin(0);
				stream::StreamReader reader(base64WStringStream);
				return reader.ReadToEnd();
			}
		}

		WString GetBinaryKeyFromImage(Ptr<INativeImage> image)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::GetBinaryKeyFromImage(Ptr<INativeImage>)#"
			auto remoteImage = image.Cast<GuiRemoteGraphicsImage>();
			CHECK_ERROR(remoteImage, ERROR_MESSAGE_PREFIX L"The image object must be GuiRemoteGraphicsImage.");
			return GetBinaryKeyFromBinary(remoteImage->GetBinaryData());
#undef ERROR_MESSAGE_PREFIX
		}

		remoteprotocol::ImageMetadata MakeImageMetadata(const remoteprotocol::ImageCreation& arguments)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::MakeImageMetadata(const remoteprotocol::ImageCreation)#"
			if (!cachedImageMetadatas)
			{
				cachedImageMetadatas = Ptr(new Base64ToImageMetadataMap);
				for (auto resource : GetResourceManager()->GetLoadedResources())
				{
					if (auto xmlImageData = resource->GetValueByPath(WString::Unmanaged(L"UnitTestConfig/ImageData")).Cast<glr::xml::XmlDocument>())
					{
						for (auto elementImage : glr::xml::XmlGetElements(xmlImageData->rootElement, WString::Unmanaged(L"Image")))
						{
							WString path, format, frames = WString::Unmanaged(L"1"), width, height;

							auto attPath = glr::xml::XmlGetAttribute(elementImage.Obj(), WString::Unmanaged(L"Path"));
							auto attFormat = glr::xml::XmlGetAttribute(elementImage.Obj(), WString::Unmanaged(L"Format"));
							auto attFrames = glr::xml::XmlGetAttribute(elementImage.Obj(), WString::Unmanaged(L"Frames"));
							auto attWidth = glr::xml::XmlGetAttribute(elementImage.Obj(), WString::Unmanaged(L"Width"));
							auto attHeight = glr::xml::XmlGetAttribute(elementImage.Obj(), WString::Unmanaged(L"Height"));

							CHECK_ERROR(attPath, ERROR_MESSAGE_PREFIX L"Missing Path attribute in Image element in an UnitTestConfig/ImageData.");
							CHECK_ERROR(attFormat, ERROR_MESSAGE_PREFIX L"Missing Format attribute in Image element in an UnitTestConfig/ImageData.");
							CHECK_ERROR(attWidth, ERROR_MESSAGE_PREFIX L"Missing Width attribute in Image element in an UnitTestConfig/ImageData.");
							CHECK_ERROR(attHeight, ERROR_MESSAGE_PREFIX L"Missing Height attribute in Image element in an UnitTestConfig/ImageData.");

							path = attPath->value.value;
							format = attFormat->value.value;
							width = attWidth->value.value;
							height = attHeight->value.value;
							if (attFrames) frames = attFrames->value.value;

							vint valueFrames = wtoi(frames);
							vint valueWidth = wtoi(width);
							vint valueHeight = wtoi(height);

							CHECK_ERROR(itow(valueFrames) == frames, ERROR_MESSAGE_PREFIX L"Frames attribute must be an integer in Image element in an UnitTestConfig/ImageData.");
							CHECK_ERROR(itow(valueWidth) == width, ERROR_MESSAGE_PREFIX L"Width attribute must be an integer in Image element in an UnitTestConfig/ImageData.");
							CHECK_ERROR(itow(valueHeight) == height, ERROR_MESSAGE_PREFIX L"Height attribute must be an integer in Image element in an UnitTestConfig/ImageData.");

							auto imageData = resource->GetImageByPath(path);
							WString binaryKey = GetBinaryKeyFromImage(imageData->GetImage());

							if (!cachedImageMetadatas->Keys().Contains(binaryKey))
							{
								remoteprotocol::ImageMetadata imageMetadata;
								imageMetadata.id = -1;
								imageMetadata.frames = Ptr(new collections::List<remoteprotocol::ImageFrameMetadata>);
								{
									auto node = Ptr(new glr::json::JsonString);
									node->content.value = format;
									remoteprotocol::ConvertJsonToCustomType(node, imageMetadata.format);
								}
								for (vint frame = 0; frame < valueFrames; frame++)
								{
									imageMetadata.frames->Add({ {valueWidth,valueHeight} });
								}

								cachedImageMetadatas->Add(binaryKey, imageMetadata);
							}
						}
					}
				}
			}

			auto binaryKey = GetBinaryKeyFromBinary(*arguments.imageData.Obj());
			vint binaryIndex = cachedImageMetadatas->Keys().IndexOf(binaryKey);
			CHECK_ERROR(binaryIndex != -1, ERROR_MESSAGE_PREFIX L"The image is not registered in any UnitTestConfig/ImageData.");
			auto metadata = cachedImageMetadatas->Values()[binaryIndex];
			metadata.id = arguments.id;

			loggedTrace.imageCreations->Add(arguments);
			loggedTrace.imageMetadatas->Add(metadata);
			return metadata;
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestImageCreated(vint id, const remoteprotocol::ImageCreation& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestImageCreated(vint, const vint&)#"
			CHECK_ERROR(!loggedTrace.imageMetadatas->Keys().Contains(arguments.id), ERROR_MESSAGE_PREFIX L"Image with the specified id has been created or used.");
			this->GetEvents()->RespondImageCreated(id, MakeImageMetadata(arguments));
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestImageDestroyed(const vint& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Rendering<TProtocol>::RequestImageDestroyed(const vint&)#"
			CHECK_ERROR(loggedTrace.imageMetadatas->Keys().Contains(arguments), ERROR_MESSAGE_PREFIX L"Image with the specified id has not been created.");
			CHECK_ERROR(!removedImageIds.Contains(arguments), ERROR_MESSAGE_PREFIX L"Image with the specified id has been destroyed.");
			removedImageIds.Add(arguments);
#undef ERROR_MESSAGE_PREFIX
		}

		void RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments) override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::RequestRendererUpdateElement_ImageFrame<TProtocol>::RequestRendererCreated(const ElementDesc_ImageFrame&)#"
			if (arguments.imageCreation)
			{
				auto&& imageCreation = arguments.imageCreation.Value();
				if (!imageCreation.imageDataOmitted)
				{
					CHECK_ERROR(arguments.imageId && arguments.imageId.Value() != !imageCreation.id, ERROR_MESSAGE_PREFIX L"It should satisfy that (arguments.imageId.Value()id == imageCreation.id).");
					CHECK_ERROR(!loggedTrace.imageMetadatas->Keys().Contains(imageCreation.id), ERROR_MESSAGE_PREFIX L"Image with the specified id has been created.");
					CHECK_ERROR(imageCreation.imageData, ERROR_MESSAGE_PREFIX L"When imageDataOmitted == false, imageData should not be null.");
					if (!measuringForNextRendering.createdImages)
					{
						measuringForNextRendering.createdImages = Ptr(new collections::List<remoteprotocol::ImageMetadata>);
					}
					measuringForNextRendering.createdImages->Add(MakeImageMetadata(imageCreation));
				}
				else
				{
					CHECK_ERROR(!imageCreation.imageData, ERROR_MESSAGE_PREFIX L"When imageDataOmitted == true, imageData should be null.");
				}
			}
			else if (arguments.imageId)
			{
				CHECK_ERROR(loggedTrace.imageMetadatas->Keys().Contains(arguments.imageId.Value()), ERROR_MESSAGE_PREFIX L"Image with the specified id has not been created.");
			}

			auto element = arguments;
			element.imageCreation.Reset();
			REQUEST_RENDERER_UPDATE_ELEMENT2(element, remoteprotocol::RendererType::ImageFrame);
#undef ERROR_MESSAGE_PREFIX
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Elements - Document)
***********************************************************************/

		void RequestRendererUpdateElement_DocumentParagraph(vint id, const remoteprotocol::ElementDesc_DocumentParagraph& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_GetCaret(vint id, const remoteprotocol::GetCaretRequest& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_GetCaretBounds(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_GetInlineObjectFromPoint(vint id, const Point& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_GetNearestCaretFromTextPos(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_IsValidCaret(vint id, const vint& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_OpenCaret(const remoteprotocol::OpenCaretRequest& arguments) override
		{
			CHECK_FAIL(L"Not implemented.");
		}

		void RequestDocumentParagraph_CloseCaret() override
		{
			CHECK_FAIL(L"Not implemented.");
		}

#undef REQUEST_RENDERER_UPDATE_ELEMENT
#undef REQUEST_RENDERER_UPDATE_ELEMENT2
	};
}

#endif