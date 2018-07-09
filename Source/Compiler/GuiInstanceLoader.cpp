#include "GuiInstanceLoader.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"
#include "../Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../Reflection/GuiInstanceCompiledWorkflow.h"
#include "../Resources/GuiParserManager.h"

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
		using namespace workflow::typeimpl;

/***********************************************************************
GuiInstancePropertyInfo
***********************************************************************/

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Unsupported()
		{
			return new GuiInstancePropertyInfo;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Assign(Ptr<description::ITypeInfo> typeInfo)
		{
			auto info = MakePtr<GuiInstancePropertyInfo>();
			info->support = SupportAssign;
			if (typeInfo) info->acceptableTypes.Add(typeInfo);
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::AssignWithParent(Ptr<description::ITypeInfo> typeInfo)
		{
			auto info = Assign(typeInfo);
			info->mergability = MergeWithParent;
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Collection(Ptr<description::ITypeInfo> typeInfo)
		{
			auto info = Assign(typeInfo);
			info->support = SupportCollection;
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::CollectionWithParent(Ptr<description::ITypeInfo> typeInfo)
		{
			auto info = Collection(typeInfo);
			info->mergability = MergeWithParent;
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Set(Ptr<description::ITypeInfo> typeInfo)
		{
			auto info = MakePtr<GuiInstancePropertyInfo>();
			info->support = SupportSet;
			if (typeInfo) info->acceptableTypes.Add(typeInfo);
			return info;
		}

		Ptr<GuiInstancePropertyInfo> GuiInstancePropertyInfo::Array(Ptr<description::ITypeInfo> typeInfo)
		{
			auto info = MakePtr<GuiInstancePropertyInfo>();
			info->support = SupportArray;
			if (typeInfo) info->acceptableTypes.Add(typeInfo);
			return info;
		}

/***********************************************************************
IGuiInstanceLoader
***********************************************************************/

		void IGuiInstanceLoader::ClearReflectionCache()
		{
		}

		void IGuiInstanceLoader::GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)
		{
		}

		void IGuiInstanceLoader::GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)
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

		Ptr<workflow::WfBaseConstructorCall> IGuiInstanceLoader::CreateRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::CreateRootInstance(GuiResourcePrecompileContext&, types::ResolvingResult&, const TypeInfo&, Ptr<workflow::WfExpression>, ArgumentMap&, GuiResourceError::List&)#This function is not implemented.");
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::InitializeRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors)
		{
			return nullptr;
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::CreateInstance(GuiResourcePrecompileContext&, types::ResolvingResult&, const TypeInfo&, GlobalStringKey, ArgumentMap&, GuiResourceTextPos, GuiResourceError::List&)#This function is not implemented.");
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::AssignParameters(GuiResourcePrecompileContext&, types::ResolvingResult&, const TypeInfo&, GlobalStringKey, ArgumentMap&, GuiResourceTextPos, GuiResourceError::List&)#This function is not implemented.");
		}

		Ptr<workflow::WfExpression> IGuiInstanceLoader::GetParameter(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::GetParameter(GuiResourcePrecompileContext&, types::ResolvingResult&, const PropertyInfo&, GlobalStringKey, GuiResourceTextPos, GuiResourceError::List&)#This function is not implemented.");
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

			Ptr<ElementName> ParseInternal(const WString& text, collections::List<Ptr<parsing::ParsingError>>& errors)override
			{
				Ptr<RegexMatch> match = regexElementName.MatchHead(text);
				if (!match || match->Result().Length() != text.Length())
				{
					errors.Add(MakePtr<ParsingError>(L"Failed to parse an element name \"" + text + L"\"."));
					return nullptr;
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

#define CTOR_PARAM_PREFIX\
		static const wchar_t Prefix[] = L"<ctor-parameter>";\
		static const vint PrefixLength = (vint)sizeof(Prefix) / sizeof(*Prefix) - 1;\

#define CTOR_PARAM_NAME(NAME) (NAME).Right((NAME).Length() - PrefixLength)

		class GuiDefaultInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			typedef Tuple<ITypeDescriptor*, GlobalStringKey>				FieldKey;
			typedef Tuple<Ptr<GuiInstancePropertyInfo>, IPropertyInfo*>		PropertyType;

			Dictionary<FieldKey, PropertyType>								propertyTypes;
			Dictionary<ITypeDescriptor*, IMethodInfo*>						defaultConstructors;
			Dictionary<ITypeDescriptor*, IMethodInfo*>						instanceConstructors;
		public:
			IMethodInfo* GetDefaultConstructor(ITypeDescriptor* typeDescriptor)
			{
				IMethodInfo* ctor = nullptr;
				vint index = defaultConstructors.Keys().IndexOf(typeDescriptor);
				if (index == -1)
				{
					if (auto ctors = typeDescriptor->GetConstructorGroup())
					{
						vint count = ctors->GetMethodCount();
						for (vint i = 0; i < count; i++)
						{
							IMethodInfo* method = ctors->GetMethod(i);
							if (method->GetParameterCount() == 0)
							{
								ctor = method;
								break;
							}
						}
					}
					defaultConstructors.Add(typeDescriptor, ctor);
				}
				else
				{
					ctor = defaultConstructors.Values()[index];
				}
				return ctor;
			}

			IMethodInfo* GetInstanceConstructor(ITypeDescriptor* typeDescriptor)
			{
				CTOR_PARAM_PREFIX
					
				IMethodInfo* ctor = nullptr;
				vint index = instanceConstructors.Keys().IndexOf(typeDescriptor);
				if (index == -1)
				{
					if (dynamic_cast<WfClass*>(typeDescriptor))
					{
						if (auto group = typeDescriptor->GetConstructorGroup())
						{
							if (group->GetMethodCount() == 1)
							{
								auto method = group->GetMethod(0);
								vint count = method->GetParameterCount();
								for (vint i = 0; i < count; i++)
								{
									const auto& name = method->GetParameter(i)->GetName();
									if (name.Length() <= PrefixLength || name.Left(PrefixLength) != Prefix)
									{
										goto FINISHED;
									}

									if (!typeDescriptor->GetPropertyByName(CTOR_PARAM_NAME(name), false))
									{
										goto FINISHED;
									}
								}
								ctor = method;
							}
						}
					}
				FINISHED:
					instanceConstructors.Add(typeDescriptor, ctor);
				}
				else
				{
					ctor = instanceConstructors.Values()[index];
				}
				return ctor;
			}

			GlobalStringKey GetTypeName()override
			{
				return GlobalStringKey::Empty;
			}

			void ClearReflectionCache()override
			{
				propertyTypes.Clear();
				defaultConstructors.Clear();
				instanceConstructors.Clear();
			}

			//***********************************************************************************

			ITypeInfo* ProcessGenericType(ITypeInfo* propType, bool& readableList, bool& writableList, bool& collectionType)
			{
				readableList = false;
				writableList = false;
				collectionType = false;
				if (propType->GetDecorator() == ITypeInfo::SharedPtr && propType->GetElementType()->GetDecorator() == ITypeInfo::Generic)
				{
					auto genericType = propType->GetElementType();
					if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueList>())
					{
						readableList = true;
						writableList = true;
						collectionType = true;
						return genericType->GetGenericArgument(0);
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueObservableList>())
					{
						readableList = true;
						writableList = true;
						collectionType = true;
						return genericType->GetGenericArgument(0);
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueEnumerator>())
					{
						collectionType = true;
						return genericType->GetGenericArgument(0);
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueEnumerable>())
					{
						readableList = true;
						collectionType = true;
						return genericType->GetGenericArgument(0);
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueReadonlyList>())
					{
						readableList = true;
						collectionType = true;
						return genericType->GetGenericArgument(0);
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueReadonlyDictionary>())
					{
						collectionType = true;
						return nullptr;
					}
					else if (genericType->GetTypeDescriptor() == description::GetTypeDescriptor<IValueDictionary>())
					{
						collectionType = true;
						return nullptr;
					}
				}
				return propType;
			}

			ITypeInfo* GetPropertyReflectionTypeInfo(const PropertyInfo& propertyInfo, GuiInstancePropertyInfo::Support& support)
			{
				support = GuiInstancePropertyInfo::NotSupport;
				IPropertyInfo* prop = propertyInfo.typeInfo.typeInfo->GetTypeDescriptor()->GetPropertyByName(propertyInfo.propertyName.ToString(), true);
				if (prop)
				{
					ITypeInfo* propType = prop->GetReturn();
					bool readableList = false;
					bool writableList = false;
					bool collectionType = false;
					auto propValueType = ProcessGenericType(propType, readableList, writableList, collectionType);

					if (prop->IsWritable())
					{
						if (collectionType)
						{
							if (readableList)
							{
								support = GuiInstancePropertyInfo::SupportArray;
								return propValueType;
							}
						}
						else
						{
							support = GuiInstancePropertyInfo::SupportAssign;
							return propValueType;
						}
					}
					else if (prop->IsReadable())
					{
						if (collectionType)
						{
							if (writableList)
							{
								support = GuiInstancePropertyInfo::SupportCollection;
								return propValueType;
							}
						}
						else
						{
							if (propType->GetDecorator() == ITypeInfo::SharedPtr || propType->GetDecorator() == ITypeInfo::RawPtr)
							{
								if (propType->GetElementType()->GetDecorator() != ITypeInfo::Generic)
								{
									support = GuiInstancePropertyInfo::SupportSet;
									return propValueType;
								}
							}
						}
					}
				}
				return nullptr;
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

			//***********************************************************************************

			void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				if (CanCreate(typeInfo))
				{
					CTOR_PARAM_PREFIX

					if (auto ctor = GetInstanceConstructor(typeInfo.typeInfo->GetTypeDescriptor()))
					{
						vint count = ctor->GetParameterCount();
						for (vint i = 0; i < count; i++)
						{
							const auto& name = ctor->GetParameter(i)->GetName();
							propertyNames.Add(GlobalStringKey::Get(CTOR_PARAM_NAME(name)));
						}
					}
				}
			}

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				GetRequiredPropertyNames(typeInfo, propertyNames);
				CollectPropertyNames(typeInfo, typeInfo.typeInfo->GetTypeDescriptor(), propertyNames);
			}

			PropertyType GetPropertyTypeCached(const PropertyInfo& propertyInfo)
			{
				CTOR_PARAM_PREFIX

				FieldKey key(propertyInfo.typeInfo.typeInfo->GetTypeDescriptor(), propertyInfo.propertyName);
				vint index = propertyTypes.Keys().IndexOf(key);
				if (index == -1)
				{
					GuiInstancePropertyInfo::Support support = GuiInstancePropertyInfo::NotSupport;
					if (ITypeInfo* propType = GetPropertyReflectionTypeInfo(propertyInfo, support))
					{
						Ptr<GuiInstancePropertyInfo> result = new GuiInstancePropertyInfo;
						result->support = support;
						result->acceptableTypes.Add(CopyTypeInfo(propType));

						if (auto ctor = GetInstanceConstructor(propertyInfo.typeInfo.typeInfo->GetTypeDescriptor()))
						{
							vint count = ctor->GetParameterCount();
							for (vint i = 0; i < count; i++)
							{
								const auto& name = ctor->GetParameter(i)->GetName();
								if (CTOR_PARAM_NAME(name) == propertyInfo.propertyName.ToString())
								{
									result->usage = GuiInstancePropertyInfo::ConstructorArgument;
									result->bindability = GuiInstancePropertyInfo::Bindable;
								}
							}
						}

						IPropertyInfo* prop = propertyInfo.typeInfo.typeInfo->GetTypeDescriptor()->GetPropertyByName(propertyInfo.propertyName.ToString(), true);
						PropertyType value(result, prop);
						propertyTypes.Add(key, value);
						return value;
					}
					else
					{
						PropertyType value(GuiInstancePropertyInfo::Unsupported(), 0);
						propertyTypes.Add(key, value);
						return value;
					}
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
				return
					GetDefaultConstructor(typeInfo.typeInfo->GetTypeDescriptor()) != nullptr ||
					GetInstanceConstructor(typeInfo.typeInfo->GetTypeDescriptor()) != nullptr;
			}

			Ptr<workflow::WfBaseConstructorCall> CreateRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)override
			{
				CTOR_PARAM_PREFIX

				if (arguments.Count() > 0)
				{
					auto call = MakePtr<WfBaseConstructorCall>();

					auto baseTd = typeInfo.typeInfo->GetTypeDescriptor()->GetBaseTypeDescriptor(0);
					auto baseTypeInfo = MakePtr<TypeDescriptorTypeInfo>(baseTd, TypeInfoHint::Normal);
					call->type = GetTypeFromTypeInfo(baseTypeInfo.Obj());

					auto ctor = baseTd->GetConstructorGroup()->GetMethod(0);
					vint count = ctor->GetParameterCount();
					for (vint i = 0; i < count; i++)
					{
						auto key = GlobalStringKey::Get(CTOR_PARAM_NAME(ctor->GetParameter(0)->GetName()));

						vint index = arguments.Keys().IndexOf(key);
						if (index == -1)
						{
							return nullptr;
						}
						else
						{
							call->arguments.Add(arguments.GetByIndex(index)[0].expression);
						}
					}
					return call;
				}
				return nullptr;
			}

			Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
			{
				CTOR_PARAM_PREFIX
				auto defaultCtor = GetDefaultConstructor(typeInfo.typeInfo->GetTypeDescriptor());
				auto instanceCtor = GetInstanceConstructor(typeInfo.typeInfo->GetTypeDescriptor());

				auto create = MakePtr<WfNewClassExpression>();
				if (defaultCtor)
				{
					create->type = GetTypeFromTypeInfo(defaultCtor->GetReturn());
				}
				else
				{
					create->type = GetTypeFromTypeInfo(instanceCtor->GetReturn());

					vint count = instanceCtor->GetParameterCount();
					for (vint i = 0; i < count; i++)
					{
						const auto& name = instanceCtor->GetParameter(i)->GetName();
						auto key = GlobalStringKey::Get(CTOR_PARAM_NAME(name));

						vint index = arguments.Keys().IndexOf(key);
						if (index == -1)
						{
							return nullptr;
						}
						else
						{
							create->arguments.Add(arguments.GetByIndex(index)[0].expression);
						}
					}
				}

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

			Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
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
								auto& propertyValue = arguments.GetByIndex(index)[0];
								if (propertyValue.expression)
								{
									auto refValue = MakePtr<WfReferenceExpression>();
									refValue->name.value = variableName.ToString();

									auto refProp = MakePtr<WfMemberExpression>();
									refProp->parent = refValue;
									refProp->name.value = prop.ToString();

									auto assign = MakePtr<WfBinaryExpression>();
									assign->op = WfBinaryOperator::Assign;
									assign->first = refProp;
									assign->second = propertyValue.expression;

									auto stat = MakePtr<WfExpressionStatement>();
									stat->expression = assign;
									block->statements.Add(stat);
								}
							}
							break;
						default:
							errors.Add(GuiResourceError({ resolvingResult.resource }, attPosition,
								L"Precompile: Property \"" +
								prop.ToString() +
								L"\" of type \"" +
								typeInfo.typeName.ToString() +
								L"\" is not assignable."));
						}
					}
				}

				if (block->statements.Count() > 0)
				{
					return block;
				}
				return nullptr;
			}

			Ptr<workflow::WfExpression> GetParameter(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
			{
				auto refValue = MakePtr<WfReferenceExpression>();
				refValue->name.value = variableName.ToString();

				auto refProp = MakePtr<WfMemberExpression>();
				refProp->parent = refValue;
				refProp->name.value = propertyInfo.propertyName.ToString();

				return refProp;
			}
		};
#undef CTOR_PARAM_NAME
#undef CTOR_PARAM_PREFIX

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
			typedef List<Ptr<IGuiInstanceDeserializer>>									DeserializerList;

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
			DeserializerList						deserializers;
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

			GUI_PLUGIN_NAME(GacUI_Instance)
			{
				GUI_PLUGIN_DEPEND(GacUI_Parser);
			}

			void Load()override
			{
				instanceLoaderManager = this;
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
				return index == -1 ? nullptr : binders.Values()[index].Obj();
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
				return index == -1 ? nullptr : eventBinders.Values()[index].Obj();
			}

			bool AddInstanceDeserializer(Ptr<IGuiInstanceDeserializer> deserializer)override
			{
				if (deserializers.Contains(deserializer.Obj())) return false;
				deserializers.Add(deserializer);
				return true;
			}

			IGuiInstanceDeserializer* GetInstanceDeserializer(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo)override
			{
				FOREACH(Ptr<IGuiInstanceDeserializer>, deserializer, deserializers)
				{
					if (deserializer->CanDeserialize(propertyInfo, typeInfo))
					{
						return deserializer.Obj();
					}
				}
				return nullptr;
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

			Ptr<description::ITypeInfo> GetTypeInfoForType(GlobalStringKey typeName)override
			{
				vint index = typeInfos.Keys().IndexOf(typeName);
				auto td = index == -1
					? GetGlobalTypeManager()->GetTypeDescriptor(typeName.ToString())
					: typeInfos.Values()[index]->typeDescriptor;
				if (!td) return nullptr;

				if (auto ctor = td->GetConstructorGroup())
				{
					return CopyTypeInfo(ctor->GetMethod(0)->GetReturn());
				}
				else
				{
					return MakePtr<RawPtrTypeInfo>(MakePtr<TypeDescriptorTypeInfo>(td, TypeInfoHint::Normal));
				}
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

			void ClearReflectionCache()override
			{
				rootLoader->ClearReflectionCache();
				FOREACH(Ptr<VirtualTypeInfo>, info, typeInfos.Values())
				{
					info->loader->ClearReflectionCache();
				}
			}
		};
		GUI_REGISTER_PLUGIN(GuiInstanceLoaderManager)

/***********************************************************************
Helper Functions
***********************************************************************/

		void Split(const WString& input, const WString& delimiter, collections::List<WString>& fragments)
		{
			const wchar_t* attValue = input.Buffer();
			while (*attValue)
			{
				// split the value by ';'
				const wchar_t* attSemicolon = wcsstr(attValue, delimiter.Buffer());
				WString pattern;
				if (attSemicolon)
				{
					pattern = WString(attValue, vint(attSemicolon - attValue));
					attValue = attSemicolon + delimiter.Length();
				}
				else
				{
					vint len = wcslen(attValue);
					pattern = WString(attValue, len);
					attValue += len;
				}

				fragments.Add(pattern);
			}
		}

		void SplitTypeName(const WString& input, collections::List<WString>& fragments)
		{
			Split(input, L"::", fragments);
		}

		void SplitBySemicolon(const WString& input, collections::List<WString>& fragments)
		{
			Split(input, L";", fragments);
		}
	}
}
