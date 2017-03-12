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
		using namespace workflow::typeimpl;

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

		void IGuiInstanceLoader::ClearReflectionCache()
		{
		}

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

		Ptr<workflow::WfBaseConstructorCall> IGuiInstanceLoader::CreateRootInstance(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::CreateControlTemplateArgument(types::ResolvingResult&, const TypeInfo&, Ptr<workflow::WfExpression>, collections::List<WString>&)#This function is not implemented.");
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::InitializeRootInstance(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors)
		{
			return nullptr;
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::CreateInstance(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::CreateInstance(types::ResolvingResult&, const TypeInfo&, GlobalStringKey, ArgumentMap&, collections::List<WString>&)#This function is not implemented.");
		}

		Ptr<workflow::WfStatement> IGuiInstanceLoader::AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::AssignParameters(types::ResolvingResult&, const TypeInfo&, GlobalStringKey, ArgumentMap&, collections::List<WString>&)#This function is not implemented.");
		}

		Ptr<workflow::WfExpression> IGuiInstanceLoader::GetParameter(types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, GuiResourceTextPos attPosition, GuiResourceError::List& errors)
		{
			CHECK_FAIL(L"IGuiInstanceLoader::GetParameter(types::ResolvingResult&, const PropertyInfo&, GlobalStringKey, collections::List<WString>&)#This function is not implemented.");
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
					return FillPropertyInfo(propertyInfo, propType->GetElementType());
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

			//***********************************************************************************

			void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				CollectPropertyNames(typeInfo, typeInfo.typeDescriptor, propertyNames);
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
			{
				CTOR_PARAM_PREFIX

				if (auto ctor = GetInstanceConstructor(typeInfo.typeDescriptor))
				{
					vint count = ctor->GetParameterCount();
					for (vint i = 0; i < count; i++)
					{
						const auto& name = ctor->GetParameter(i)->GetName();
						propertyNames.Add(GlobalStringKey::Get(CTOR_PARAM_NAME(name)));
					}
				}
			}
			
			bool IsItemPropertyName(IPropertyInfo* prop)
			{
				auto name = prop->GetName();
				return name.Length() >= 8 && name.Right(8) == L"Property";
			}

			bool IsItemPropertyType(ITypeInfo* propType)
			{
				if (propType->GetDecorator() == ITypeInfo::SharedPtr)
				{
					auto genericType = propType->GetElementType();
					if (genericType->GetDecorator() == ITypeInfo::Generic && genericType->GetGenericArgumentCount() == 2)
					{
						if (genericType->GetElementType()->GetTypeDescriptor() == description::GetTypeDescriptor<IValueFunctionProxy>())
						{
							if (genericType->GetGenericArgument(1)->GetTypeDescriptor() == description::GetTypeDescriptor<Value>())
							{
								return true;
							}
						}
					}
				}
				return false;
			}

			bool IsWritableItemPropertyType(ITypeInfo* propType)
			{
				if (propType->GetDecorator() == ITypeInfo::SharedPtr)
				{
					auto genericType = propType->GetElementType();
					if (genericType->GetDecorator() == ITypeInfo::Generic && genericType->GetGenericArgumentCount() == 4)
					{
						if (genericType->GetElementType()->GetTypeDescriptor() == description::GetTypeDescriptor<IValueFunctionProxy>())
						{
							if (genericType->GetGenericArgument(1)->GetTypeDescriptor() == description::GetTypeDescriptor<Value>()
								&& genericType->GetGenericArgument(3)->GetTypeDescriptor() == description::GetTypeDescriptor<bool>())
							{
								if (IsSameType(genericType->GetGenericArgument(0), genericType->GetGenericArgument(2)))
								{
									return true;
								}
							}
						}
					}
				}
				return false;
			}

			bool IsItemProperty(IPropertyInfo* prop)
			{
				return prop && IsItemPropertyName(prop) && (IsItemPropertyType(prop->GetReturn()) || IsWritableItemPropertyType(prop->GetReturn()));
			}

			PropertyType GetPropertyTypeCached(const PropertyInfo& propertyInfo)
			{
				CTOR_PARAM_PREFIX

				FieldKey key(propertyInfo.typeInfo.typeDescriptor, propertyInfo.propertyName);
				vint index = propertyTypes.Keys().IndexOf(key);
				if (index == -1)
				{
					GuiInstancePropertyInfo::Support support = GuiInstancePropertyInfo::NotSupport;
					if (ITypeInfo* propType = GetPropertyReflectionTypeInfo(propertyInfo, support))
					{
						Ptr<GuiInstancePropertyInfo> result = new GuiInstancePropertyInfo;
						result->support = support;

						if (auto ctor = GetInstanceConstructor(propertyInfo.typeInfo.typeDescriptor))
						{
							vint count = ctor->GetParameterCount();
							for (vint i = 0; i < count; i++)
							{
								const auto& name = ctor->GetParameter(i)->GetName();
								if (CTOR_PARAM_NAME(name) == propertyInfo.propertyName.ToString())
								{
									result->scope = GuiInstancePropertyInfo::ViewModel;
								}
							}
						}

						IPropertyInfo* prop = propertyInfo.typeInfo.typeDescriptor->GetPropertyByName(propertyInfo.propertyName.ToString(), true);
						if (IsItemProperty(prop))
						{
							result->acceptableTypes.Add(description::GetTypeDescriptor<WString>());
						}
						else if (!FillPropertyInfo(result, propType))
						{
							goto UNSUPPORTED;
						}

						PropertyType value(result, prop);
						propertyTypes.Add(key, value);
						return value;
					}
					
				UNSUPPORTED:
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
				return
					GetDefaultConstructor(typeInfo.typeDescriptor) != nullptr ||
					GetInstanceConstructor(typeInfo.typeDescriptor) != nullptr;
			}

			Ptr<workflow::WfStatement> CreateInstance(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
			{
				CTOR_PARAM_PREFIX
				auto defaultCtor = GetDefaultConstructor(typeInfo.typeDescriptor);
				auto instanceCtor = GetInstanceConstructor(typeInfo.typeDescriptor);

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

			Ptr<workflow::WfStatement> AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
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
								auto propertyValueExpression = propertyValue.expression;

								if (IsItemPropertyName(propertyType.f1))
								{
									bool isItemProperty = IsItemPropertyType(propertyType.f1->GetReturn());
									bool isWritableItemProperty = IsWritableItemPropertyType(propertyType.f1->GetReturn());

									if (isItemProperty || isWritableItemProperty)
									{
										auto propertyDescription = propertyValueExpression.Cast<WfStringExpression>()->value.value;
										Ptr<WfExpression> propertyExpression;
										{
											propertyExpression = Workflow_ParseExpression({ resolvingResult.resource }, propertyDescription, propertyValue.valuePosition, errors);
											if (!propertyExpression)
											{
												break;
											}
										};

										vint indexItemType = resolvingResult.envVars.Keys().IndexOf(GlobalStringKey::Get(L"ItemType"));
										if (indexItemType == -1)
										{
											auto error
												= L"Precompile: env.ItemType must be specified before assigning \""
												+ propertyDescription
												+ L"\" to property \""
												+ propertyType.f1->GetName()
												+ L"\" of type \""
												+ propertyType.f1->GetOwnerTypeDescriptor()->GetTypeName()
												+ L"\".";
											errors.Add(GuiResourceError({ resolvingResult.resource }, attPosition, error));
											break;
										}

										Ptr<WfType> itemType;
										{
											const auto& values = resolvingResult.envVars.GetByIndex(indexItemType);
											auto itemTypeValue = values[values.Count() - 1];

											itemType = Workflow_ParseType({ resolvingResult.resource }, itemTypeValue->value, itemTypeValue->valuePosition, errors);
											if (!itemType)
											{
												break;
											}
										};

										vint indexItemName = resolvingResult.envVars.Keys().IndexOf(GlobalStringKey::Get(L"ItemName"));
										WString itemName(L"item", false);
										if (indexItemName != -1)
										{
											const auto& values = resolvingResult.envVars.GetByIndex(indexItemName);
											itemName = values[values.Count() - 1]->value;
										}

										if (auto refExpr = propertyExpression.Cast<WfReferenceExpression>())
										{
											if (refExpr->name.value != itemName)
											{
												auto refItem = MakePtr<WfReferenceExpression>();
												refItem->name.value = itemName;

												auto member = MakePtr<WfMemberExpression>();
												member->parent = refItem;
												member->name.value = refExpr->name.value;

												propertyExpression = member;
											}
										}

										auto funcDecl = MakePtr<WfFunctionDeclaration>();
										funcDecl->anonymity = WfFunctionAnonymity::Anonymous;
										{
											auto genericType = propertyType.f1->GetReturn()->GetElementType();
											funcDecl->returnType = GetTypeFromTypeInfo(genericType->GetGenericArgument(0));
											{
												auto argument = MakePtr<WfFunctionArgument>();
												argument->name.value = L"<item>";
												argument->type = GetTypeFromTypeInfo(genericType->GetGenericArgument(1));
												funcDecl->arguments.Add(argument);
											}

											if (isWritableItemProperty)
											{
												{
													auto argument = MakePtr<WfFunctionArgument>();
													argument->name.value = L"<value>";
													argument->type = GetTypeFromTypeInfo(genericType->GetGenericArgument(2));
													funcDecl->arguments.Add(argument);
												}
												{
													auto argument = MakePtr<WfFunctionArgument>();
													argument->name.value = L"<update>";
													argument->type = GetTypeFromTypeInfo(genericType->GetGenericArgument(3));
													funcDecl->arguments.Add(argument);
												}
											}
										}

										auto funcBlock = MakePtr<WfBlockStatement>();
										funcDecl->statement = funcBlock;

										{
											auto refItem = MakePtr<WfReferenceExpression>();
											refItem->name.value = L"<item>";

											auto refCast = MakePtr<WfTypeCastingExpression>();
											refCast->strategy = WfTypeCastingStrategy::Strong;
											refCast->type = itemType;
											refCast->expression = refItem;

											auto varDecl = MakePtr<WfVariableDeclaration>();
											varDecl->name.value = itemName;
											varDecl->expression = refCast;

											auto varStat = MakePtr<WfVariableStatement>();
											varStat->variable = varDecl;
											funcBlock->statements.Add(varStat);
										}

										Ptr<WfReturnStatement> returnStat;
										{
											returnStat = MakePtr<WfReturnStatement>();
											returnStat->expression = propertyExpression;
										}

										if (isItemProperty)
										{
											funcBlock->statements.Add(returnStat);
										}
										else if (isWritableItemProperty)
										{
											auto ifStat = MakePtr<WfIfStatement>();
											funcBlock->statements.Add(ifStat);
											{
												auto refUpdate = MakePtr<WfReferenceExpression>();
												refUpdate->name.value = L"<update>";

												ifStat->expression = refUpdate;
											}
											{
												auto block = MakePtr<WfBlockStatement>();
												ifStat->trueBranch = block;

												{
													auto refValue = MakePtr<WfReferenceExpression>();
													refValue->name.value = L"<value>";

													auto assignExpr = MakePtr<WfBinaryExpression>();
													assignExpr->op = WfBinaryOperator::Assign;
													assignExpr->first = CopyExpression(propertyExpression);
													assignExpr->second = refValue;

													auto stat = MakePtr<WfExpressionStatement>();
													stat->expression = assignExpr;
													block->statements.Add(stat);
												}
												{
													auto returnStat = MakePtr<WfReturnStatement>();
													block->statements.Add(returnStat);

													auto returnType = propertyType.f1->GetReturn()->GetElementType()->GetGenericArgument(0);
													auto returnTd = returnType->GetTypeDescriptor();

													if ((returnTd->GetTypeDescriptorFlags() & TypeDescriptorFlags::ReferenceType) != TypeDescriptorFlags::Undefined)
													{
														auto expr = MakePtr<WfLiteralExpression>();
														expr->value = WfLiteralValue::Null;
														returnStat->expression = expr;
													}
													else if ((returnTd->GetTypeDescriptorFlags() & TypeDescriptorFlags::EnumType) != TypeDescriptorFlags::Undefined)
													{
														auto zero = MakePtr<WfIntegerExpression>();
														zero->value.value = L"0";

														auto expr = MakePtr<WfInferExpression>();
														expr->type = GetTypeFromTypeInfo(returnType);
														expr->expression = zero;
														returnStat->expression = expr;
													}
													else if (returnTd->GetTypeDescriptorFlags() == TypeDescriptorFlags::Struct)
													{
														auto expr = MakePtr<WfConstructorExpression>();
														returnStat->expression = expr;
													}
													else if (returnTd == description::GetTypeDescriptor<WString>())
													{
														returnStat->expression = MakePtr<WfStringExpression>();
													}
													else if (returnTd == description::GetTypeDescriptor<bool>())
													{
														auto expr = MakePtr<WfLiteralExpression>();
														expr->value = WfLiteralValue::False;
														returnStat->expression = expr;
													}
													else if (returnTd == description::GetTypeDescriptor<float>() || returnTd == description::GetTypeDescriptor<double>())
													{
														auto expr = MakePtr<WfFloatingExpression>();
														expr->value.value = L"0";
														returnStat->expression = expr;
													}
													else
													{
														auto expr = MakePtr<WfIntegerExpression>();
														expr->value.value = L"0";
														returnStat->expression = expr;
													}
												}
											}
											{
												auto block = MakePtr<WfBlockStatement>();
												ifStat->falseBranch = block;

												block->statements.Add(returnStat);
											}
										}

										auto funcExpr = MakePtr<WfFunctionExpression>();
										funcExpr->function = funcDecl;
										propertyValueExpression = funcExpr;
									}
								}

								auto refValue = MakePtr<WfReferenceExpression>();
								refValue->name.value = variableName.ToString();

								auto refProp = MakePtr<WfMemberExpression>();
								refProp->parent = refValue;
								refProp->name.value = prop.ToString();

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refProp;
								assign->second = propertyValueExpression;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;
								block->statements.Add(stat);
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

			Ptr<workflow::WfExpression> GetParameter(types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
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
	}
}
