#include "GuiDocument.h"
#include "GuiParserManager.h"
#include "../Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;
		using namespace collections;
		using namespace parsing;
		using namespace parsing::tabling;
		using namespace parsing::xml;
		using namespace parsing::json;
		using namespace regex;
		using namespace stream;

		WString GetFolderPath(const WString& filePath)
		{
			for (vint i = filePath.Length() - 1; i >= 0; i--)
			{
				if (filePath[i] == L'\\' || filePath[i] == L'/')
				{
					return filePath.Sub(0, i + 1);
				}
			}
			return L"";
		}

		WString GetFileName(const WString& filePath)
		{
			for (vint i = filePath.Length() - 1; i >= 0; i--)
			{
				if (filePath[i] == L'\\' || filePath[i] == L'/')
				{
					return filePath.Sub(i + 1, filePath.Length() - i - 1);
				}
			}
			return filePath;
		}

		bool LoadTextFile(const WString& filePath, WString& text)
		{
			stream::FileStream fileStream(filePath, stream::FileStream::ReadOnly);
			return LoadTextFromStream(fileStream, text);
		}

		bool LoadTextFromStream(stream::IStream& stream, WString& text)
		{
			if (stream.IsAvailable())
			{
				stream::BomDecoder decoder;
				stream::DecoderStream decoderStream(stream, decoder);
				stream::StreamReader reader(decoderStream);
				text = reader.ReadToEnd();
				return true;
			}
			return false;
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
		GlobalStringKey GlobalStringKey::_Set;
		GlobalStringKey GlobalStringKey::_Ref;
		GlobalStringKey GlobalStringKey::_Bind;
		GlobalStringKey GlobalStringKey::_Format;
		GlobalStringKey GlobalStringKey::_Eval;
		GlobalStringKey GlobalStringKey::_Uri;
		GlobalStringKey GlobalStringKey::_ControlTemplate;
		GlobalStringKey GlobalStringKey::_ItemTemplate;

		class GlobalStringKeyManager
		{
		public:
			Dictionary<WString, vint>		stoi;
			List<WString>					itos;

			void InitializeConstants()
			{
				GlobalStringKey::_Set = GlobalStringKey::Get(L"set");
				GlobalStringKey::_Ref = GlobalStringKey::Get(L"ref");
				GlobalStringKey::_Bind = GlobalStringKey::Get(L"bind");
				GlobalStringKey::_Format = GlobalStringKey::Get(L"format");
				GlobalStringKey::_Eval = GlobalStringKey::Get(L"eval");
				GlobalStringKey::_Uri = GlobalStringKey::Get(L"uri");
				GlobalStringKey::_ControlTemplate = GlobalStringKey::Get(L"ControlTemplate");
				GlobalStringKey::_ItemTemplate = GlobalStringKey::Get(L"ItemTemplate");
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

		GuiImageData::GuiImageData(Ptr<INativeImage> _image, vint _frameIndex, const WString& _filePath)
			: image(_image)
			, frameIndex(_frameIndex)
			, filePath(_filePath)
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

		const WString& GuiImageData::GetFilePath()
		{
			return filePath;
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

		void GuiResourceNodeBase::SetFileContentPath(const WString& value)
		{
			fileContentPath = value;
		}

		GuiResourceFolder* GuiResourceNodeBase::GetParent()
		{
			return parent;
		}

/***********************************************************************
GuiResourceError
***********************************************************************/

		void GuiResourceError::Initialize(Ptr<GuiResourceNodeBase> node)
		{
			auto current = node.Obj();
			while (current)
			{
				if (current->GetFileContentPath() != L"")
				{
					fileContentPath = current->GetFileContentPath();
					break;
				}
				current = current->GetParent();
			}
		}

		GuiResourceError::GuiResourceError()
		{
		}

		GuiResourceError::GuiResourceError(parsing::ParsingTextPos _position, const WString& _message)
			:position(_position)
			, message(_message)
		{
		}

		GuiResourceError::GuiResourceError(const WString& _fileContentPath, parsing::ParsingTextPos _position, const WString& _message)
			:fileContentPath(_fileContentPath)
			, position(_position)
			, message(_message)
		{
		}

		GuiResourceError::GuiResourceError(Ptr<GuiResourceNodeBase> node, parsing::ParsingTextPos _position, const WString& _message)
			:resourcePath(node->GetResourcePath())
			, position(_position)
			, message(_message)
		{
			Initialize(node);
		}

		GuiResourceError::GuiResourceError(Ptr<GuiResourceNodeBase> node, const WString& _message)
			:resourcePath(node->GetResourcePath())
			, message(_message)
		{
			Initialize(node);
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
						errors.Add(GuiResourceError(this, element->codeRange.start, L"A resource folder should have a name."));
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
									folder->SetFileContentPath(XmlGetValue(element));
									WString filePath = containingFolder + folder->GetFileContentPath();
									WString text;
									if (LoadTextFile(filePath, text))
									{
										if (auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML"))
										{
											List<Ptr<ParsingError>> parsingErrors;
											if (auto xml = parser->TypedParse(text, parsingErrors))
											{
												newContainingFolder = GetFolderPath(filePath);
												newFolderXml = xml->rootElement;
											}
											FOREACH(Ptr<ParsingError>, error, parsingErrors)
											{
												errors.Add(GuiResourceError(filePath, error->codeRange.start, error->errorMessage));
											}
										}
									}
									else
									{
										errors.Add(GuiResourceError(this, element->codeRange.start, L"Failed to load file \"" + filePath + L"\"."));
									}
								}
							}
							folder->LoadResourceFolderFromXml(delayLoadings, newContainingFolder, newFolderXml, errors);
						}
						else
						{
							errors.Add(GuiResourceError(this, element->codeRange.start, L"Duplicated resource folder name \"" + name + L"\"."));
						}
					}
				}
				else if (element->name.value.Length() <= 3 || element->name.value.Sub(0, 4) != L"ref.")
				{
					WString relativeFilePath;
					WString filePath;
					if (Ptr<XmlAttribute> contentAtt = XmlGetAttribute(element, L"content"))
					{
						if (contentAtt->value.value == L"File")
						{
							relativeFilePath = XmlGetValue(element);
							filePath = containingFolder + relativeFilePath;
							if (name == L"")
							{
								name = GetFileName(filePath);
							}
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
								if (preloadType != L"")
								{
									preloadResolver = GetResourceResolverManager()->GetTypeResolver(preloadType);
									if (!preloadResolver)
									{
										errors.Add(GuiResourceError(this, element->codeRange.start, L"Unknown resource resolver \"" + preloadType + L"\" of resource type \"" + type + L"\"."));
									}
								}
							}
						}
						else
						{
							errors.Add(GuiResourceError(this, element->codeRange.start, L"Unknown resource type \"" + type + L"\"."));
						}

						if (typeResolver && preloadResolver)
						{
							if (auto directLoad = preloadResolver->DirectLoadXml())
							{
								Ptr<DescriptableObject> resource;
								WString itemType = preloadResolver->GetType();
								if (filePath == L"")
								{
									resource = directLoad->ResolveResource(element, errors);
								}
								else
								{
									item->SetFileContentPath(relativeFilePath);
									resource = directLoad->ResolveResource(filePath, errors);
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
										else if (resource)
										{
											resource = indirectLoad->ResolveResource(resource, 0, errors);
											itemType = typeResolver->GetType();
										}
									}
									else
									{
										resource = 0;
										errors.Add(GuiResourceError(this, element->codeRange.start, L"Resource type \"" + typeResolver->GetType() + L"\" is not a indirect load resource type."));
									}
								}
								item->SetContent(itemType, resource);
							}
							else
							{
								errors.Add(GuiResourceError(this, element->codeRange.start, L"Resource type \"" + preloadResolver->GetType() + L"\" is not a direct load resource type."));
							}
						}

						if (!item->GetContent())
						{
							RemoveItem(name);
						}
					}
					else
					{
						errors.Add(GuiResourceError(this, element->codeRange.start, L"Duplicated resource item name \"" + name + L"\"."));
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
							xmlElement = directLoad->Serialize(item->GetContent());
						}
						else if (auto indirectLoad = resolver->IndirectLoad())
						{
							if (auto preloadResolver = GetResourceResolverManager()->GetTypeResolver(indirectLoad->GetPreloadType()))
							{
								if (auto directLoad = preloadResolver->DirectLoadXml())
								{
									if (auto resource = indirectLoad->Serialize(item->GetContent()))
									{
										xmlElement = directLoad->Serialize(resource);
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
				

				if (folder->GetFileContentPath() == L"")
				{
					folder->SaveResourceFolderToXml(xmlFolder);
				}
				else
				{
					auto attContent = MakePtr<XmlAttribute>();
					attContent->name.value = L"content";
					attContent->value.value = L"Link";
					xmlFolder->attributes.Add(attContent);

					auto xmlText = MakePtr<XmlText>();
					xmlText->content.value = folder->GetFileContentPath();
					xmlFolder->subNodes.Add(xmlText);
				}
			}
		}

		void GuiResourceFolder::CollectTypeNames(collections::List<WString>& typeNames)
		{
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
									errors.Add(GuiResourceError(item, L"Unknown resource resolver \"" + preloadType + L"\" of resource type \"" + type + L"\"."));
								}
							}
						}
					}
					else
					{
						errors.Add(GuiResourceError(item, L"Unknown resource type \"" + type + L"\"."));
					}

					if(typeResolver && preloadResolver)
					{
						if (auto directLoad = preloadResolver->DirectLoadStream())
						{
							WString itemType = preloadResolver->GetType();
							auto resource = directLoad->ResolveResourcePrecompiled(reader.input, errors);

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
									else if(resource)
									{
										resource = indirectLoad->ResolveResource(resource, nullptr, errors);
										itemType = typeResolver->GetType();
									}
								}
								else
								{
									resource = 0;
									errors.Add(GuiResourceError(item, L"Resource type \"" + typeResolver->GetType() + L"\" is not a indirect load resource type."));
								}
							}
							item->SetContent(itemType, resource);
						}
						else
						{
							errors.Add(GuiResourceError(item, L"Resource type \"" + preloadResolver->GetType() + L"\" is not a direct load resource type."));
						}
					}

					if(!item->GetContent())
					{
						RemoveItem(name);
					}
				}
				else
				{
					errors.Add(GuiResourceError(this, L"Duplicated resource item name \"" + name + L"\"."));
				}
			}

			reader << count;
			for (vint i = 0; i < count; i++)
			{
				WString name;
				reader << name;

				auto folder = MakePtr<GuiResourceFolder>();
				folder->LoadResourceFolderFromBinary(delayLoadings, reader, typeNames, errors);
				AddFolder(name, folder);
			}
		}

		void GuiResourceFolder::SaveResourceFolderToBinary(stream::internal::ContextFreeWriter& writer, collections::List<WString>& typeNames)
		{
			typedef Tuple<vint, WString, IGuiResourceTypeResolver_DirectLoadStream*, Ptr<DescriptableObject>> ItemTuple;
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
						itemTuples.Add(ItemTuple(typeName, name, directLoad, item->GetContent()));
					}
					else if (auto indirectLoad = resolver->IndirectLoad())
					{
						if (auto preloadResolver = GetResourceResolverManager()->GetTypeResolver(indirectLoad->GetPreloadType()))
						{
							if (auto directLoad = preloadResolver->DirectLoadStream())
							{
								if (auto resource = indirectLoad->Serialize(item->GetContent()))
								{
									itemTuples.Add(ItemTuple(typeName, name, directLoad, resource));
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
				directLoad->SerializePrecompiled(resource, writer.output);
			}

			count = folders.Count();
			writer << count;
			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				WString name = folder->GetName();
				writer << name;
				folder->SaveResourceFolderToBinary(writer, typeNames);
			}
		}

		void GuiResourceFolder::PrecompileResourceFolder(GuiResourcePrecompileContext& context, IGuiResourcePrecompileCallback* callback, GuiResourceError::List& errors)
		{
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

		void GuiResourceFolder::InitializeResourceFolder(GuiResourceInitializeContext& context)
		{
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto typeResolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
				if (auto initialize = typeResolver->Initialize())
				{
					initialize->Initialize(item, context);
				}
			}

			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				folder->InitializeResourceFolder(context);
			}
		}

		GuiResourceFolder::GuiResourceFolder()
		{
		}

		GuiResourceFolder::~GuiResourceFolder()
		{
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
			if(item->GetParent()!=0 || items.Keys().Contains(name)) return false;
			items.Add(name, item);
			item->parent=this;
			item->name=name;
			return true;
		}

		Ptr<GuiResourceItem> GuiResourceFolder::RemoveItem(const WString& name)
		{
			Ptr<GuiResourceItem> item=GetItem(name);
			if(!item) return 0;
			items.Remove(name);
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
			if(folder->GetParent()!=0 || folders.Keys().Contains(name)) return false;
			folders.Add(name, folder);
			folder->parent=this;
			folder->name=name;
			return true;
		}

		Ptr<GuiResourceFolder> GuiResourceFolder::RemoveFolder(const WString& name)
		{
			Ptr<GuiResourceFolder> folder=GetFolder(name);
			if(!folder) return 0;
			folders.Remove(name);
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
GuiResource
***********************************************************************/

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
							Ptr<DescriptableObject> resource = indirectLoad->ResolveResource(item->GetContent(), pathResolver, errors);
							if (resource)
							{
								item->SetContent(typeResolver->GetType(), resource);
							}
						}
					}
					else
					{
						errors.Add(GuiResourceError(item, L"Resource type \"" + type + L"\" is not a indirect load resource type."));
					}
				}
				else
				{
					errors.Add(GuiResourceError(item, L"Unknown resource type \"" + type + L"\"."));
				}
			}
		}

		GuiResource::GuiResource()
		{
		}

		GuiResource::~GuiResource()
		{
		}

		WString GuiResource::GetWorkingDirectory()
		{
			return workingDirectory;
		}

		Ptr<GuiResource> GuiResource::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, const WString& filePath, const WString& workingDirectory, GuiResourceError::List& errors)
		{
			Ptr<GuiResource> resource = new GuiResource;
			resource->SetFileContentPath(filePath);
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
					List<Ptr<ParsingError>> parsingErrors;
					xml = parser->TypedParse(text, parsingErrors);
					FOREACH(Ptr<ParsingError>, error, parsingErrors)
					{
						errors.Add(GuiResourceError(filePath, error->codeRange.start, error->errorMessage));
					}
				}
				else
				{
					errors.Add(GuiResourceError(filePath, ParsingTextPos(), L"Failed to load file \"" + filePath + L"\"."));
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

			List<WString> typeNames;
			reader << typeNames;
			
			DelayLoadingList delayLoadings;
			resource->LoadResourceFolderFromBinary(delayLoadings, reader, typeNames, errors);
			
			ProcessDelayLoading(resource, delayLoadings, errors);
			return resource;
		}

		void GuiResource::SavePrecompiledBinary(stream::IStream& stream)
		{
			stream::internal::ContextFreeWriter writer(stream);

			List<WString> typeNames;
			CollectTypeNames(typeNames);
			writer << typeNames;
			SaveResourceFolderToBinary(writer, typeNames);
		}

		void GuiResource::Precompile(IGuiResourcePrecompileCallback* callback, GuiResourceError::List& errors)
		{
			if (GetFolder(L"Precompiled"))
			{
				errors.Add(GuiResourceError(this, L"A precompiled resource cannot be compiled again."));
				return;
			}

			GuiResourcePrecompileContext context;
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
			}
			if (errors.Count() == 0)
			{
				AddFolder(L"Precompiled", context.targetFolder);
			}
		}

		void GuiResource::Initialize(GuiResourceUsage usage)
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
				InitializeResourceFolder(context);
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
GuiResourcePathFileResolver
***********************************************************************/

		class GuiResourcePathFileResolver : public Object, public IGuiResourcePathResolver
		{
		protected:
			WString					workingDirectory;

		public:
			GuiResourcePathFileResolver(const WString& _workingDirectory)
				:workingDirectory(_workingDirectory)
			{
			}

			Ptr<DescriptableObject> ResolveResource(const WString& path)
			{
				WString filename=path;
				if(filename.Length()>=2 && filename[1]!=L':')
				{
					filename=workingDirectory+filename;
				}
				Ptr<INativeImage> image=GetCurrentController()->ImageService()->CreateImageFromFile(filename);
				return new GuiImageData(image, 0);
			}

			class Factory : public Object, public IGuiResourcePathResolverFactory
			{
			public:
				WString GetProtocol()override
				{
					return L"file";
				}

				Ptr<IGuiResourcePathResolver> CreateResolver(Ptr<GuiResource> resource, const WString& workingDirectory)override
				{
					return new GuiResourcePathFileResolver(workingDirectory);
				}
			};
		};

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
				if(resource)
				{
					if(path.Length()>0)
					{
						switch(path[path.Length()-1])
						{
						case L'\\':case L'/':
							return resource->GetFolderByPath(path);
						default:
							return resource->GetValueByPath(path);
						}
					}
				}
				return 0;
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
			GuiResourceResolverManager()
			{
			}

			~GuiResourceResolverManager()
			{
			}

			void Load()override
			{
				globalStringKeyManager = new GlobalStringKeyManager();
				globalStringKeyManager->InitializeConstants();

				resourceResolverManager = this;
				SetPathResolverFactory(new GuiResourcePathFileResolver::Factory);
				SetPathResolverFactory(new GuiResourcePathResResolver::Factory);
			}

			void AfterLoad()override
			{
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
	}
}
