#include "GuiDocument.h"
#include "GuiParserManager.h"
#include "../Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace controls;
		using namespace collections;
		using namespace parsing::tabling;
		using namespace parsing::xml;
		using namespace parsing::json;
		using namespace regex;
		using namespace stream;

		WString GetFolderPath(const WString& filePath)
		{
			for(vint i=filePath.Length()-1;i>=0;i--)
			{
				if(filePath[i]==L'\\' || filePath[i]==L'/')
				{
					return filePath.Sub(0, i+1);
				}
			}
			return L"";
		}

		WString GetFileName(const WString& filePath)
		{
			for(vint i=filePath.Length()-1;i>=0;i--)
			{
				if(filePath[i]==L'\\' || filePath[i]==L'/')
				{
					return filePath.Sub(i+1, filePath.Length()-i-1);
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
			if(stream.IsAvailable())
			{
				stream::BomDecoder decoder;
				stream::DecoderStream decoderStream(stream, decoder);
				stream::StreamReader reader(decoderStream);
				text=reader.ReadToEnd();
				return true;
			}
			return false;
		}

		bool IsResourceUrl(const WString& text, WString& protocol, WString& path)
		{
			Pair<vint, vint> index=INVLOC.FindFirst(text, L"://", Locale::None);
			if(index.key!=-1)
			{
				protocol=INVLOC.ToLower(text.Sub(0, index.key));
				path=text.Sub(index.key+index.value, text.Length()-index.key-index.value);
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
		GlobalStringKey GlobalStringKey::_Workflow_Assembly_Cache;
		GlobalStringKey GlobalStringKey::_Workflow_Global_Context;
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
				GlobalStringKey::_Workflow_Assembly_Cache = GlobalStringKey::Get(L"WORKFLOW-ASSEMBLY-CACHE");
				GlobalStringKey::_Workflow_Global_Context = GlobalStringKey::Get(L"WORKFLOW-GLOBAL-CONTEXT");
				GlobalStringKey::_ControlTemplate = GlobalStringKey::Get(L"ControlTemplate");
				GlobalStringKey::_ItemTemplate = GlobalStringKey::Get(L"ItemTemplate");
			}
		}* globalStringKeyManager = 0;

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
			:image(_image)
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

		const WString& GuiResourceNodeBase::GetPath()
		{
			return path;
		}

		void GuiResourceNodeBase::SetPath(const WString& value)
		{
			path = value;
		}

		GuiResourceFolder* GuiResourceNodeBase::GetParent()
		{
			return parent;
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

		void GuiResourceFolder::LoadResourceFolderFromXml(DelayLoadingList& delayLoadings, const WString& containingFolder, Ptr<parsing::xml::XmlElement> folderXml, collections::List<WString>& errors)
		{
			ClearItems();
			ClearFolders();
			FOREACH(Ptr<XmlElement>, element, XmlGetElements(folderXml))
			{
				WString name;
				if(Ptr<XmlAttribute> nameAtt=XmlGetAttribute(element, L"name"))
				{
					name=nameAtt->value.value;
				}
				if(element->name.value==L"Folder")
				{
					if (name == L"")
					{
						errors.Add(L"A resource folder should have a name.");
						errors.Add(
							L"Format: RESOURCE, Row: " + itow(element->codeRange.start.row + 1) +
							L", Column: " + itow(element->codeRange.start.column + 1) +
							L", Message: A resource folder should have a name.");
					}
					else
					{
						Ptr<GuiResourceFolder> folder=new GuiResourceFolder;
						if(AddFolder(name, folder))
						{
							WString newContainingFolder=containingFolder;
							Ptr<XmlElement> newFolderXml=element;
							if(Ptr<XmlAttribute> contentAtt=XmlGetAttribute(element, L"content"))
							{
								if(contentAtt->value.value==L"Link")
								{
									folder->SetPath(XmlGetValue(element));
									WString filePath = containingFolder + folder->GetPath();
									WString text;
									if(LoadTextFile(filePath, text))
									{
										if(auto parser=GetParserManager()->GetParser<XmlDocument>(L"XML"))
										{
											if(auto xml=parser->TypedParse(text, errors))
											{
												newContainingFolder=GetFolderPath(filePath);
												newFolderXml=xml->rootElement;
											}
										}
									}
									else
									{
										errors.Add(L"Failed to load file \"" + filePath + L"\".");
									}
								}
							}
							folder->LoadResourceFolderFromXml(delayLoadings, newContainingFolder, newFolderXml, errors);
						}
						else
						{
							errors.Add(L"Duplicated resource folder name \"" + name + L"\".");
						}
					}
				}
				else if(element->name.value.Length() <= 3 || element->name.value.Sub(0, 4) != L"ref.")
				{
					WString relativeFilePath;
					WString filePath;
					if(Ptr<XmlAttribute> contentAtt=XmlGetAttribute(element, L"content"))
					{
						if(contentAtt->value.value==L"File")
						{
							relativeFilePath = XmlGetValue(element);
							filePath = containingFolder + relativeFilePath;
							if(name==L"")
							{
								name=GetFileName(filePath);
							}
						}
					}

					Ptr<GuiResourceItem> item = new GuiResourceItem;
					if(AddItem(name, item))
					{
						WString type = element->name.value;
						IGuiResourceTypeResolver* typeResolver = GetResourceResolverManager()->GetTypeResolver(type);
						IGuiResourceTypeResolver* preloadResolver = typeResolver;

						if(typeResolver)
						{
							if (!typeResolver->DirectLoadXml())
							{
								WString preloadType = typeResolver->IndirectLoad()->GetPreloadType();
								if (preloadType != L"")
								{
									preloadResolver = GetResourceResolverManager()->GetTypeResolver(preloadType);
									if (!preloadResolver)
									{
										errors.Add(L"Unknown resource resolver \"" + preloadType + L"\" of resource type \"" + type + L"\".");
									}
								}
							}
						}
						else
						{
							errors.Add(L"Unknown resource type \"" + type + L"\".");
						}

						if(typeResolver && preloadResolver)
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
									item->SetPath(relativeFilePath);
									resource = directLoad->ResolveResource(filePath, errors);
								}

								if (typeResolver != preloadResolver)
								{
									if (auto indirectLoad = typeResolver->IndirectLoad())
									{
										if(indirectLoad->IsDelayLoad())
										{
											DelayLoading delayLoading;
											delayLoading.type = type;
											delayLoading.workingDirectory = containingFolder;
											delayLoading.preloadResource = item;
											delayLoadings.Add(delayLoading);
										}
										else if(resource)
										{
											resource = indirectLoad->ResolveResource(resource, 0, errors);
											itemType = typeResolver->GetType();
										}
									}
									else
									{
										resource = 0;
										errors.Add(L"Resource type \"" + typeResolver->GetType() + L"\" is not a indirect load resource type.");
									}
								}
								item->SetContent(itemType, resource);
							}
							else
							{
								errors.Add(L"Resource type \"" + preloadResolver->GetType() + L"\" is not a direct load resource type.");
							}
						}

						if(!item->GetContent())
						{
							RemoveItem(name);
						}
					}
					else
					{
						errors.Add(L"Duplicated resource item name \"" + name + L"\".");
					}
				}
			}
		}

		void GuiResourceFolder::SaveResourceFolderToXml(Ptr<parsing::xml::XmlElement> xmlParent, bool serializePrecompiledResource)
		{
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto attName = MakePtr<XmlAttribute>();
				attName->name.value = L"name";
				attName->value.value = item->GetName();

				if (serializePrecompiledResource || item->GetPath() == L"")
				{
					auto resolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
					Ptr<XmlElement> xmlElement;

					if (auto directLoad = resolver->DirectLoadXml())
					{
						xmlElement = directLoad->Serialize(item->GetContent(), serializePrecompiledResource);
					}
					else if (auto indirectLoad = resolver->IndirectLoad())
					{
						if (auto preloadResolver = GetResourceResolverManager()->GetTypeResolver(indirectLoad->GetPreloadType()))
						{
							if (auto directLoad = preloadResolver->DirectLoadXml())
							{
								if (auto resource = indirectLoad->Serialize(item->GetContent(), serializePrecompiledResource))
								{
									xmlElement = directLoad->Serialize(resource, serializePrecompiledResource);
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
					xmlText->content.value = item->GetPath();
					xmlElement->subNodes.Add(xmlText);
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
				

				if (serializePrecompiledResource || folder->GetPath() == L"")
				{
					folder->SaveResourceFolderToXml(xmlFolder, serializePrecompiledResource);
				}
				else
				{
					auto attContent = MakePtr<XmlAttribute>();
					attContent->name.value = L"content";
					attContent->value.value = L"Link";
					xmlFolder->attributes.Add(attContent);

					auto xmlText = MakePtr<XmlText>();
					xmlText->content.value = folder->GetPath();
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

		void GuiResourceFolder::LoadResourceFolderFromBinary(DelayLoadingList& delayLoadings, stream::internal::Reader& reader, collections::List<WString>& typeNames, collections::List<WString>& errors)
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
									errors.Add(L"Unknown resource resolver \"" + preloadType + L"\" of resource type \"" + type + L"\".");
								}
							}
						}
					}
					else
					{
						errors.Add(L"Unknown resource type \"" + type + L"\".");
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
										resource = indirectLoad->ResolveResource(resource, 0, errors);
										itemType = typeResolver->GetType();
									}
								}
								else
								{
									resource = 0;
									errors.Add(L"Resource type \"" + typeResolver->GetType() + L"\" is not a indirect load resource type.");
								}
							}
							item->SetContent(itemType, resource);
						}
						else
						{
							errors.Add(L"Resource type \"" + preloadResolver->GetType() + L"\" is not a direct load resource type.");
						}
					}

					if(!item->GetContent())
					{
						RemoveItem(name);
					}
				}
				else
				{
					errors.Add(L"Duplicated resource item name \"" + name + L"\".");
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

		void GuiResourceFolder::SaveResourceFolderToBinary(stream::internal::Writer& writer, collections::List<WString>& typeNames)
		{
			typedef Tuple<vint, WString, IGuiResourceTypeResolver_DirectLoadStream*, Ptr<DescriptableObject>> ItemTuple;
			List<ItemTuple> itemTuples;

			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				vint typeName = typeNames.IndexOf(item->GetTypeName());
				WString name = item->GetName();

				auto resolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
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
							if (auto resource = indirectLoad->Serialize(item->GetContent(), true))
							{
								itemTuples.Add(ItemTuple(typeName, name, directLoad, resource));
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

		void GuiResourceFolder::PrecompileResourceFolder(Ptr<GuiResourcePathResolver> resolver, GuiResource* rootResource, vint passIndex, collections::List<WString>& errors)
		{
			FOREACH(Ptr<GuiResourceItem>, item, items.Values())
			{
				auto typeResolver = GetResourceResolverManager()->GetTypeResolver(item->GetTypeName());
				if (auto precompile = typeResolver->Precompile())
				{
					precompile->Precompile(item->GetContent(), rootResource, passIndex, resolver, errors);
				}
			}

			FOREACH(Ptr<GuiResourceFolder>, folder, folders.Values())
			{
				folder->PrecompileResourceFolder(resolver, rootResource, passIndex, errors);
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

/***********************************************************************
GuiResource
***********************************************************************/

		void IGuiResourceCache::LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::Dictionary<GlobalStringKey, Ptr<IGuiResourceCache>>& caches)
		{
			FOREACH(Ptr<XmlElement>, element, XmlGetElements(xml, L"Cache"))
			{
				auto attName = XmlGetAttribute(element, L"Name");
				auto attType = XmlGetAttribute(element, L"Type");
				if (attName && attType)
				{
					auto resolver = GetResourceResolverManager()->GetCacheResolver(GlobalStringKey::Get(attType->value.value));

					MemoryStream stream;
					HexToBinary(stream, XmlGetValue(element));
					stream.SeekFromBegin(0);

					auto cache = resolver->Deserialize(stream);
					caches.Add(GlobalStringKey::Get(attName->value.value), cache);
				}
			}
		}

		void IGuiResourceCache::SaveToXml(Ptr<parsing::xml::XmlElement> xml, collections::Dictionary<GlobalStringKey, Ptr<IGuiResourceCache>>& caches)
		{
			for (vint i = 0; i < caches.Count(); i++)
			{
				auto key = caches.Keys()[i];
				auto value = caches.Values()[i];
				auto resolver = GetResourceResolverManager()->GetCacheResolver(value->GetCacheTypeName());

				MemoryStream stream;
				resolver->Serialize(value, stream);
				stream.SeekFromBegin(0);
				auto hex = BinaryToHex(stream);
					
				auto xmlCache = MakePtr<XmlElement>();
				xmlCache->name.value = L"Cache";
				xml->subNodes.Add(xmlCache);

				auto attName = MakePtr<XmlAttribute>();
				attName->name.value = L"Name";
				attName->value.value = key.ToString();
				xmlCache->attributes.Add(attName);

				auto attType = MakePtr<XmlAttribute>();
				attType->name.value = L"Type";
				attType->value.value = value->GetCacheTypeName().ToString();
				xmlCache->attributes.Add(attType);

				auto xmlContent = MakePtr<XmlCData>();
				xmlContent->content.value = hex;
				xmlCache->subNodes.Add(xmlContent);
			}
		}

		void IGuiResourceCache::LoadFromBinary(stream::internal::Reader& reader, collections::Dictionary<GlobalStringKey, Ptr<IGuiResourceCache>>& caches, collections::List<GlobalStringKey>& sortedKeys)
		{
			vint count = 0;
			reader << count;

			for (vint i = 0; i < count; i++)
			{
				GlobalStringKey key, name;
				if (&sortedKeys)
				{
					vint keyIndex = -1;
					vint nameIndex = -1;
					reader << keyIndex << nameIndex;
					auto key = sortedKeys[keyIndex];
					auto name = sortedKeys[nameIndex];
				}
				else
				{
					WString keyString, nameString;
					reader << keyString << nameString;
					key = GlobalStringKey::Get(keyString);
					name = GlobalStringKey::Get(nameString);
				}

				stream::MemoryStream stream;
				reader << (stream::IStream&)stream;

				if (auto resolver = GetResourceResolverManager()->GetCacheResolver(name))
				{
					if (auto cache = resolver->Deserialize(stream))
					{
						caches.Add(key, cache);
					}
				}
			}
		}

		void IGuiResourceCache::SaveToBinary(stream::internal::Writer& writer, collections::Dictionary<GlobalStringKey, Ptr<IGuiResourceCache>>& caches, collections::SortedList<GlobalStringKey>& sortedKeys)
		{
			vint count = caches.Count();
			writer << count;
			for (vint i = 0; i < count; i++)
			{
				auto cache = caches.Values()[i];
				auto key = caches.Keys()[i];
				auto name = cache->GetCacheTypeName();
				if (&sortedKeys)
				{
					auto keyIndex = sortedKeys.IndexOf(key);
					vint nameIndex = sortedKeys.IndexOf(name);
					CHECK_ERROR(keyIndex != -1 && nameIndex != -1, L"IGuiResourceCache::SaveToBinary(stream::internal::Writer&, collections::Dictionary<GlobalStringKey, Ptr<IGuiResourceCache>>&)#Internal Error.");
					writer << keyIndex << nameIndex;
				}
				else
				{
					WString keyString = key.ToString();
					WString nameString = name.ToString();
					writer << keyString << nameString;
				}

				stream::MemoryStream stream;
				
				if (auto resolver = GetResourceResolverManager()->GetCacheResolver(name))
				{
					resolver->Serialize(cache, stream);
				}
				writer << (stream::IStream&)stream;
			}
		}

/***********************************************************************
GuiResource
***********************************************************************/

		void GuiResource::ProcessDelayLoading(Ptr<GuiResource> resource, DelayLoadingList& delayLoadings, collections::List<WString>& errors)
		{
			FOREACH(DelayLoading, delay, delayLoadings)
			{
				WString type = delay.type;
				WString folder = delay.workingDirectory;
				Ptr<GuiResourceItem> item = delay.preloadResource;

				auto typeResolver = GetResourceResolverManager()->GetTypeResolver(type);
				auto indirectLoad = typeResolver->IndirectLoad();
				if (!indirectLoad)
				{
					errors.Add(L"Unknown resource type \"" + type + L"\".");
				}
				else if (item->GetContent())
				{
					Ptr<GuiResourcePathResolver> pathResolver = new GuiResourcePathResolver(resource, folder);
					Ptr<DescriptableObject> resource = indirectLoad->ResolveResource(item->GetContent(), pathResolver, errors);
					if(resource)
					{
						item->SetContent(typeResolver->GetType(), resource);
					}
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

		Ptr<GuiResource> GuiResource::LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, const WString& workingDirectory, collections::List<WString>& errors)
		{
			Ptr<GuiResource> resource = new GuiResource;
			resource->workingDirectory = workingDirectory;
			DelayLoadingList delayLoadings;
			if (auto xmlCaches = XmlGetElement(xml->rootElement, L"ref.Caches"))
			{
				IGuiResourceCache::LoadFromXml(xmlCaches, resource->precompiledCaches);
			}
			resource->LoadResourceFolderFromXml(delayLoadings, resource->workingDirectory, xml->rootElement, errors);

			ProcessDelayLoading(resource, delayLoadings, errors);
			return resource;
		}

		Ptr<GuiResource> GuiResource::LoadFromXml(const WString& filePath, collections::List<WString>& errors)
		{
			Ptr<XmlDocument> xml;
			if(auto parser=GetParserManager()->GetParser<XmlDocument>(L"XML"))
			{
				WString text;
				if(LoadTextFile(filePath, text))
				{
					xml = parser->TypedParse(text, errors);
				}
				else
				{
					errors.Add(L"Failed to load file \"" + filePath + L"\".");
				}
			}
			if(xml)
			{
				return LoadFromXml(xml, GetFolderPath(filePath), errors);
			}
			return 0;
		}

		Ptr<parsing::xml::XmlDocument> GuiResource::SaveToXml(bool serializePrecompiledResource)
		{
			auto xmlRoot = MakePtr<XmlElement>();
			xmlRoot->name.value = L"Resource";
			if (serializePrecompiledResource && precompiledCaches.Count() > 0)
			{
				auto xmlCaches = MakePtr<XmlElement>();
				xmlCaches->name.value = L"ref.Caches";
				xmlRoot->subNodes.Add(xmlCaches);
				IGuiResourceCache::SaveToXml(xmlCaches, precompiledCaches);
			}
			SaveResourceFolderToXml(xmlRoot, serializePrecompiledResource);

			auto doc = MakePtr<XmlDocument>();
			doc->rootElement = xmlRoot;
			return doc;
		}

		Ptr<GuiResource> GuiResource::LoadPrecompiledBinary(stream::IStream& stream, collections::List<WString>& errors)
		{
			stream::internal::Reader reader(stream);
			auto resource = MakePtr<GuiResource>();

			List<WString> typeNames;
			reader << typeNames;
			IGuiResourceCache::LoadFromBinary(reader, resource->precompiledCaches);
			
			DelayLoadingList delayLoadings;
			resource->LoadResourceFolderFromBinary(delayLoadings, reader, typeNames, errors);
			
			ProcessDelayLoading(resource, delayLoadings, errors);
			return resource;
		}

		void GuiResource::SavePrecompiledBinary(stream::IStream& stream)
		{
			stream::internal::Writer writer(stream);

			List<WString> typeNames;
			CollectTypeNames(typeNames);
			writer << typeNames;
			IGuiResourceCache::SaveToBinary(writer, precompiledCaches);
			SaveResourceFolderToBinary(writer, typeNames);
		}

		void GuiResource::Precompile(collections::List<WString>& errors)
		{
			vint maxPass = GetResourceResolverManager()->GetMaxPrecompilePassIndex();
			Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(this, workingDirectory);
			for (vint i = 0; i <= maxPass; i++)
			{
				PrecompileResourceFolder(resolver, this, i, errors);
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
			typedef Dictionary<GlobalStringKey, Ptr<IGuiResourceCacheResolver>>			CacheResolverMap;
		protected:
			PathFactoryMap				pathFactories;
			TypeResolverMap				typeResolvers;
			CacheResolverMap			cacheResolvers;

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
				return true;
			}

			vint GetMaxPrecompilePassIndex()
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

			IGuiResourceCacheResolver* GetCacheResolver(GlobalStringKey cacheTypeName)override
			{
				vint index = cacheResolvers.Keys().IndexOf(cacheTypeName);
				return index == -1 ? 0 : cacheResolvers.Values()[index].Obj();
			}

			bool SetCacheResolver(Ptr<IGuiResourceCacheResolver> cacheResolver)override
			{
				if (cacheResolvers.Keys().Contains(cacheResolver->GetCacheTypeName())) return false;
				cacheResolvers.Add(cacheResolver->GetCacheTypeName(), cacheResolver);
				return true;
			}
		};
		GUI_REGISTER_PLUGIN(GuiResourceResolverManager)
	}
}