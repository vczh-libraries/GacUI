#include "GuiInstanceLoader.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;

/***********************************************************************
LogInstanceLoaderManager_GetParentTypes
***********************************************************************/

		void LogInstanceLoaderManager_GetParentTypes(const WString& typeName, List<WString>& parentTypes)
		{
			if (ITypeDescriptor* type = GetGlobalTypeManager()->GetTypeDescriptor(typeName))
			{
				vint parentCount = type->GetBaseTypeDescriptorCount();
				for (vint j = 0; j < parentCount; j++)
				{
					ITypeDescriptor* parent = type->GetBaseTypeDescriptor(j);
					parentTypes.Add(parent->GetTypeName());
				}
			}
			else
			{
				parentTypes.Add(GetInstanceLoaderManager()->GetParentTypeForVirtualType(GlobalStringKey::Get(typeName)).ToString());
			}
		}

/***********************************************************************
LogInstanceLoaderManager_PrintParentTypes
***********************************************************************/

		void LogInstanceLoaderManager_PrintParentTypes(stream::TextWriter& writer, const WString& typeName)
		{
			List<WString> parentTypes;
			LogInstanceLoaderManager_GetParentTypes(typeName, parentTypes);
			FOREACH_INDEXER(WString, parentType, index, parentTypes)
			{
				writer.WriteLine(L"        " + WString(index == 0 ? L": " : L", ") + parentType);
			}
		}

/***********************************************************************
LogInstanceLoaderManager_PrintFieldName
***********************************************************************/

		void LogInstanceLoaderManager_PrintFieldName(stream::TextWriter& writer, const WString& name)
		{
			writer.WriteString(L"        " + name);
			for (vint i = name.Length(); i < 24; i++)
			{
				writer.WriteChar(L' ');
			}
			writer.WriteString(L" : ");
		}

/***********************************************************************
LogInstanceLoaderManager_PrintProperties
***********************************************************************/

		void LogInstanceLoaderManager_PrintProperties(stream::TextWriter& writer, const WString& typeName)
		{
			List<IGuiInstanceLoader*> loaders;
			{
				IGuiInstanceLoader* loader = GetInstanceLoaderManager()->GetLoader(GlobalStringKey::Get(typeName));
				while (loader)
				{
					loaders.Add(loader);
					loader = GetInstanceLoaderManager()->GetParentLoader(loader);
				}
			}
			
			IGuiInstanceLoader::TypeInfo typeInfo(GlobalStringKey::Get(typeName), GetInstanceLoaderManager()->GetTypeDescriptorForType(GlobalStringKey::Get(typeName)));
			Dictionary<GlobalStringKey, IGuiInstanceLoader*> propertyLoaders;
			FOREACH(IGuiInstanceLoader*, loader, loaders)
			{
				List<GlobalStringKey> propertyNames;
				loader->GetPropertyNames(typeInfo, propertyNames);

				FOREACH(GlobalStringKey, propertyName, propertyNames)
				{
					if (!propertyLoaders.Keys().Contains(propertyName))
					{
						propertyLoaders.Add(propertyName, loader);
					}
				}
			}

			FOREACH_INDEXER(GlobalStringKey, propertyName, index, propertyLoaders.Keys())
			{
				SortedList<WString> acceptableTypes;
				Ptr<GuiInstancePropertyInfo> firstInfo;
				IGuiInstanceLoader* loader = propertyLoaders.Values()[index];
				IGuiInstanceLoader::PropertyInfo propertyInfo(typeInfo, propertyName);

				while (loader)
				{
					if (auto info = loader->GetPropertyType(propertyInfo))
					{
						if (firstInfo)
						{
							if (info->support != firstInfo->support)
							{
								break;
							}
						}
						else
						{
							firstInfo = info;
						}

						if (info->support!=GuiInstancePropertyInfo::NotSupport)
						{
							FOREACH(ITypeDescriptor*, type, info->acceptableTypes)
							{
								if (!acceptableTypes.Contains(type->GetTypeName()))
								{
									acceptableTypes.Add(type->GetTypeName());
								}
							}

							if (!info->tryParent)
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					
					vint index = loaders.IndexOf(loader);
					loader = index == loaders.Count() - 1 ? 0 : loaders[index + 1];
				}

				if (firstInfo->support == GuiInstancePropertyInfo::NotSupport)
				{
					continue;
				}

				LogInstanceLoaderManager_PrintFieldName(writer, (propertyName == GlobalStringKey::Empty? L"<DEFAULT-PROPERTY>" : propertyName.ToString()));
				switch (firstInfo->scope)
				{
				case GuiInstancePropertyInfo::ViewModel:
					writer.WriteString(L"#");
					break;
				case GuiInstancePropertyInfo::Constructor:
					writer.WriteString(firstInfo->required ? L"+" : L"*");
					break;
				case GuiInstancePropertyInfo::Property:
					writer.WriteString(L" ");
					break;
				}
				switch (firstInfo->support)
				{
				case GuiInstancePropertyInfo::SupportAssign:
					writer.WriteString(L"[assign]     ");
					break;
				case GuiInstancePropertyInfo::SupportCollection:
					writer.WriteString(L"[collection] ");
					break;
				case GuiInstancePropertyInfo::SupportArray:
					writer.WriteString(L"[array]      ");
					break;
				case GuiInstancePropertyInfo::SupportSet:
					writer.WriteString(L"[set]        ");
					break;
				default:;
				}

				switch (acceptableTypes.Count())
				{
				case 0:
					writer.WriteLine(L"<UNKNOWN-TYPE>");
					break;
				case 1:
					writer.WriteLine(acceptableTypes[0]);
					break;
				default:
					writer.WriteLine(L"{");
					FOREACH(WString, typeName, acceptableTypes)
					{
						writer.WriteLine(L"            " + typeName + L",");
					}
					writer.WriteLine(L"        }");
				}
			}
		}

/***********************************************************************
LogInstanceLoaderManager_PrintProperties
***********************************************************************/

		void LogInstanceLoaderManager_PrintEvents(stream::TextWriter& writer, const WString& typeName)
		{
			List<IGuiInstanceLoader*> loaders;
			{
				IGuiInstanceLoader* loader = GetInstanceLoaderManager()->GetLoader(GlobalStringKey::Get(typeName));
				while (loader)
				{
					loaders.Add(loader);
					loader = GetInstanceLoaderManager()->GetParentLoader(loader);
				}
			}
			
			IGuiInstanceLoader::TypeInfo typeInfo(GlobalStringKey::Get(typeName), GetInstanceLoaderManager()->GetTypeDescriptorForType(GlobalStringKey::Get(typeName)));
			Dictionary<GlobalStringKey, IGuiInstanceLoader*> eventLoaders;
			FOREACH(IGuiInstanceLoader*, loader, loaders)
			{
				List<GlobalStringKey> eventNames;
				loader->GetEventNames(typeInfo, eventNames);

				FOREACH(GlobalStringKey, eventName, eventNames)
				{
					if (!eventLoaders.Keys().Contains(eventName))
					{
						eventLoaders.Add(eventName, loader);
					}
				}
			}

			FOREACH_INDEXER(GlobalStringKey, eventName, index, eventLoaders.Keys())
			{
				IGuiInstanceLoader* loader = eventLoaders.Values()[index];
				IGuiInstanceLoader::PropertyInfo propertyInfo(typeInfo, eventName);
				auto info = loader->GetEventType(propertyInfo);
				if (info->support == GuiInstanceEventInfo::NotSupport)
				{
					continue;
				}

				LogInstanceLoaderManager_PrintFieldName(writer, eventName.ToString());
				writer.WriteString(L" [event]      ");
				writer.WriteLine(info->argumentType->GetTypeName());
			}
		}

/***********************************************************************
LogInstanceLoaderManager_PrintSerializableType
***********************************************************************/

		void LogInstanceLoaderManager_PrintSerializableType(stream::TextWriter& writer, const WString& typeName)
		{
			if (ITypeDescriptor* type = GetGlobalTypeManager()->GetTypeDescriptor(typeName))
			{
				if (IValueSerializer* serializer = type->GetValueSerializer())
				{
					if (serializer->HasCandidate())
					{
						if (serializer->CanMergeCandidate())
						{
							writer.WriteLine(L"    enum " + typeName + L" = {" + serializer->GetDefaultText() + L"}");
						}
						else
						{
							writer.WriteLine(L"    flags " + typeName + L" = {" + serializer->GetDefaultText() + L"}");
						}

						writer.WriteLine(L"    {");
						vint count = serializer->GetCandidateCount();
						for (vint i = 0; i < count; i++)
						{
							writer.WriteLine(L"        " + serializer->GetCandidate(i) + L",");
						}
						writer.WriteLine(L"    }");
						return;
					}
					else if (type->GetPropertyCount() > 0)
					{
						writer.WriteLine(L"    struct "+ typeName +  + L" = {" + serializer->GetDefaultText() + L"}");
						writer.WriteLine(L"    {");
						vint count = type->GetPropertyCount();
						for (vint i = 0; i < count; i++)
						{
							IPropertyInfo* prop = type->GetProperty(i);
							LogInstanceLoaderManager_PrintFieldName(writer, prop->GetName());
							writer.WriteLine(prop->GetReturn()->GetTypeFriendlyName() + L";");
						}
						writer.WriteLine(L"    }");
						return;
					}
					else
					{
						writer.WriteLine(L"    data "+ typeName +  + L" = {" + serializer->GetDefaultText() + L"}");
						return;
					}
				}
			}
			writer.WriteLine(L"    serializable " + typeName);
		}

/***********************************************************************
LogInstanceLoaderManager_PrintConstructableType
***********************************************************************/

		void LogInstanceLoaderManager_PrintConstructableType(stream::TextWriter& writer, const WString& typeName)
		{
			writer.WriteLine(L"    class " + typeName);
			LogInstanceLoaderManager_PrintParentTypes(writer, typeName);
			writer.WriteLine(L"    {");
			LogInstanceLoaderManager_PrintProperties(writer, typeName);
			LogInstanceLoaderManager_PrintEvents(writer, typeName);
			writer.WriteLine(L"    }");
		}

/***********************************************************************
LogInstanceLoaderManager_PrintUnconstructableParentType
***********************************************************************/

		void LogInstanceLoaderManager_PrintUnconstructableParentType(stream::TextWriter& writer, const WString& typeName)
		{
			writer.WriteLine(L"    abstract class " + typeName);
			LogInstanceLoaderManager_PrintParentTypes(writer, typeName);
			writer.WriteLine(L"    {");
			writer.WriteLine(L"    }");
		}

/***********************************************************************
LogInstanceLoaderManager_Others
***********************************************************************/

		void LogInstanceLoaderManager_PrintVirtualizedType(stream::TextWriter& writer, const WString& typeName)
		{
			writer.WriteLine(L"    abstract class " + typeName);
		}

		void LogInstanceLoaderManager_PrintUnconstructableType(stream::TextWriter& writer, const WString& typeName)
		{
			writer.WriteLine(L"    abstract class " + typeName);
		}

		void LogInstanceLoaderManager_PrintInterfaceType(stream::TextWriter& writer, const WString& typeName)
		{
			writer.WriteLine(L"    interface " + typeName);
		}

		void LogInstanceLoaderManager_PrintInterfaceConstructableType(stream::TextWriter& writer, const WString& typeName)
		{
			writer.WriteLine(L"    interface " + typeName);
		}

/***********************************************************************
LogInstanceLoaderManager
***********************************************************************/

		void LogInstanceLoaderManager(stream::TextWriter& writer)
		{
			SortedList<WString> allTypes, virtualizedTypes;
			Group<WString, WString> typeParents, typeChildren;

			// collect types
			{
				vint typeCount = GetGlobalTypeManager()->GetTypeDescriptorCount();
				for (vint i = 0; i < typeCount; i++)
				{
					ITypeDescriptor* type = GetGlobalTypeManager()->GetTypeDescriptor(i);
					allTypes.Add(type->GetTypeName());

					vint parentCount = type->GetBaseTypeDescriptorCount();
					for (vint j = 0; j < parentCount; j++)
					{
						ITypeDescriptor* parent = type->GetBaseTypeDescriptor(j);
						typeParents.Add(type->GetTypeName(), parent->GetTypeName());
						typeChildren.Add(parent->GetTypeName(), type->GetTypeName());
					}
				}

				List<GlobalStringKey> virtualTypes;
				GetInstanceLoaderManager()->GetVirtualTypes(virtualTypes);
				FOREACH(GlobalStringKey, typeName, virtualTypes)
				{
					GlobalStringKey parentType = GetInstanceLoaderManager()->GetParentTypeForVirtualType(typeName);
					if (description::GetTypeDescriptor(parentType.ToString()) && !virtualizedTypes.Contains(parentType.ToString()))
					{
						virtualizedTypes.Add(parentType.ToString());
					}
					allTypes.Add(typeName.ToString());
					typeParents.Add(typeName.ToString(), parentType.ToString());
					typeChildren.Add(parentType.ToString(), typeName.ToString());
				}
			}

			// sort types
			List<WString> sortedTypes;
			{
				FOREACH(WString, typeName, allTypes)
				{
					if (!typeParents.Contains(typeName))
					{
						sortedTypes.Add(typeName);
					}
				}

				for (vint i = 0; i < sortedTypes.Count(); i++)
				{
					WString selectedType = sortedTypes[i];
					vint index = typeChildren.Keys().IndexOf(selectedType);
					if (index != -1)
					{
						FOREACH(WString, childType, typeChildren.GetByIndex(index))
						{
							typeParents.Remove(childType, selectedType);
							if (!typeParents.Contains(childType))
							{
								sortedTypes.Add(childType);
							}
						}
						typeChildren.Remove(selectedType);
					}
				}
			}

			// categorize types
			List<WString> serializableTypes;
			List<WString> constructableTypes;
			List<WString> unconstructableParentTypes;
			List<WString> unconstructableTypes;
			List<WString> interfaceTypes;
			List<WString> interfaceConstructableTypes;
			{
				FOREACH(WString, typeName, sortedTypes)
				{
					auto typeKey = GlobalStringKey::Get(typeName);
					auto typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(typeKey);
					IGuiInstanceLoader::TypeInfo typeInfo(typeKey, typeDescriptor);
					
					auto loader = GetInstanceLoaderManager()->GetLoader(typeKey);
					while (loader)
					{
						if (loader->IsDeserializable(typeInfo))
						{
							serializableTypes.Add(typeName);
							break;
						}
						else if (loader->IsCreatable(typeInfo))
						{
							constructableTypes.Add(typeName);
							break;
						}
						else
						{
							loader = GetInstanceLoaderManager()->GetParentLoader(loader);
						}
					}
					if (!loader && !virtualizedTypes.Contains(typeName))
					{
						bool acceptProxy = false;
						if (typeDescriptor->GetTypeName() == typeName && IsInterfaceType(typeDescriptor, acceptProxy))
						{
							if (acceptProxy)
							{
								interfaceConstructableTypes.Add(typeName);
							}
							else
							{
								interfaceTypes.Add(typeName);
							}
						}
						else
						{
							unconstructableTypes.Add(typeName);
						}
					}
				}

				List<WString> parentTypes;
				FOREACH(WString, typeName, constructableTypes)
				{
					parentTypes.Add(typeName);
				}
				for (vint i = 0; i < parentTypes.Count(); i++)
				{
					LogInstanceLoaderManager_GetParentTypes(parentTypes[i], parentTypes);
				}

				for (vint i = unconstructableTypes.Count() - 1; i >= 0; i--)
				{
					WString selectedType = unconstructableTypes[i];
					if (parentTypes.Contains(selectedType))
					{
						unconstructableTypes.RemoveAt(i);
						unconstructableParentTypes.Insert(0, selectedType);
					}
				}
			}

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Serializable Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, serializableTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintSerializableType(writer, typeName);
			}
			writer.WriteLine(L"");

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Constructable Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, constructableTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintConstructableType(writer, typeName);
			}
			writer.WriteLine(L"");

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Unconstructable Parent Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, unconstructableParentTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintUnconstructableParentType(writer, typeName);
			}
			writer.WriteLine(L"");

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Unconstructable Virtualized Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, virtualizedTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintVirtualizedType(writer, typeName);
			}
			writer.WriteLine(L"");

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Unconstructable Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, unconstructableTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintUnconstructableType(writer, typeName);
			}
			writer.WriteLine(L"");

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Interface Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, interfaceTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintInterfaceType(writer, typeName);
			}
			writer.WriteLine(L"");

			writer.WriteLine(L"/***********************************************************************");
			writer.WriteLine(L"Interface Constructable Types");
			writer.WriteLine(L"***********************************************************************/");
			FOREACH(WString, typeName, interfaceConstructableTypes)
			{
				writer.WriteLine(L"");
				LogInstanceLoaderManager_PrintInterfaceConstructableType(writer, typeName);
			}
			writer.WriteLine(L"");
		}
	}
}