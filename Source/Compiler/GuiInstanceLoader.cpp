#include "GuiInstanceLoader.h"
#include "../Reflection/TypeDescriptors/GuiReflectionEvents.h"
#include "../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../Resources/GuiParserManager.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing;
		using namespace parsing::xml;
		using namespace parsing::tabling;
		using namespace controls;
		using namespace regex;
		using namespace reflection::description;
		using namespace stream;
		using namespace workflow;
		using namespace workflow::analyzer;

/***********************************************************************
GuiInstancePropertyInfo
***********************************************************************/

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Unsupported()
		{
			return new GuiInstancePropertyInfo;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Assign(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstancePropertyInfo> info = new GuiInstancePropertyInfo;
			info->support = SupportAssign;
			if (typeDescriptor) info->acceptableTypes.Add(typeDescriptor);
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::AssignWithParent(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstancePropertyInfo> info = Assign(typeDescriptor);
			info->tryParent = true;
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Collection(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstancePropertyInfo> info = Assign(typeDescriptor);
			info->support = SupportCollection;
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::CollectionWithParent(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstancePropertyInfo> info = Collection(typeDescriptor);
			info->tryParent = true;
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Set(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstancePropertyInfo> info = new GuiInstancePropertyInfo;
			info->support = SupportSet;
			if (typeDescriptor) info->acceptableTypes.Add(typeDescriptor);
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Array(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstancePropertyInfo> info = new GuiInstancePropertyInfo;
			info->support = SupportArray;
			if (typeDescriptor) info->acceptableTypes.Add(typeDescriptor);
			return info;
		}

/***********************************************************************
IGuiInstanceLoader
***********************************************************************/

		void IGuiInstanceLoader::GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)
		{
		}

		void IGuiInstanceLoader::GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)
		{
		}

		void IGuiInstanceLoader::GetPairedProperties(const PropertyInfo& propertyInfo, collections::List<GlobalStringKey>& propertyNames)
		{
		}

		Ptr<GuiInstancePropertyInfo> IGuiInstanceLoader::GetPropertyType(const PropertyInfo& propertyInfo)
		{
			return nullptr;
		}

		bool IGuiInstanceLoader::CanCreate(const TypeInfo& typeInfo)
		{
			return false;
		}

		Ptr<workflow::WfBaseConstructorCall> IGuiInstanceLoader::CreateRootInstance(const TypeInfo& typeInfo, Ptr<workflow::WfExpression> controlTemplate, collections::List<WString>& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::CreateControlTemplateArgument(const TypeInfo&, Ptr<workflow::WfExpression>, collections::List<WString>&)#This function is not implemented.");
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::CreateInstance(const TypeInfo&, GlobalStringKey, ArgumentMap&, collections::List<WString>&)#This function is not implemented.");
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::AssignParameters(const TypeInfo&, GlobalStringKey, ArgumentMap&, collections::List<WString>&)#This function is not implemented.");
		}

		Ptr<workflow::WfExpression> IGuiInstanceLoader::GetParameter(const PropertyInfo& propertyInfo, GlobalStringKey variableName, collections::List<WString>& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::GetParameter(const PropertyInfo&, GlobalStringKey, collections::List<WString>&)#This function is not implemented.");
		}

/***********************************************************************
GuiInstanceContext::ElementName Parser
***********************************************************************/

		class GuiInstanceContextElementNameParser : public Object, public IGuiParser<GuiInstanceContext::ElementName>
		{
			typedef GuiInstanceContext::ElementName			ElementName;
		public:
			Regex						regexElementName;

			GuiInstanceContextElementNameParser()
				:regexElementName(L"((<namespaceName>[a-zA-Z_]/w*):)?((<category>[a-zA-Z_]/w*).)?(<name>[a-zA-Z_]/w*)(-(<binding>[a-zA-Z_]/w*))?")
			{
			}

			Ptr<ElementName> TypedParse(const WString& text, collections::List<WString>& errors)override
			{
				Ptr<RegexMatch> match = regexElementName.MatchHead(text);
				if (!match || match->Result().Length() != text.Length())
				{
					errors.Add(L"Failed to parse an element name \"" + text + L"\".");
					return 0;
				}

				Ptr<ElementName> elementName = new ElementName;
				if (match->Groups().Keys().Contains(L"namespaceName"))
				{
					elementName->namespaceName = match->Groups()[L"namespaceName"][0].Value();
				}
				if (match->Groups().Keys().Contains(L"category"))
				{
					elementName->category = match->Groups()[L"category"][0].Value();
				}
				if (match->Groups().Keys().Contains(L"name"))
				{
					elementName->name = match->Groups()[L"name"][0].Value();
				}
				if (match->Groups().Keys().Contains(L"binding"))
				{
					elementName->binding = match->Groups()[L"binding"][0].Value();
				}
				return elementName;
			}
		};

/***********************************************************************
GuiDefaultInstanceLoader
***********************************************************************/

		class GuiDefaultInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			typedef Tuple<ITypeDescriptor*, GlobalStringKey>				FieldKey;
			typedef Tuple<Ptr<GuiInstancePropertyInfo>, IPropertyInfo*>		PropertyType;

			Dictionary<FieldKey, PropertyType>								propertyTypes;

		public:
			static IMethodInfo* GetDefaultConstructor(ITypeDescriptor* typeDescriptor)
			{
				if (auto ctors = typeDescriptor->GetConstructorGroup())
				{
					vint count = ctors->GetMethodCount();
					for (vint i = 0; i < count; i++)
					{
						IMethodInfo* method = ctors->GetMethod(i);
						if (method->GetParameterCount() == 0)
						{
							return method;
						}
					}
				}
				return 0;
			}

			GlobalStringKey GetTypeName()override
			{
				return GlobalStringKey::Empty;
			}

			//***********************************************************************************

			void ProcessGenericType(ITypeInfo* propType, ITypeInfo*& genericType, ITypeInfo*& elementType, bool& readableList, bool& writableList, bool& collectionType)
			{
				genericType = 0;
				elementType = 0;
				readableList = false;
				writableList = false;
				collectionType = false;
				if (propType->GetDecorator() == ITypeInfo::SharedPtr && propType->GetElementType()->GetDecorator() == ITypeInfo::Generic)
				{
					propType = propType->GetElementType();
					genericType = propType->GetElementType();
					if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueList>())
					{
						elementType = propType->GetGenericArgument(0);
						readableList = true;
						writableList = true;
						collectionType = true;
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueEnumerator>())
					{
						collectionType = true;
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueEnumerable>())
					{
						elementType = propType->GetGenericArgument(0);
						readableList = true;
						collectionType = true;
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueReadonlyList>())
					{
						elementType = propType->GetGenericArgument(0);
						readableList = true;
						collectionType = true;
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueReadonlyDictionary>())
					{
						collectionType = true;
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueDictionary>())
					{
						collectionType = true;
					}
				}
			}

			ITypeInfo* GetPropertyReflectionTypeInfo(const PropertyInfo& propertyInfo, GuiInstancePropertyInfo::Support& support)
			{
				support = GuiInstancePropertyInfo::NotSupport;
				IPropertyInfo* prop = propertyInfo.typeInfo.typeDescriptor->GetPropertyByName(propertyInfo.propertyName.ToString(), true);
				if (prop)
				{
					ITypeInfo* propType = prop->GetReturn();
					ITypeInfo* genericType = 0;
					ITypeInfo* elementType = 0;
					bool readableList = false;
					bool writableList = false;
					bool collectionType = false;
					ProcessGenericType(propType, genericType, elementType, readableList, writableList, collectionType);

					if (prop->IsWritable())
					{
						if (collectionType)
						{
							if (readableList)
							{
								support = GuiInstancePropertyInfo::SupportArray;
								return elementType;
							}
						}
						else if (genericType)
						{
							support = GuiInstancePropertyInfo::SupportAssign;
							return genericType;
						}
						else
						{
							support = GuiInstancePropertyInfo::SupportAssign;
							return propType;
						}
					}
					else if (prop->IsReadable())
					{
						if (collectionType)
						{
							if (writableList)
							{
								support = GuiInstancePropertyInfo::SupportCollection;
								return elementType;
							}
						}
						else if (!genericType)
						{
							if (propType->GetDecorator() == ITypeInfo::SharedPtr || propType->GetDecorator() == ITypeInfo::RawPtr)
							{
								support = GuiInstancePropertyInfo::SupportSet;
								return propType;
							}
						}
					}
				}
				return 0;
			}

			bool FillPropertyInfo(Ptr<GuiInstancePropertyInfo> propertyInfo, ITypeInfo* propType)
			{
				switch (propType->GetDecorator())
				{
				case ITypeInfo::RawPtr:
				case ITypeInfo::SharedPtr:
				case ITypeInfo::Nullable:
					FillPropertyInfo(propertyInfo, propType->GetElementType());
					return true;
				case ITypeInfo::TypeDescriptor:
					propertyInfo->acceptableTypes.Add(propType->GetTypeDescriptor());
					return true;
				default:;
				}
				return false;
			}

			void CollectPropertyNames(const TypeInfo& typeInfo, ITypeDescriptor* typeDescriptor, collections::List<GlobalStringKey>& propertyNames)
			{
				vint propertyCount = typeDescriptor->GetPropertyCount();
				for (vint i = 0; i < propertyCount; i++)
				{
					GlobalStringKey propertyName = GlobalStringKey::Get(typeDescriptor->GetProperty(i)->GetName());
					if (!propertyNames.Contains(propertyName))
					{
						auto info = GetPropertyType(PropertyInfo(typeInfo, propertyName));
						if (info && info->support != GuiInstancePropertyInfo::NotSupport)
						{
							propertyNames.Add(propertyName);
						}
					}
				}

				vint parentCount = typeDescriptor->GetBaseTypeDescriptorCount();
				for (vint i = 0; i < parentCount; i++)
				{
					CollectPropertyNames(typeInfo, typeDescriptor->GetBaseTypeDescriptor(i), propertyNames);
				}
			}

			bool ContainsViewModels(const TypeInfo& typeInfo)
			{
				if (auto ctors = typeInfo.typeDescriptor->GetConstructorGroup())
				{
					if (ctors->GetMethodCount() == 1)
					{
						IMethodInfo* method = ctors->GetMethod(0);
						vint count = method->GetParameterCount();
						for (vint i = 0; i < count; i++)
						{
							auto parameter = method->GetParameter(i);
							auto prop = typeInfo.typeDescriptor->GetPropertyByName(parameter->GetName(), false);
							if (!prop || !prop->GetGetter() || prop->GetSetter() || prop->GetValueChangedEvent()) return false;
							if (parameter->GetType()->GetTypeFriendlyName() != prop->GetReturn()->GetTypeFriendlyName()) return false;
						}
						return true;
					}
				}
				return false;
			}

			//***********************************************************************************

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				CollectPropertyNames(typeInfo, typeInfo.typeDescriptor, propertyNames);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (ContainsViewModels(typeInfo))
				{
					IMethodInfo* method = typeInfo.typeDescriptor->GetConstructorGroup()->GetMethod(0);
					vint count = method->GetParameterCount();
					for (vint i = 0; i < count; i++)
					{
						propertyNames.Add(GlobalStringKey::Get(method->GetParameter(i)->GetName()));
					}
				}
			}

			PropertyType GetPropertyTypeCached(const PropertyInfo& propertyInfo)
			{
				FieldKey key(propertyInfo.typeInfo.typeDescriptor, propertyInfo.propertyName);
				vint index = propertyTypes.Keys().IndexOf(key);
				if (index == -1)
				{
					GuiInstancePropertyInfo::Support support = GuiInstancePropertyInfo::NotSupport;
					if (ITypeInfo* propType = GetPropertyReflectionTypeInfo(propertyInfo, support))
					{
						Ptr<GuiInstancePropertyInfo> result = new GuiInstancePropertyInfo;
						result->support = support;

						if (ContainsViewModels(propertyInfo.typeInfo))
						{
							IMethodInfo* method = propertyInfo.typeInfo.typeDescriptor->GetConstructorGroup()->GetMethod(0);
							vint count = method->GetParameterCount();
							for (vint i = 0; i < count; i++)
							{
								if (method->GetParameter(i)->GetName() == propertyInfo.propertyName.ToString())
								{
									result->scope = GuiInstancePropertyInfo::ViewModel;
								}
							}
						}

						if (FillPropertyInfo(result, propType))
						{
							IPropertyInfo* prop = propertyInfo.typeInfo.typeDescriptor->GetPropertyByName(propertyInfo.propertyName.ToString(), true);
							PropertyType value(result, prop);
							propertyTypes.Add(key, value);
							return value;
						}
					}
					
					PropertyType value(GuiInstancePropertyInfo::Unsupported(), 0);
					propertyTypes.Add(key, value);
					return value;
				}
				else
				{
					return propertyTypes.Values()[index];
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
			{
				return GetPropertyTypeCached(propertyInfo).f0;
			}

			//***********************************************************************************

			bool CanCreate(const TypeInfo& typeInfo)override
			{
				return GetDefaultConstructor(typeInfo.typeDescriptor) != 0;
			}

			Ptr<workflow::WfStatement> CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
			{
				auto create = MakePtr<WfNewClassExpression>();
				create->type = GetTypeFromTypeInfo(GetDefaultConstructor(typeInfo.typeDescriptor)->GetReturn());

				auto refValue = MakePtr<WfReferenceExpression>();
				refValue->name.value = variableName.ToString();

				auto assign = MakePtr<WfBinaryExpression>();
				assign->op = WfBinaryOperator::Assign;
				assign->first = refValue;
				assign->second = create;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = assign;
				return stat;
			}

			Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
			{
				auto block = MakePtr<WfBlockStatement>();

				FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
				{
					PropertyType propertyType = GetPropertyTypeCached(PropertyInfo(typeInfo, prop));
					if (propertyType.f1)
					{
						switch (propertyType.f0->support)
						{
						case GuiInstancePropertyInfo::SupportCollection:
							{
								const auto& values = arguments.GetByIndex(index);
								if (values.Count() > 0)
								{
									{
										auto refValue = MakePtr<WfReferenceExpression>();
										refValue->name.value = variableName.ToString();

										auto refProp = MakePtr<WfMemberExpression>();
										refProp->parent = refValue;
										refProp->name.value = prop.ToString();

										auto varDesc = MakePtr<WfVariableDeclaration>();
										varDesc->name.value = L"<collection>";
										varDesc->expression = refProp;

										auto stat = MakePtr<WfVariableStatement>();
										stat->variable = varDesc;
										block->statements.Add(stat);
									}

									for (vint i = 0; i < values.Count(); i++)
									{
										auto refCollection = MakePtr<WfReferenceExpression>();
										refCollection->name.value = L"<collection>";

										auto refAdd = MakePtr<WfMemberExpression>();
										refAdd->parent = refCollection;
										refAdd->name.value = L"Add";

										auto call = MakePtr<WfCallExpression>();
										call->function = refAdd;
										call->arguments.Add(values[i].expression);

										auto stat = MakePtr<WfExpressionStatement>();
										stat->expression = call;
										block->statements.Add(stat);
									}
								}
							}
							break;
						case GuiInstancePropertyInfo::SupportArray:
							{
								auto refArray = MakePtr<WfConstructorExpression>();
								FOREACH(ArgumentInfo, item, arguments.GetByIndex(index))
								{
									auto argument = MakePtr<WfConstructorArgument>();
									argument->key = item.expression;
									refArray->arguments.Add(argument);
								}

								auto refValue = MakePtr<WfReferenceExpression>();
								refValue->name.value = variableName.ToString();

								auto refProp = MakePtr<WfMemberExpression>();
								refProp->parent = refValue;
								refProp->name.value = prop.ToString();

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refProp;
								assign->second = refArray;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;
								block->statements.Add(stat);
							}
							break;
						case GuiInstancePropertyInfo::SupportAssign:
							{
								auto refValue = MakePtr<WfReferenceExpression>();
								refValue->name.value = variableName.ToString();

								auto refProp = MakePtr<WfMemberExpression>();
								refProp->parent = refValue;
								refProp->name.value = prop.ToString();

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refProp;
								assign->second = arguments.GetByIndex(index)[0].expression;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;
								block->statements.Add(stat);
							}
							break;
						default:;
						}
					}
				}

				if (block->statements.Count() > 0)
				{
					return block;
				}
				return nullptr;
			}

			Ptr<workflow::WfExpression> GetParameter(const PropertyInfo& propertyInfo, GlobalStringKey variableName, collections::List<WString>& errors)override
			{
				auto refValue = MakePtr<WfReferenceExpression>();
				refValue->name.value = variableName.ToString();

				auto refProp = MakePtr<WfMemberExpression>();
				refProp->parent = refValue;
				refProp->name.value = propertyInfo.propertyName.ToString();

				return refProp;
			}
		};

/***********************************************************************
FindInstanceLoadingSource
***********************************************************************/

		InstanceLoadingSource FindInstanceLoadingSource(
			Ptr<GuiInstanceContext> context,
			GuiConstructorRepr* ctor
			)
		{
			vint index=context->namespaces.Keys().IndexOf(ctor->typeNamespace);
			if(index!=-1)
			{
				Ptr<GuiInstanceContext::NamespaceInfo> namespaceInfo=context->namespaces.Values()[index];
				FOREACH(Ptr<GuiInstanceNamespace>, ns, namespaceInfo->namespaces)
				{
					auto fullName = GlobalStringKey::Get(ns->prefix + ctor->typeName.ToString() + ns->postfix);
					IGuiInstanceLoader* loader = GetInstanceLoaderManager()->GetLoader(fullName);
					if(loader)
					{
						return InstanceLoadingSource(loader, fullName);
					}
				}
			}
			return InstanceLoadingSource();
		}

/***********************************************************************
GuiInstanceLoaderManager
***********************************************************************/

		IGuiInstanceLoaderManager* instanceLoaderManager = 0;

		IGuiInstanceLoaderManager* GetInstanceLoaderManager()
		{
			return instanceLoaderManager;
		}

		class GuiInstanceLoaderManager : public Object, public IGuiInstanceLoaderManager, public IGuiPlugin
		{
		protected:
			typedef Dictionary<GlobalStringKey, Ptr<IGuiInstanceBinder>>				BinderMap;
			typedef Dictionary<GlobalStringKey, Ptr<IGuiInstanceEventBinder>>			EventBinderMap;

			struct VirtualTypeInfo
			{
				GlobalStringKey						typeName;
				ITypeDescriptor*					typeDescriptor = nullptr;
				GlobalStringKey						parentTypeName;				// for virtual type only
				Ptr<IGuiInstanceLoader>				loader;

				List<ITypeDescriptor*>				parentTypes;				// all direct or indirect base types that does not has a type info
				List<VirtualTypeInfo*>				parentTypeInfos;			// type infos for all registered direct or indirect base types
			};

			typedef Dictionary<GlobalStringKey, Ptr<VirtualTypeInfo>>		VirtualTypeInfoMap;

			Ptr<IGuiInstanceLoader>					rootLoader;
			BinderMap								binders;
			EventBinderMap							eventBinders;
			VirtualTypeInfoMap						typeInfos;

			bool IsTypeExists(GlobalStringKey name)
			{
				return GetGlobalTypeManager()->GetTypeDescriptor(name.ToString()) != 0 || typeInfos.Keys().Contains(name);
			}

			void FindParentTypeInfos(Ptr<VirtualTypeInfo> typeInfo, ITypeDescriptor* searchType)
			{
				if (searchType != typeInfo->typeDescriptor)
				{
					vint index = typeInfos.Keys().IndexOf(GlobalStringKey::Get(searchType->GetTypeName()));
					if (index == -1)
					{
						typeInfo->parentTypes.Add(searchType);
					}
					else
					{
						typeInfo->parentTypeInfos.Add(typeInfos.Values()[index].Obj());
						return;
					}
				}

				vint count = searchType->GetBaseTypeDescriptorCount();
				for (vint i = 0; i < count; i++)
				{
					ITypeDescriptor* baseType = searchType->GetBaseTypeDescriptor(i);
					FindParentTypeInfos(typeInfo, baseType);
				}
			}

			void FillParentTypeInfos(Ptr<VirtualTypeInfo> typeInfo)
			{
				if (typeInfo->parentTypeName != GlobalStringKey::Empty)
				{
					typeInfo->typeDescriptor = nullptr;
				}
				typeInfo->parentTypes.Clear();
				typeInfo->parentTypeInfos.Clear();

				ITypeDescriptor* searchType = typeInfo->typeDescriptor;
				if (!searchType)
				{
					vint index = typeInfos.Keys().IndexOf(typeInfo->parentTypeName);
					if (index == -1)
					{
						searchType = GetGlobalTypeManager()->GetTypeDescriptor(typeInfo->parentTypeName.ToString());
						typeInfo->typeDescriptor = searchType;
						typeInfo->parentTypes.Add(searchType);
					}
					else
					{
						VirtualTypeInfo* parentTypeInfo = typeInfos.Values()[index].Obj();
						typeInfo->typeDescriptor = parentTypeInfo->typeDescriptor;
						typeInfo->parentTypeInfos.Add(parentTypeInfo);
						return;
					}
				}

				if (searchType)
				{
					FindParentTypeInfos(typeInfo, searchType);
				}
			}

			IGuiInstanceLoader* GetLoaderFromType(ITypeDescriptor* typeDescriptor)
			{
				vint index = typeInfos.Keys().IndexOf(GlobalStringKey::Get(typeDescriptor->GetTypeName()));
				if (index == -1)
				{
					vint count = typeDescriptor->GetBaseTypeDescriptorCount();
					for (vint i = 0; i < count; i++)
					{
						ITypeDescriptor* baseType = typeDescriptor->GetBaseTypeDescriptor(i);
						IGuiInstanceLoader* loader = GetLoaderFromType(baseType);
						if (loader) return loader;
					}
					return 0;
				}
				else
				{
					return typeInfos.Values()[index]->loader.Obj();
				}
			}
		public:
			GuiInstanceLoaderManager()
			{
				rootLoader = new GuiDefaultInstanceLoader;
			}

			void Load()override
			{
				instanceLoaderManager = this;
			}

			void AfterLoad()override
			{
				IGuiParserManager* manager = GetParserManager();
				manager->SetParser(L"INSTANCE-ELEMENT-NAME", new GuiInstanceContextElementNameParser);
			}

			void Unload()override
			{
				instanceLoaderManager = nullptr;
			}

			bool AddInstanceBinder(Ptr<IGuiInstanceBinder> binder)override
			{
				if (binders.Keys().Contains(binder->GetBindingName())) return false;
				binders.Add(binder->GetBindingName(), binder);
				return true;
			}

			IGuiInstanceBinder* GetInstanceBinder(GlobalStringKey bindingName)override
			{
				vint index = binders.Keys().IndexOf(bindingName);
				return index == -1 ? 0 : binders.Values()[index].Obj();
			}

			bool AddInstanceEventBinder(Ptr<IGuiInstanceEventBinder> binder)override
			{
				if (eventBinders.Keys().Contains(binder->GetBindingName())) return false;
				eventBinders.Add(binder->GetBindingName(), binder);
				return true;
			}

			IGuiInstanceEventBinder* GetInstanceEventBinder(GlobalStringKey bindingName)override
			{
				vint index = eventBinders.Keys().IndexOf(bindingName);
				return index == -1 ? 0 : eventBinders.Values()[index].Obj();
			}

			bool CreateVirtualType(GlobalStringKey parentType, Ptr<IGuiInstanceLoader> loader)override
			{
				if (IsTypeExists(loader->GetTypeName()) || !IsTypeExists(parentType)) return false;

				Ptr<VirtualTypeInfo> typeInfo = new VirtualTypeInfo;
				typeInfo->typeName = loader->GetTypeName();
				typeInfo->parentTypeName = parentType;
				typeInfo->loader = loader;
				typeInfos.Add(loader->GetTypeName(), typeInfo);
				FillParentTypeInfos(typeInfo);

				return true;
			}

			bool SetLoader(Ptr<IGuiInstanceLoader> loader)override
			{
				vint index = typeInfos.Keys().IndexOf(loader->GetTypeName());
				if (index != -1) return false;

				ITypeDescriptor* typeDescriptor = GetGlobalTypeManager()->GetTypeDescriptor(loader->GetTypeName().ToString());
				if (typeDescriptor == 0) return false;

				Ptr<VirtualTypeInfo> typeInfo = new VirtualTypeInfo;
				typeInfo->typeName = loader->GetTypeName();
				typeInfo->typeDescriptor = typeDescriptor;
				typeInfo->loader = loader;
				typeInfos.Add(typeInfo->typeName, typeInfo);
				FillParentTypeInfos(typeInfo);

				FOREACH(Ptr<VirtualTypeInfo>, derived, typeInfos.Values())
				{
					if (derived->parentTypes.Contains(typeInfo->typeDescriptor))
					{
						FillParentTypeInfos(derived);
					}
				}

				return true;
			}

			IGuiInstanceLoader* GetLoader(GlobalStringKey typeName)override
			{
				vint index = typeInfos.Keys().IndexOf(typeName);
				if (index != -1)
				{
					return typeInfos.Values()[index]->loader.Obj();
				}

				ITypeDescriptor* typeDescriptor = GetGlobalTypeManager()->GetTypeDescriptor(typeName.ToString());
				if (typeDescriptor)
				{
					IGuiInstanceLoader* loader = GetLoaderFromType(typeDescriptor);
					return loader ? loader : rootLoader.Obj();
				}
				return 0;
			}

			IGuiInstanceLoader* GetParentLoader(IGuiInstanceLoader* loader)override
			{
				vint index = typeInfos.Keys().IndexOf(loader->GetTypeName());
				if (index != -1)
				{
					Ptr<VirtualTypeInfo> typeInfo = typeInfos.Values()[index];
					if (typeInfo->parentTypeInfos.Count() > 0)
					{
						return typeInfo->parentTypeInfos[0]->loader.Obj();
					}
					return rootLoader.Obj();
				}
				return 0;
			}

			description::ITypeDescriptor* GetTypeDescriptorForType(GlobalStringKey typeName)override
			{
				vint index = typeInfos.Keys().IndexOf(typeName);
				return index == -1
					? GetGlobalTypeManager()->GetTypeDescriptor(typeName.ToString())
					: typeInfos.Values()[index]->typeDescriptor;
			}

			void GetVirtualTypes(collections::List<GlobalStringKey>& typeNames)override
			{
				for (vint i = 0; i < typeInfos.Count(); i++)
				{
					if (typeInfos.Values()[i]->parentTypeName != GlobalStringKey::Empty)
					{
						typeNames.Add(typeInfos.Keys()[i]);
					}
				}
			}

			GlobalStringKey GetParentTypeForVirtualType(GlobalStringKey virtualType)override
			{
				vint index = typeInfos.Keys().IndexOf(virtualType);
				if (index != -1)
				{
					auto typeInfo = typeInfos.Values()[index];
					return typeInfo->parentTypeName;
				}
				return GlobalStringKey::Empty;
			}
		};
		GUI_REGISTER_PLUGIN(GuiInstanceLoaderManager)
	}
}
