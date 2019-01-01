#include "GuiDocument.h"
#include "GuiParserManager.h"
#include "GuiResourceManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;
		using namespace collections;
		using namespace parsing;
		using namespace parsing::xml;
		using namespace stream;
		using namespace filesystem;

		WString GetFolderPath(const WString& filePath)
		{
			auto path = FilePath(filePath).GetFolder().GetFullPath();
			if (path != L"")
			{
				if (path[path.Length() - 1] != FilePath::Delimiter)
				{
					path += FilePath::Delimiter;
				}
			}
			return path;
		}

		WString GetFileName(const WString& filePath)
		{
			return FilePath(filePath).GetName();
		}

		bool LoadTextFile(const WString& filePath, WString& text)
		{
			BomEncoder::Encoding encoding;
			bool bom;
			return File(filePath).ReadAllTextWithEncodingTesting(text, encoding, bom);
		}

		bool IsResourceUrl(const WString& text, WString& protocol, WString& path)
		{
			Pair<vint, vint> index = INVLOC.FindFirst(text, L"://", Locale::None);
			if (index.key != -1)
			{
				protocol = INVLOC.ToLower(text.Sub(0, index.key));
				path = text.Sub(index.key + index.value, text.Length() - index.key - index.value);
				return true;
			}
			else
			{
				return false;
			}
		}

		vint HexToInt(wchar_t c)
		{
			if (L'0' <= c && c <= L'9')
			{
				return c - L'0';
			}
			else if (L'A' <= c && c <= L'F')
			{
				return c - L'A' + 10;
			}
			else
			{
				return 0;
			}
		}

		void HexToBinary(stream::IStream& stream, const WString& hexText)
		{
			const wchar_t* buffer = hexText.Buffer();
			vint count = hexText.Length() / 2;
			for (vint i = 0; i < count; i++)
			{
				vuint8_t byte = (vuint8_t)(HexToInt(buffer[0]) * 16 + HexToInt(buffer[1]));
				buffer += 2;
				stream.Write(&byte, 1);
			}
		}

		WString BinaryToHex(stream::IStream& stream)
		{
			stream::MemoryStream memoryStream;
			{
				stream::StreamWriter writer(memoryStream);
				vuint8_t byte;
				while (stream.Read(&byte, 1) == 1)
				{
					writer.WriteChar(L"0123456789ABCDEF"[byte / 16]);
					writer.WriteChar(L"0123456789ABCDEF"[byte % 16]);
				}
			}
			memoryStream.SeekFromBegin(0);
			{
				stream::StreamReader reader(memoryStream);
				return reader.ReadToEnd();
			}
		}

/***********************************************************************
GlobalStringKey
***********************************************************************/

		GlobalStringKey GlobalStringKey::Empty;
		GlobalStringKey GlobalStringKey::_InferType;
		GlobalStringKey GlobalStringKey::_Set;
		GlobalStringKey GlobalStringKey::_Ref;
		GlobalStringKey GlobalStringKey::_Bind;
		GlobalStringKey GlobalStringKey::_Format;
		GlobalStringKey GlobalStringKey::_Str;
		GlobalStringKey GlobalStringKey::_Eval;
		GlobalStringKey GlobalStringKey::_Uri;
		GlobalStringKey GlobalStringKey::_ControlTemplate;

		class GlobalStringKeyManager
		{
		public:
			Dictionary<WString, vint>		stoi;
			List<WString>					itos;

			void InitializeConstants()
			{
				GlobalStringKey::_Set = GlobalStringKey::Get(L"set");
				GlobalStringKey::_InferType = GlobalStringKey::Get(L"_");
				GlobalStringKey::_Ref = GlobalStringKey::Get(L"ref");
				GlobalStringKey::_Bind = GlobalStringKey::Get(L"bind");
				GlobalStringKey::_Format = GlobalStringKey::Get(L"format");
				GlobalStringKey::_Str = GlobalStringKey::Get(L"str");
				GlobalStringKey::_Eval = GlobalStringKey::Get(L"eval");
				GlobalStringKey::_Uri = GlobalStringKey::Get(L"uri");
				GlobalStringKey::_ControlTemplate = GlobalStringKey::Get(L"ControlTemplate");
			}
		}*globalStringKeyManager = 0;

		GlobalStringKey GlobalStringKey::Get(const WString& string)
		{
			GlobalStringKey key;
			if (string != L"")
			{
				vint index = globalStringKeyManager->stoi.Keys().IndexOf(string);
				if (index == -1)
				{
					key.key = globalStringKeyManager->itos.Add(string);
					globalStringKeyManager->stoi.Add(string, key.key);
				}
				else
				{
					key.key = globalStringKeyManager->stoi.Values()[index];
				}
			}
			return key;
		}

		vint GlobalStringKey::ToKey()const
		{
			return key;
		}

		WString GlobalStringKey::ToString()const
		{
			return *this == GlobalStringKey::Empty
				? L""
				: globalStringKeyManager->itos[key];
		}

/***********************************************************************
GuiImageData
***********************************************************************/

		GuiImageData::GuiImageData()
			:frameIndex(-1)
		{
		}

		GuiImageData::GuiImageData(Ptr<INativeImage> _image, vint _frameIndex)
			: image(_image)
			, frameIndex(_frameIndex)
		{
		}

		GuiImageData::~GuiImageData()
		{
		}

		Ptr<INativeImage> GuiImageData::GetImage()
		{
			return image;
		}

		vint GuiImageData::GetFrameIndex()
		{
			return frameIndex;
		}

/***********************************************************************
GuiTextData
***********************************************************************/

		GuiTextData::GuiTextData()
		{
		}

		GuiTextData::GuiTextData(const WString& _text)
			:text(_text)
		{
		}

		WString GuiTextData::GetText()
		{
			return text;
		}

/***********************************************************************
GuiResourceNodeBase
***********************************************************************/

		GuiResourceNodeBase::GuiResourceNodeBase()
			:parent(0)
		{
		}

		GuiResourceNodeBase::~GuiResourceNodeBase()
		{
		}

		const WString& GuiResourceNodeBase::GetName()
		{
			return name;
		}

		WString GuiResourceNodeBase::GetResourcePath()
		{
			auto resourcePath = name;
			auto current = parent;
			while (current && current->GetParent())
			{
				resourcePath = current->GetName() + L"/" + resourcePath;
				current = current->GetParent();
			}
			return resourcePath;
		}

		const WString& GuiResourceNodeBase::GetFileContentPath()
		{
			return fileContentPath;
		}

		const WString& GuiResourceNodeBase::GetFileAbsolutePath()
		{
			return fileAbsolutePath;
		}

		void GuiResourceNodeBase::SetFileContentPath(const WString& content, const WString& absolute)
		{
			fileContentPath = content;
			fileAbsolutePath = absolute;
		}

		GuiResourceFolder* GuiResourceNodeBase::GetParent()
		{
			return parent;
		}

/***********************************************************************
GuiResourceLocation
***********************************************************************/

		GuiResourceLocation::GuiResourceLocation(const WString& _resourcePath, const WString& _filePath)
			:resourcePath(_resourcePath)
			, filePath(_filePath)
		{
		}

		GuiResourceLocation::GuiResourceLocation(Ptr<GuiResourceNodeBase> node)
		{
			if (node)
			{
				resourcePath = node->GetResourcePath();

				auto current = node.Obj();
				while (current)
				{
					if (current->GetFileContentPath() != L"")
					{
						filePath = current->GetFileAbsolutePath();
						break;
					}
					current = current->GetParent();
				}
			}
		}

/***********************************************************************
GuiResourceTextPos
***********************************************************************/

		GuiResourceTextPos::GuiResourceTextPos(GuiResourceLocation location, parsing::ParsingTextPos position)
			:originalLocation(location)
			, row(position.row)
			, column(position.column)
		{
		}

/***********************************************************************
GuiResourceError
***********************************************************************/

		GuiResourceError::GuiResourceError(GuiResourceTextPos _position, const WString& _message)
			:location(_position.originalLocation)
			, position(_position)
			, message(_message)
		{
		}

		GuiResourceError::GuiResourceError(GuiResourceLocation _location, const WString& _message)
			:location(_location)
			, position(_location, {})
			, message(_message)
		{
		}

		GuiResourceError::GuiResourceError(GuiResourceLocation _location, GuiResourceTextPos _position, const WString& _message)
			:location(_location)
			, position(_position)
			, message(_message)
		{
		}

		template<typename TCallback>
		void TransformErrors(GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors, GuiResourceTextPos offset, const TCallback& callback)
		{
			if (offset.row < 0 || offset.column < 0)
			{
				offset.row = 0;
				offset.column = 0;
			}

			FOREACH(Ptr<ParsingError>, error, parsingErrors)
			{
				auto pos = error->codeRange.start;
				if (pos.row < 0 || pos.column < 0)
				{
					pos = { offset.row,offset.column };
				}
				else
				{
					if (pos.row == 0)
					{
						pos.column += offset.column;
					}
					pos.row += offset.row;
				}
				errors.Add(callback({ offset.originalLocation,pos }, error->errorMessage));
			}
		}

		void GuiResourceError::Transform(GuiResourceLocation _location, GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors)
		{
			Transform(_location, errors, parsingErrors, { _location,{ 0,0 } });
		}

		void GuiResourceError::Transform(GuiResourceLocation _location, GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors, parsing::ParsingTextPos offset)
		{
			Transform(_location, errors, parsingErrors, { _location,offset });
		}

		void GuiResourceError::Transform(GuiResourceLocation _location, GuiResourceError::List& errors, collections::List<Ptr<parsing::ParsingError>>& parsingErrors, GuiResourceTextPos offset)
		{
			TransformErrors(errors, parsingErrors, offset, [&](GuiResourceTextPos pos, const WString& message)
			{
				return GuiResourceError(_location, pos, message);
			});
		}

		void GuiResourceError::SortAndLog(List& errors, collections::List<WString>& output, const WString& workingDirectory)
		{
			if (errors.Count() == 0) return;
			SortLambda(&errors[0], errors.Count(), [](const GuiResourceError& a, const GuiResourceError& b)
			{
				vint result = 0;
				if (result == 0) result = WString::Compare(a.location.resourcePath, b.location.resourcePath);
				if (result == 0) result = WString::Compare(a.location.filePath, b.location.filePath);
				if (result == 0) result = WString::Compare(a.position.originalLocation.resourcePath, b.position.originalLocation.resourcePath);
				if (result == 0) result = WString::Compare(a.position.originalLocation.filePath, b.position.originalLocation.filePath);
				if (result == 0) result = a.position.row - b.position.row;
				if (result == 0) result = a.position.column - b.position.column;
				return result;
			});

			FOREACH_INDEXER(GuiResourceError, error, index, errors)
			{
				bool needHeader = index == 0;
				if (index > 0)
				{
					auto previousError = errors[index - 1];
					if (error.location != previousError.location || error.position.originalLocation != previousError.position.originalLocation)
					{
						needHeader = true;
					}
				}

#define CONVERT_FILEPATH(FILEPATH) (workingDirectory == L"" ? FILEPATH  : filesystem::FilePath(workingDirectory).GetRelativePathFor(FILEPATH))
#define CONVERT_LOCATION(LOCATION) (LOCATION).resourcePath + L" # " + CONVERT_FILEPATH((LOCATION).filePath)
				if (needHeader)
				{
					output.Add(CONVERT_LOCATION(error.location));
					if (error.location != error.position.originalLocation)
					{
						output.Add(L"    Original: " + CONVERT_LOCATION(error.position.originalLocation));
					}
				}

				WString prefix = L"Failed to load file \"";
				WString postfix = L"\".";
				if (INVLOC.StartsWith(error.message, prefix, Locale::Normalization::None) && INVLOC.EndsWith(error.message, postfix, Locale::Normalization::None))
				{
					auto path = error.message.Sub(prefix.Length(), error.message.Length() - prefix.Length() - postfix.Length());
					path = CONVERT_FILEPATH(path);
					error.message = prefix + path + postfix;
				}

				auto row = error.position.row;
				if (row >= 0) row++;
				auto column = error.position.column;
				if (column >= 0) column++;
				output.Add(L"(" + itow(row) + L", " + itow(column) + L"): " + error.message);
#undef CONVERT_FILEPATH
#undef CONVERT_LOCATION
			}
		}

/***********************************************************************
GuiResourceItem
***********************************************************************/

		GuiResourceItem::GuiResourceItem()
		{
		}

		GuiResourceItem::~GuiResourceItem()
		{
		}

		const WString& GuiResourceItem::GetTypeName()
		{
			return typeName;
		}

		Ptr<DescriptableObject> GuiResourceItem::GetContent()
		{
			return content;
		}

		void GuiResourceItem::SetContent(const WString& _typeName, Ptr<DescriptableObject> value)
		{
			typeName = _typeName;
			content = value;
		}

		Ptr<GuiImageData> GuiResourceItem::AsImage()
		{
			return content.Cast<GuiImageData>();
		}

		Ptr<parsing::xml::XmlDocument> GuiResourceItem::AsXml()
		{
			return content.Cast<XmlDocument>();
		}

		Ptr<GuiTextData> GuiResourceItem::AsString()
		{
			return content.Cast<GuiTextData>();
		}

		Ptr<DocumentModel> GuiResourceItem::AsDocument()
		{
			return content.Cast<DocumentModel>();
		}

/***********************************************************************
GuiResourceFolder
***********************************************************************/

		void GuiResourceFolder::LoadResourceFolderFromXml(DelayLoadingList& delayLoadings, const WString& containingFolder, Ptr<parsing::xml::XmlElement> folderXml, GuiResourceError::List& errors)
		{
			ClearItems();
			ClearFolders();
			FOREACH(Ptr<XmlElement>, element, XmlGetElements(folderXml))
			{
				WString name;
				if (Ptr<XmlAttribute> nameAtt = XmlGetAttribute(element, L"name"))
				{
					name = nameAtt->value.value;
				}
				if (element->name.value == L"Folder")
				{
					if (name == L"")
					{
						errors.Add(GuiResourceError({ {this},element->codeRange.start }, L"A resource folder should have a name."));
					}
					else
					{
						Ptr<GuiResourceFolder> folder = new GuiResourceFolder;
						if (AddFolder(name, folder))
						{
							WString newContainingFolder = containingFolder;
							Ptr<XmlElement> newFolderXml = element;
							if (Ptr<XmlAttribute> contentAtt = XmlGetAttribute(element, L"content"))
							{
								if (contentAtt->value.value == L"Link")
								{
									auto fileContentPath = XmlGetValue(element);
									auto fileAbsolutePath = containingFolder + fileContentPath;
									folder->SetFileContentPath(fileContentPath, fileAbsolutePath);

									WString text;
									if (LoadTextFile(fileAbsolutePath, text))
									{
										if (auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML"))
										{
											if (auto xml = parser->Parse({ WString::Empty,fileAbsolutePath }, text, errors))
											{
												newContainingFolder = GetFolderPath(fileAbsolutePath);
												newFolderXml = xml->rootElement;
											}
										}
									}
									else
									{
										errors.Add(GuiResourceError({ {this},element->codeRange.start }, L"Failed to load file \"" + fileAbsolutePath + L"\"."));
									}
								}
								else if (contentAtt->value.value == L"Import")
								{
									auto importUri = XmlGetValue(element);
									folder->ImportFromUri(importUri, { { this },element->codeRange.start }, errors);
								}
								else
								{
									errors.Add(GuiResourceError({ { this },element->codeRange.start }, L"Folder's content attributes can only be \"Link\"."));
								}
							}
							if (folder->GetImportUri() == L"")
							{
								folder->LoadResourceFolderFromXml(delayLoadings, newContainingFolder, newFolderXml, errors);
							}
						}
						else
						{
							errors.Add(GuiResourceError({ {this},element->codeRange.start }, L"Duplicated resource folder name \"" + name + L"\"."));
						}
					}
				}
				else if (element->name.value.Length() <= 3 || element->name.value.Sub(0, 4) != L"ref.")
				{
					WString fileContentPath;
					WString fileAbsolutePath;
					if (Ptr<XmlAttribute> contentAtt = XmlGetAttribute(element, L"content"))
					{
						if (contentAtt->value.value == L"File")
						{
							fileContentPath = XmlGetValue(element);
							fileAbsolutePath = containingFolder + fileContentPath;
							if (name == L"")
							{
								name = GetFileName(fileAbsolutePath);
							}
						}
						else
						{
							errors.Add(GuiResourceError({ { this },element->codeRange.start }, L"File's content attributes can only be \"File\"."));
						}
					}

					Ptr<GuiResourceItem> item = new GuiResourceItem;
					if (AddItem(name, item))
					{
						WString type = element->name.value;
						IGuiResourceTypeResolver* typeResolver = GetResourceResolverManager()->GetTypeResolver(type);
						IGuiResourceTypeResolver* preloadResolver = typeResolver;

						if (typeResolver)
						{
							if (!typeResolver->DirectLoadXml())
							{
								WString preloadType = typeResolver->IndirectLoad()->GetPreloadType();
								preloadResolver = GetResourceResolverManager()->GetTypeResolver(preloadType);
								if (!preloadResolver)
								{
									errors.Add(GuiResourceError({ {this}, element->codeRange.start }, L"[INTERNAL-ERROR] Unknown resource resolver \"" + preloadType + L"\" of resource type \"" + type + L"\"."));
								}
							}
						}
						else
						{
							errors.Add(GuiResourceError({ {this}, element->codeRange.start }, L"Unknown resource type \"" + type + L"\"."));
						}

						if (typeResolver && preloadResolver)
						{
							if (auto directLoad = preloadResolver->DirectLoadXml())
							{
								{
									Ptr<DescriptableObject> resource;
									if (fileAbsolutePath == L"")
									{
										resource = directLoad->ResolveResource(item, element, errors);
									}
									else
									{
										item->SetFileContentPath(fileContentPath, fileAbsolutePath);
										resource = directLoad->ResolveResource(item, fileAbsolutePath, errors);
									}
									item->SetContent(preloadResolver->GetType(), resource);
								}

								if (typeResolver != preloadResolver)
								{
									if (auto indirectLoad = typeResolver->IndirectLoad())
									{
										if (indirectLoad->IsDelayLoad())
										{
											DelayLoading delayLoading;
											delayLoading.type = type;
											delayLoading.workingDirectory = containingFolder;
											delayLoading.preloadResource = item;
											delayLoadings.Add(delayLoading);
										}
										else if (item->GetContent())
										{
											auto resource = indirectLoad->ResolveResource(item, 0, errors);
											item->SetContent(typeResolver->GetType(), resource);
										}
									}
									else
									{
										item->SetContent(typeResolver->GetType(), nullptr);
										errors.Add(GuiResourceError({ {this},element->codeRange.start }, L"[INTERNAL-ERROR] Resource type \"" + typeResolver->GetType() + L"\" is not a indirect load resource type."));									}
								}
							}
							else
							{
								errors.Add(GuiResourceError({ {this},element->codeRange.start }, L"[INTERNAL-ERROR] Resource type \"" + preloadResolver->GetType() + L"\" is not a direct load resource type."));
							}
						}

						if (!item->GetContent())
						{
							RemoveItem(name);
						}
					}
					else
					{
						errors.Add(GuiResourceError({ {this},element->codeRange.start }, L"Duplicated resource item name \"" + name + L"\"."));
					}
				}
			}
		}

		void GuiResourceFolder::SaveResourceFolderToXml(Ptr<parsing::xml::XmlElement> xmlParent)
		{
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto resolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
				if (resolver->XmlSerializable())
				{
					auto attName = MakePtr<XmlAttribute>();
					attName->name.value = L"name";
					attName->value.value = item->GetName();

					if (item->GetFileContentPath() == L"")
					{
						Ptr<XmlElement> xmlElement;

						if (auto directLoad = resolver->DirectLoadXml())
						{
							xmlElement = directLoad->Serialize(item, item->GetContent());
						}
						else if (auto indirectLoad = resolver->IndirectLoad())
						{
							if (auto preloadResolver = GetResourceResolverManager()->GetTypeResolver(indirectLoad->GetPreloadType()))
							{
								if (auto directLoad = preloadResolver->DirectLoadXml())
								{
									if (auto resource = indirectLoad->Serialize(item, item->GetContent()))
									{
										xmlElement = directLoad->Serialize(item, resource);
										xmlElement->name.value = resolver->GetType();
									}
								}
							}
						}

						if (xmlElement)
						{
							xmlElement->attributes.Add(attName);
							xmlParent->subNodes.Add(xmlElement);
						}
					}
					else
					{
						auto xmlElement = MakePtr<XmlElement>();
						xmlElement->name.value = item->GetTypeName();
						xmlParent->subNodes.Add(xmlElement);

						auto attContent = MakePtr<XmlAttribute>();
						attContent->name.value = L"content";
						attContent->value.value = L"File";
						xmlElement->attributes.Add(attContent);

						auto xmlText = MakePtr<XmlText>();
						xmlText->content.value = item->GetFileContentPath();
						xmlElement->subNodes.Add(xmlText);
					}
				}
			}

			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				auto attName = MakePtr<XmlAttribute>();
				attName->name.value = L"name";
				attName->value.value = folder->GetName();

				auto xmlFolder = MakePtr<XmlElement>();
				xmlFolder->name.value = L"Folder";
				xmlFolder->attributes.Add(attName);
				xmlParent->subNodes.Add(xmlFolder);
				
				if (folder->GetImportUri() != L"")
				{
					auto attContent = MakePtr<XmlAttribute>();
					attContent->name.value = L"content";
					attContent->value.value = L"Import";
					xmlFolder->attributes.Add(attContent);

					auto xmlText = MakePtr<XmlText>();
					xmlText->content.value = folder->GetImportUri();
					xmlFolder->subNodes.Add(xmlText);
				}
				else if (folder->GetFileContentPath() != L"")
				{
					auto attContent = MakePtr<XmlAttribute>();
					attContent->name.value = L"content";
					attContent->value.value = L"Link";
					xmlFolder->attributes.Add(attContent);

					auto xmlText = MakePtr<XmlText>();
					xmlText->content.value = folder->GetFileContentPath();
					xmlFolder->subNodes.Add(xmlText);
				}
				else
				{
					folder->SaveResourceFolderToXml(xmlFolder);
				}
			}
		}

		void GuiResourceFolder::CollectTypeNames(collections::List<WString>& typeNames)
		{
			if (importUri != L"") return;
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				if (!typeNames.Contains(item->GetTypeName()))
				{
					typeNames.Add(item->GetTypeName());
				}
			}
			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				folder->CollectTypeNames(typeNames);
			}
		}

		void GuiResourceFolder::LoadResourceFolderFromBinary(DelayLoadingList& delayLoadings, stream::internal::ContextFreeReader& reader, collections::List<WString>& typeNames, GuiResourceError::List& errors)
		{
			vint count = 0;
			reader << count;
			for (vint i = 0; i < count; i++)
			{
				vint typeName = 0;
				WString name;
				reader << typeName << name;

				auto resolver = GetResourceResolverManager()->GetTypeResolver(typeNames[typeName]);
				Ptr<GuiResourceItem> item = new GuiResourceItem;
				if(AddItem(name, item))
				{
					WString type = typeNames[typeName];
					IGuiResourceTypeResolver* typeResolver = GetResourceResolverManager()->GetTypeResolver(type);
					IGuiResourceTypeResolver* preloadResolver = typeResolver;

					if(typeResolver)
					{
						if (!typeResolver->DirectLoadStream())
						{
							WString preloadType = typeResolver->IndirectLoad()->GetPreloadType();
							if (preloadType != L"")
							{
								preloadResolver = GetResourceResolverManager()->GetTypeResolver(preloadType);
								if (!preloadResolver)
								{
									errors.Add(GuiResourceError({ item }, L"[INTERNAL-ERROR] Unknown resource resolver \"" + preloadType + L"\" of resource type \"" + type + L"\"."));
								}
							}
						}
					}
					else
					{
						errors.Add(GuiResourceError({ item }, L"[BINARY] Unknown resource type \"" + type + L"\"."));
					}

					if(typeResolver && preloadResolver)
					{
						if (auto directLoad = preloadResolver->DirectLoadStream())
						{
							{
								auto resource = directLoad->ResolveResourcePrecompiled(item, reader.input, errors);
								item->SetContent(preloadResolver->GetType(), resource);
							}

							if (typeResolver != preloadResolver)
							{
								if (auto indirectLoad = typeResolver->IndirectLoad())
								{
									if(indirectLoad->IsDelayLoad())
									{
										DelayLoading delayLoading;
										delayLoading.type = type;
										delayLoading.preloadResource = item;
										delayLoadings.Add(delayLoading);
									}
									else if(item->GetContent())
									{
										auto resource = indirectLoad->ResolveResource(item, nullptr, errors);
										item->SetContent(typeResolver->GetType(), resource);
									}
								}
								else
								{
									item->SetContent(typeResolver->GetType(), nullptr);
									errors.Add(GuiResourceError({ item }, L"[INTERNAL-ERROR] Resource type \"" + typeResolver->GetType() + L"\" is not a indirect load resource type."));
								}
							}
						}
						else
						{
							errors.Add(GuiResourceError({ item }, L"[INTERNAL-ERROR] Resource type \"" + preloadResolver->GetType() + L"\" is not a direct load resource type."));
						}
					}

					if(!item->GetContent())
					{
						RemoveItem(name);
					}
				}
				else
				{
					errors.Add(GuiResourceError({ this }, L"[BINARY] Duplicated resource item name \"" + name + L"\"."));
				}
			}

			reader << count;
			for (vint i = 0; i < count; i++)
			{
				WString name, importUri;
				reader << name << importUri;

				auto folder = MakePtr<GuiResourceFolder>();
				if (importUri == L"")
				{
					folder->LoadResourceFolderFromBinary(delayLoadings, reader, typeNames, errors);
				}
				else
				{
					folder->ImportFromUri(importUri, { { this },{0,0} }, errors);
				}
				AddFolder(name, folder);
			}
		}

		void GuiResourceFolder::SaveResourceFolderToBinary(stream::internal::ContextFreeWriter& writer, collections::List<WString>& typeNames)
		{
			typedef Tuple<vint, WString, IGuiResourceTypeResolver_DirectLoadStream*, Ptr<GuiResourceItem>, Ptr<DescriptableObject>> ItemTuple;
			List<ItemTuple> itemTuples;

			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto resolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
				if (resolver->StreamSerializable())
				{
					vint typeName = typeNames.IndexOf(item->GetTypeName());
					WString name = item->GetName();
				
					if (auto directLoad = resolver->DirectLoadStream())
					{
						itemTuples.Add(ItemTuple(typeName, name, directLoad, item, item->GetContent()));
					}
					else if (auto indirectLoad = resolver->IndirectLoad())
					{
						if (auto preloadResolver = GetResourceResolverManager()->GetTypeResolver(indirectLoad->GetPreloadType()))
						{
							if (auto directLoad = preloadResolver->DirectLoadStream())
							{
								if (auto resource = indirectLoad->Serialize(item, item->GetContent()))
								{
									itemTuples.Add(ItemTuple(typeName, name, directLoad, item, resource));
								}
							}
						}
					}
				}
			}

			vint count = itemTuples.Count();
			writer << count;
			FOREACH(ItemTuple, item, itemTuples)
			{
				vint typeName = item.f0;
				WString name = item.f1;
				writer << typeName << name;

				auto directLoad = item.f2;
				auto resource = item.f3;
				auto content = item.f4;
				directLoad->SerializePrecompiled(resource, content, writer.output);
			}

			count = folders.Count();
			writer << count;
			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				WString name = folder->GetName();
				WString importUri = folder->GetImportUri();
				writer << name << importUri;
				if (importUri == L"")
				{
					folder->SaveResourceFolderToBinary(writer, typeNames);
				}
			}
		}

		void GuiResourceFolder::PrecompileResourceFolder(GuiResourcePrecompileContext& context, IGuiResourcePrecompileCallback* callback, GuiResourceError::List& errors)
		{
			if (importUri != L"") return;
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto typeResolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
				if (auto precompile = typeResolver->Precompile())
				{
					if (precompile->GetPassSupport(context.passIndex) == IGuiResourceTypeResolver_Precompile::PerResource)
					{
						if (callback)
						{
							callback->OnPerResource(context.passIndex, item);
						}
						precompile->PerResourcePrecompile(item, context, errors);
					}
				}
			}

			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				folder->PrecompileResourceFolder(context, callback, errors);
			}
		}

		void GuiResourceFolder::InitializeResourceFolder(GuiResourceInitializeContext& context, GuiResourceError::List& errors)
		{
			if (importUri != L"") return;
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto typeResolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
				if (auto initialize = typeResolver->Initialize())
				{
					initialize->Initialize(item, context, errors);
				}
			}

			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				folder->InitializeResourceFolder(context, errors);
			}
		}

		void GuiResourceFolder::ImportFromUri(const WString& uri, GuiResourceTextPos position, GuiResourceError::List& errors)
		{
			SetImportUri(uri);
			if (importUri.Length() == 0 || importUri[importUri.Length() - 1] != L'/')
			{
				errors.Add(GuiResourceError(position, L"Path of imported folder should ends with L\"/\"."));
			}
			else
			{
				WString protocol, path;
				if (IsResourceUrl(importUri, protocol, path))
				{
					if (protocol == L"import-res")
					{
						auto factory = GetResourceResolverManager()->GetPathResolverFactory(protocol);
						auto resolver = factory->CreateResolver(nullptr, L"");
						if (auto sourceFolder = resolver->ResolveResource(path).Cast<GuiResourceFolder>())
						{
							CopyFrom(items, sourceFolder->items);
							CopyFrom(folders, sourceFolder->folders);
						}
						else
						{
							errors.Add(GuiResourceError(position, L"Path of imported folder does not exist: \"" + importUri + L"\"."));
						}
					}
					else
					{
						errors.Add(GuiResourceError(position, L"Path of imported folder should begin with \"import-res://\"."));
					}
				}
				else
				{
					errors.Add(GuiResourceError(position, L"Invalid path of imported folder : \"" + importUri + L"\"."));
				}
			}
		}

		GuiResourceFolder::GuiResourceFolder()
		{
		}

		GuiResourceFolder::~GuiResourceFolder()
		{
		}

		const WString& GuiResourceFolder::GetImportUri()
		{
			return importUri;
		}

		void GuiResourceFolder::SetImportUri(const WString& uri)
		{
			importUri = uri;
		}

		const GuiResourceFolder::ItemList& GuiResourceFolder::GetItems()
		{
			return items.Values();
		}

		Ptr<GuiResourceItem> GuiResourceFolder::GetItem(const WString& name)
		{
			vint index=items.Keys().IndexOf(name);
			return index == -1 ? nullptr : items.Values().Get(index);
		}

		bool GuiResourceFolder::AddItem(const WString& name, Ptr<GuiResourceItem> item)
		{
			if (item->GetParent() != 0 || items.Keys().Contains(name)) return false;
			items.Add(name, item);
			item->parent = this;
			item->name = name;
			return true;
		}

		Ptr<GuiResourceItem> GuiResourceFolder::RemoveItem(const WString& name)
		{
			Ptr<GuiResourceItem> item = GetItem(name);
			if (!item) return 0;
			items.Remove(name);
			item->parent = nullptr;
			item->name = L"";
			return item;
		}

		void GuiResourceFolder::ClearItems()
		{
			items.Clear();
		}

		const GuiResourceFolder::FolderList& GuiResourceFolder::GetFolders()
		{
			return folders.Values();
		}

		Ptr<GuiResourceFolder> GuiResourceFolder::GetFolder(const WString& name)
		{
			vint index=folders.Keys().IndexOf(name);
			return index == -1 ? nullptr : folders.Values().Get(index);
		}

		bool GuiResourceFolder::AddFolder(const WString& name, Ptr<GuiResourceFolder> folder)
		{
			if (folder->GetParent() != 0 || folders.Keys().Contains(name)) return false;
			folders.Add(name, folder);
			folder->parent = this;
			folder->name = name;
			return true;
		}

		Ptr<GuiResourceFolder> GuiResourceFolder::RemoveFolder(const WString& name)
		{
			Ptr<GuiResourceFolder> folder = GetFolder(name);
			if (!folder) return 0;
			folders.Remove(name);
			folder->parent = nullptr;
			folder->name = L"";
			return folder;
		}

		void GuiResourceFolder::ClearFolders()
		{
			folders.Clear();
		}

		Ptr<DescriptableObject> GuiResourceFolder::GetValueByPath(const WString& path)
		{
			const wchar_t* buffer=path.Buffer();
			const wchar_t* index=wcschr(buffer, L'\\');
			if(!index) index=wcschr(buffer, '/');

			if(index)
			{
				WString name=path.Sub(0, index-buffer);
				Ptr<GuiResourceFolder> folder=GetFolder(name);
				if(folder)
				{
					vint start=index-buffer+1;
					return folder->GetValueByPath(path.Sub(start, path.Length()-start));
				}
			}
			else
			{
				Ptr<GuiResourceItem> item=GetItem(path);
				if(item)
				{
					return item->GetContent();
				}
			}
			return 0;
		}

		Ptr<GuiResourceFolder> GuiResourceFolder::GetFolderByPath(const WString& path)
		{
			const wchar_t* buffer=path.Buffer();
			const wchar_t* index=wcschr(buffer, L'\\');
			if(!index) index=wcschr(buffer, '/');
			if(!index) return 0;

			WString name=path.Sub(0, index-buffer);
			Ptr<GuiResourceFolder> folder=GetFolder(name);

			if(index-buffer==path.Length()-1)
			{
				return folder;
			}

			if(folder)
			{
				vint start=index-buffer+1;
				return folder->GetFolderByPath(path.Sub(start, path.Length()-start));
			}

			return 0;
		}

		bool GuiResourceFolder::CreateValueByPath(const WString& path, const WString& typeName, Ptr<DescriptableObject> value)
		{
			const wchar_t* buffer = path.Buffer();
			const wchar_t* index = wcschr(buffer, L'\\');
			if (!index) index = wcschr(buffer, '/');

			if(index)
			{
				WString name = path.Sub(0, index - buffer);
				Ptr<GuiResourceFolder> folder = GetFolder(name);
				if (!folder)
				{
					folder = new GuiResourceFolder;
					AddFolder(name, folder);
				}
				vint start = index - buffer + 1;
				return folder->CreateValueByPath(path.Sub(start, path.Length() - start), typeName, value);
			}
			else
			{
				if(GetItem(path))
				{
					return false;
				}

				auto item = new GuiResourceItem;
				item->SetContent(typeName, value);
				return AddItem(path, item);
			}
		}

/***********************************************************************
GuiResourceMetadata
***********************************************************************/

		void GuiResourceMetadata::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, GuiResourceLocation location, GuiResourceError::List& errors)
		{
			auto attrName = XmlGetAttribute(xml->rootElement, L"Name");
			auto attrVersion = XmlGetAttribute(xml->rootElement, L"Version");
			if (!attrName || !attrVersion)
			{
				errors.Add(GuiResourceError(location, L"[INTERNAL-ERROR] Resource metadata lacks of Name or Version attribute."));
				return;
			}
			name = attrName->value.value;
			version = attrVersion->value.value;
			dependencies.Clear();

			if (auto xmlDeps = XmlGetElement(xml->rootElement, L"Dependencies"))
			{
				FOREACH(Ptr<XmlElement>, xmlDep, XmlGetElements(xmlDeps, L"Resource"))
				{
					auto attrDep = XmlGetAttribute(xmlDep, L"Name");
					if (!attrDep)
					{
						errors.Add(GuiResourceError(location, L"[INTERNAL-ERROR] Resource dependency lacks of Name attribute."));
					}
					dependencies.Add(attrDep->value.value);
				}
			}
		}

		Ptr<parsing::xml::XmlDocument> GuiResourceMetadata::SaveToXml()
		{
			auto root = MakePtr<XmlElement>();
			root->name.value = L"ResourceMetadata";
			{
				auto attr = MakePtr<XmlAttribute>();
				attr->name.value = L"Name";
				attr->value.value = name;
				root->attributes.Add(attr);
			}
			{
				auto attr = MakePtr<XmlAttribute>();
				attr->name.value = L"Version";
				attr->value.value = version;
				root->attributes.Add(attr);
			}
			{
				auto xmlDeps = MakePtr<XmlElement>();
				xmlDeps->name.value = L"Dependencies";
				root->subNodes.Add(xmlDeps);

				FOREACH(WString, dep, dependencies)
				{
					auto xmlDep = MakePtr<XmlElement>();
					xmlDep->name.value = L"Resource";
					xmlDeps->subNodes.Add(xmlDep);
					{
						auto attr = MakePtr<XmlAttribute>();
						attr->name.value = L"Name";
						attr->value.value = dep;
						xmlDep->attributes.Add(attr);
					}
				}
			}

			auto doc = MakePtr<XmlDocument>();
			doc->rootElement = root;
			return doc;
		}

/***********************************************************************
GuiResource
***********************************************************************/

		const wchar_t* GuiResource::CurrentVersionString = L"1.0";

		void GuiResource::ProcessDelayLoading(Ptr<GuiResource> resource, DelayLoadingList& delayLoadings, GuiResourceError::List& errors)
		{
			FOREACH(DelayLoading, delay, delayLoadings)
			{
				WString type = delay.type;
				WString folder = delay.workingDirectory;
				Ptr<GuiResourceItem> item = delay.preloadResource;

				if (auto typeResolver = GetResourceResolverManager()->GetTypeResolver(type))
				{
					if (auto indirectLoad = typeResolver->IndirectLoad())
					{
						if (item->GetContent())
						{
							Ptr<GuiResourcePathResolver> pathResolver = new GuiResourcePathResolver(resource, folder);
							Ptr<DescriptableObject> resource = indirectLoad->ResolveResource(item, pathResolver, errors);
							if (resource)
							{
								item->SetContent(typeResolver->GetType(), resource);
							}
						}
					}
					else
					{
						errors.Add(GuiResourceError({ item }, L"[INTERNAL-ERROR] Resource type \"" + type + L"\" is not a indirect load resource type."));
					}
				}
				else
				{
					errors.Add(GuiResourceError({ item }, L"[INTERNAL-ERROR] Unknown resource type \"" + type + L"\"."));
				}
			}
		}

		GuiResource::GuiResource()
		{
			metadata = MakePtr<GuiResourceMetadata>();
			metadata->version = CurrentVersionString;
		}

		GuiResource::~GuiResource()
		{
		}

		Ptr<GuiResourceMetadata> GuiResource::GetMetadata()
		{
			return metadata;
		}

		WString GuiResource::GetWorkingDirectory()
		{
			return workingDirectory;
		}

		Ptr<GuiResource> GuiResource::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, const WString& filePath, const WString& workingDirectory, GuiResourceError::List& errors)
		{
			Ptr<GuiResource> resource = new GuiResource;
			resource->SetFileContentPath(filePath, filePath);
			resource->workingDirectory = workingDirectory;
			DelayLoadingList delayLoadings;
			resource->LoadResourceFolderFromXml(delayLoadings, resource->workingDirectory, xml->rootElement, errors);

			ProcessDelayLoading(resource, delayLoadings, errors);
			return resource;
		}

		Ptr<GuiResource> GuiResource::LoadFromXml(const WString& filePath, GuiResourceError::List& errors)
		{
			Ptr<XmlDocument> xml;
			if(auto parser=GetParserManager()->GetParser<XmlDocument>(L"XML"))
			{
				WString text;
				if(LoadTextFile(filePath, text))
				{
					xml = parser->Parse({ WString::Empty,filePath }, text, errors);
				}
				else
				{
					errors.Add(GuiResourceError({ WString::Empty,filePath }, L"Failed to load file \"" + filePath + L"\"."));
				}
			}
			if(xml)
			{
				return LoadFromXml(xml, filePath, GetFolderPath(filePath), errors);
			}
			return 0;
		}

		Ptr<parsing::xml::XmlDocument> GuiResource::SaveToXml()
		{
			auto xmlRoot = MakePtr<XmlElement>();
			xmlRoot->name.value = L"Resource";
			SaveResourceFolderToXml(xmlRoot);

			auto doc = MakePtr<XmlDocument>();
			doc->rootElement = xmlRoot;
			return doc;
		}

		Ptr<GuiResource> GuiResource::LoadPrecompiledBinary(stream::IStream& stream, GuiResourceError::List& errors)
		{
			stream::internal::ContextFreeReader reader(stream);
			auto resource = MakePtr<GuiResource>();
			{
				WString metadata;
				reader << metadata;
				
				auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
				auto xmlMetadata = parser->Parse({ resource }, metadata, errors);
				if (!xmlMetadata) return nullptr;

				resource->metadata->LoadFromXml(xmlMetadata, { resource }, errors);
				if (errors.Count() != 0) return nullptr;

				if (resource->metadata->version != CurrentVersionString)
				{
					errors.Add(GuiResourceError({ resource }, L"Only resource binary of version \"" + WString(CurrentVersionString) + L"\" is accepted. Please recompile the resource before loading it."));
					return nullptr;
				}
			}

			List<WString> typeNames;
			reader << typeNames;
			
			DelayLoadingList delayLoadings;
			resource->LoadResourceFolderFromBinary(delayLoadings, reader, typeNames, errors);
			
			ProcessDelayLoading(resource, delayLoadings, errors);
			return resource;
		}

		Ptr<GuiResource> GuiResource::LoadPrecompiledBinary(stream::IStream& stream)
		{
			GuiResourceError::List errors;
			auto resource = LoadPrecompiledBinary(stream, errors);
			CHECK_ERROR(errors.Count() == 0, L"GuiResource::LoadPrecompiledBinary(IStream&)#There are errors.");
			return resource;
		}

		void GuiResource::SavePrecompiledBinary(stream::IStream& stream)
		{
			stream::internal::ContextFreeWriter writer(stream);
			{
				auto xmlMetadata = metadata->SaveToXml();
				WString xml = GenerateToStream([&](StreamWriter& writer)
				{
					XmlPrint(xmlMetadata, writer);
				});
				writer << xml;
			}
			List<WString> typeNames;
			CollectTypeNames(typeNames);
			writer << typeNames;
			SaveResourceFolderToBinary(writer, typeNames);
		}

		Ptr<GuiResourceFolder> GuiResource::Precompile(IGuiResourcePrecompileCallback* callback, GuiResourceError::List& errors)
		{
			if (GetFolder(L"Precompiled"))
			{
				errors.Add(GuiResourceError({ this }, L"A precompiled resource cannot be compiled again."));
				return nullptr;
			}

			GuiResourcePrecompileContext context;
			context.compilerCallback = callback ? callback->GetCompilerCallback() : nullptr;
			context.rootResource = this;
			context.resolver = new GuiResourcePathResolver(this, workingDirectory);
			context.targetFolder = new GuiResourceFolder;
			
			auto manager = GetResourceResolverManager();
			vint maxPass = manager->GetMaxPrecompilePassIndex();
			List<WString> resolvers;
			for (vint i = 0; i <= maxPass; i++)
			{
				context.passIndex = i;
				{
					manager->GetPerResourceResolverNames(i, resolvers);
					if (resolvers.Count() > 0)
					{
						PrecompileResourceFolder(context, callback, errors);
					}
				}
				{
					manager->GetPerPassResolverNames(i, resolvers);
					if (resolvers.Count() > 0)
					{
						if (callback)
						{
							callback->OnPerPass(i);
						}
						FOREACH(WString, name, resolvers)
						{
							auto resolver = manager->GetTypeResolver(name);
							resolver->Precompile()->PerPassPrecompile(context, errors);
						}
					}
				}
				if (errors.Count() > 0)
				{
					return context.targetFolder;
				}
			}
			AddFolder(L"Precompiled", context.targetFolder);
			return context.targetFolder;
		}

		void GuiResource::Initialize(GuiResourceUsage usage, GuiResourceError::List& errors)
		{
			auto precompiledFolder = GetFolder(L"Precompiled");
			if (!precompiledFolder)
			{
				CHECK_FAIL(L"GuiResource::Initialize()#Cannot initialize a non-precompiled resource.");
				return;
			}
			
			GuiResourceInitializeContext context;
			context.rootResource = this;
			context.resolver = new GuiResourcePathResolver(this, workingDirectory);
			context.targetFolder = precompiledFolder;
			context.usage = usage;

			vint maxPass = GetResourceResolverManager()->GetMaxInitializePassIndex();
			for (vint i = 0; i <= maxPass; i++)
			{
				context.passIndex = i;
				InitializeResourceFolder(context, errors);
			}
		}

		Ptr<DocumentModel> GuiResource::GetDocumentByPath(const WString& path)
		{
			Ptr<DocumentModel> result=GetValueByPath(path).Cast<DocumentModel>();
			if(!result) throw ArgumentException(L"Path not exists.", L"GuiResource::GetDocumentByPath", L"path");
			return result;
		}

		Ptr<GuiImageData> GuiResource::GetImageByPath(const WString& path)
		{
			Ptr<GuiImageData> result=GetValueByPath(path).Cast<GuiImageData>();
			if(!result) throw ArgumentException(L"Path not exists.", L"GuiResource::GetImageByPath", L"path");
			return result;
		}

		Ptr<parsing::xml::XmlDocument> GuiResource::GetXmlByPath(const WString& path)
		{
			Ptr<XmlDocument> result=GetValueByPath(path).Cast<XmlDocument>();
			if(!result) throw ArgumentException(L"Path not exists.", L"GuiResource::GetXmlByPath", L"path");
			return result;
		}

		WString GuiResource::GetStringByPath(const WString& path)
		{
			Ptr<ObjectBox<WString>> result=GetValueByPath(path).Cast<ObjectBox<WString>>();
			if(!result) throw ArgumentException(L"Path not exists.", L"GuiResource::GetStringByPath", L"path");
			return result->Unbox();
		}

/***********************************************************************
GuiResourcePathResolver
***********************************************************************/

		GuiResourcePathResolver::GuiResourcePathResolver(Ptr<GuiResource> _resource, const WString& _workingDirectory)
			:resource(_resource)
			,workingDirectory(_workingDirectory)
		{
		}

		GuiResourcePathResolver::~GuiResourcePathResolver()
		{
		}

		Ptr<DescriptableObject> GuiResourcePathResolver::ResolveResource(const WString& protocol, const WString& path)
		{
			Ptr<IGuiResourcePathResolver> resolver;
			vint index=resolvers.Keys().IndexOf(protocol);
			if(index==-1)
			{
				IGuiResourcePathResolverFactory* factory=GetResourceResolverManager()->GetPathResolverFactory(protocol);
				if(factory)
				{
					resolver=factory->CreateResolver(resource, workingDirectory);
				}
				resolvers.Add(protocol, resolver);
			}
			else
			{
				resolver=resolvers.Values()[index];
			}

			if(resolver)
			{
				return resolver->ResolveResource(path);
			}
			else
			{
				return 0;
			}
		}

/***********************************************************************
GuiResourcePathResResolver
***********************************************************************/

		class GuiResourcePathResResolver : public Object, public IGuiResourcePathResolver
		{
		protected:
			Ptr<GuiResource>		resource;

		public:
			GuiResourcePathResResolver(Ptr<GuiResource> _resource)
				:resource(_resource)
			{
			}

			Ptr<DescriptableObject> ResolveResource(const WString& path)
			{
				if (resource)
				{
					if (path.Length() > 0)
					{
						switch (path[path.Length() - 1])
						{
						case L'\\':case L'/':
							return resource->GetFolderByPath(path);
						default:
							return resource->GetValueByPath(path);
						}
					}
				}
				return nullptr;
			}

			class Factory : public Object, public IGuiResourcePathResolverFactory
			{
			public:
				WString GetProtocol()override
				{
					return L"res";
				}

				Ptr<IGuiResourcePathResolver> CreateResolver(Ptr<GuiResource> resource, const WString& workingDirectory)override
				{
					return new GuiResourcePathResResolver(resource);
				}
			};
		};

/***********************************************************************
GuiImportResourcePathResResolver
***********************************************************************/

		class GuiImportResourcePathResResolver : public Object, public IGuiResourcePathResolver
		{
		public:
			GuiImportResourcePathResResolver()
			{
			}

			Ptr<DescriptableObject> ResolveResource(const WString& path)
			{
				const wchar_t* buffer = path.Buffer();
				const wchar_t* d1 = wcschr(buffer, L'\\');
				const wchar_t* d2 = wcschr(buffer, L'/');
				const wchar_t* d =
					d1 == nullptr&&d2 == nullptr ? nullptr :
					d1 == nullptr ? d2 :
					d2 == nullptr ? d1 :
					d1 < d2 ? d1 : d2;

				if (!d) return nullptr;
				WString resourceName(buffer, d - buffer);
				WString resourcePath(path.Right(path.Length() - resourceName.Length() - 1));
				if (auto resource = GetResourceManager()->GetResource(resourceName))
				{
					switch (path[path.Length() - 1])
					{
					case L'\\':case L'/':
						return resource->GetFolderByPath(resourcePath);
					default:
						return resource->GetValueByPath(resourcePath);
					}
				}
				return nullptr;
			}

			class Factory : public Object, public IGuiResourcePathResolverFactory
			{
			public:
				WString GetProtocol()override
				{
					return L"import-res";
				}

				Ptr<IGuiResourcePathResolver> CreateResolver(Ptr<GuiResource> resource, const WString& workingDirectory)override
				{
					return new GuiImportResourcePathResResolver;
				}
			};
		};

/***********************************************************************
IGuiResourceResolverManager
***********************************************************************/

		IGuiResourceResolverManager* resourceResolverManager=0;

		IGuiResourceResolverManager* GetResourceResolverManager()
		{
			return resourceResolverManager;
		}

		class GuiResourceResolverManager : public Object, public IGuiResourceResolverManager, public IGuiPlugin
		{
			typedef Dictionary<WString, Ptr<IGuiResourcePathResolverFactory>>			PathFactoryMap;
			typedef Dictionary<WString, Ptr<IGuiResourceTypeResolver>>					TypeResolverMap;
			typedef Group<vint, WString>												ResolverGroup;
		protected:
			PathFactoryMap				pathFactories;
			TypeResolverMap				typeResolvers;
			ResolverGroup				perResourceResolvers;
			ResolverGroup				perPassResolvers;

		public:
			
			GUI_PLUGIN_NAME(GacUI_Res_ResourceResolver)
			{
			}

			void Load()override
			{
				globalStringKeyManager = new GlobalStringKeyManager();
				globalStringKeyManager->InitializeConstants();

				resourceResolverManager = this;
				SetPathResolverFactory(new GuiResourcePathResResolver::Factory);
				SetPathResolverFactory(new GuiImportResourcePathResResolver::Factory);
			}

			void Unload()override
			{
				delete globalStringKeyManager;
				globalStringKeyManager = 0;
				resourceResolverManager = 0;
			}

			IGuiResourcePathResolverFactory* GetPathResolverFactory(const WString& protocol)override
			{
				vint index=pathFactories.Keys().IndexOf(protocol);
				return index==-1?0:pathFactories.Values()[index].Obj();
			}

			bool SetPathResolverFactory(Ptr<IGuiResourcePathResolverFactory> factory)override
			{
				if(pathFactories.Keys().Contains(factory->GetProtocol())) return false;
				pathFactories.Add(factory->GetProtocol(), factory);
				return true;
			}

			IGuiResourceTypeResolver* GetTypeResolver(const WString& type)override
			{
				vint index=typeResolvers.Keys().IndexOf(type);
				return index==-1?0:typeResolvers.Values()[index].Obj();
			}

			bool SetTypeResolver(Ptr<IGuiResourceTypeResolver> resolver)override
			{
				if(typeResolvers.Keys().Contains(resolver->GetType())) return false;
				typeResolvers.Add(resolver->GetType(), resolver);

				if (auto precompile = resolver->Precompile())
				{
					vint maxPassIndex = precompile->GetMaxPassIndex();
					for (vint i = 0; i <= maxPassIndex; i++)
					{
						switch (precompile->GetPassSupport(i))
						{
						case IGuiResourceTypeResolver_Precompile::PerResource:
							perResourceResolvers.Add(i, resolver->GetType());
							break;
						case IGuiResourceTypeResolver_Precompile::PerPass:
							perPassResolvers.Add(i, resolver->GetType());
							break;
						default:;
						}
					}
				}

				return true;
			}

			vint GetMaxPrecompilePassIndex()override
			{
				vint maxPass = -1;
				FOREACH(Ptr<IGuiResourceTypeResolver>, resolver, typeResolvers.Values())
				{
					if (auto precompile = resolver->Precompile())
					{
						vint pass = precompile->GetMaxPassIndex();
						if (maxPass < pass)
						{
							maxPass = pass;
						}
					}
				}
				return maxPass;
			}

			vint GetMaxInitializePassIndex()override
			{
				vint maxPass = -1;
				FOREACH(Ptr<IGuiResourceTypeResolver>, resolver, typeResolvers.Values())
				{
					if (auto initialize = resolver->Initialize())
					{
						vint pass = initialize->GetMaxPassIndex();
						if (maxPass < pass)
						{
							maxPass = pass;
						}
					}
				}
				return maxPass;
			}

			void GetPerResourceResolverNames(vint passIndex, collections::List<WString>& names)override
			{
				names.Clear();
				vint index = perResourceResolvers.Keys().IndexOf(passIndex);
				if (index != -1)
				{
					CopyFrom(names, perResourceResolvers.GetByIndex(index));
				}
			}

			void GetPerPassResolverNames(vint passIndex, collections::List<WString>& names)override
			{
				names.Clear();
				vint index = perPassResolvers.Keys().IndexOf(passIndex);
				if (index != -1)
				{
					CopyFrom(names, perPassResolvers.GetByIndex(index));
				}
			}
		};
		GUI_REGISTER_PLUGIN(GuiResourceResolverManager)

/***********************************************************************
Helpers
***********************************************************************/

		void DecompressStream(const char** buffer, bool decompress, vint rows, vint block, vint remain, stream::IStream& outputStream)
		{
			if (decompress)
			{
				MemoryStream compressedStream;
				DecompressStream(buffer, false, rows, block, remain, compressedStream);
				compressedStream.SeekFromBegin(0);
				DecompressStream(compressedStream, outputStream);
			}
			else
			{
				for (vint i = 0; i < rows; i++)
				{
					vint size = i == rows - 1 ? remain : block;
					outputStream.Write((void*)buffer[i], size);
				}
			}
		}
	}
}
