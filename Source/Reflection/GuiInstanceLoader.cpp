#include "GuiInstanceLoader.h"
#include "TypeDescriptors/GuiReflectionEvents.h"
#include "../Resources/GuiParserManager.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSharedScript.h"
#include "GuiInstanceLoader_WorkflowCompiler.h"

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

/***********************************************************************
GuiInstancePropertyInfo
***********************************************************************/

		GuiInstancePropertyInfo::GuiInstancePropertyInfo()
			:support(NotSupport)
			, tryParent(false)
			, required(false)
			, scope(Property)
		{
		}

		GuiInstancePropertyInfo::~GuiInstancePropertyInfo()
		{
		}

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
GuiInstanceEventInfo
***********************************************************************/

		GuiInstanceEventInfo::GuiInstanceEventInfo()
			:support(NotSupport)
			, argumentType(0)
		{
		}

		GuiInstanceEventInfo::~GuiInstanceEventInfo()
		{
		}

		Ptr<GuiInstanceEventInfo> GuiInstanceEventInfo::Unsupported()
		{
			return new GuiInstanceEventInfo;
		}

		Ptr<GuiInstanceEventInfo> GuiInstanceEventInfo::Assign(description::ITypeDescriptor* typeDescriptor)
		{
			Ptr<GuiInstanceEventInfo> info = new GuiInstanceEventInfo;
			info->support = SupportAssign;
			info->argumentType = typeDescriptor;
			return info;
		}

/***********************************************************************
IGuiInstanceLoader
***********************************************************************/

		bool IGuiInstanceLoader::IsDeserializable(const TypeInfo& typeInfo)
		{
			return false;
		}

		description::Value IGuiInstanceLoader::Deserialize(const TypeInfo& typeInfo, const WString& text)
		{
			return Value();
		}

		bool IGuiInstanceLoader::IsCreatable(const TypeInfo& typeInfo)
		{
			return false;
		}

		description::Value IGuiInstanceLoader::CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)
		{
			return Value();
		}

		bool IGuiInstanceLoader::IsInitializable(const TypeInfo& typeInfo)
		{
			return false;
		}

		Ptr<GuiInstanceContextScope> IGuiInstanceLoader::InitializeInstance(const TypeInfo& typeInfo, description::Value instance)
		{
			return 0;
		}

		void IGuiInstanceLoader::GetPropertyNames(const TypeInfo& typeInfo, List<GlobalStringKey>& propertyNames)
		{
		}

		void IGuiInstanceLoader::GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)
		{
		}

		Ptr<GuiInstancePropertyInfo> IGuiInstanceLoader::GetPropertyType(const PropertyInfo& propertyInfo)
		{
			return 0;
		}

		bool IGuiInstanceLoader::GetPropertyValue(PropertyValue& propertyValue)
		{
			return false;
		}

		bool IGuiInstanceLoader::SetPropertyValue(PropertyValue& propertyValue)
		{
			return false;
		}

		void IGuiInstanceLoader::GetEventNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& eventNames)
		{
		}

		Ptr<GuiInstanceEventInfo> IGuiInstanceLoader::GetEventType(const PropertyInfo& eventInfo)
		{
			return 0;
		}

		bool IGuiInstanceLoader::SetEventValue(PropertyValue& propertyValue)
		{
			return false;
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
			typedef Tuple<Ptr<GuiInstanceEventInfo>, IEventInfo*>			EventType;

			Dictionary<FieldKey, PropertyType>								propertyTypes;
			Dictionary<FieldKey, EventType>									eventTypes;

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

			bool IsDeserializable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeDescriptor->GetValueSerializer() != 0;
			}

			description::Value Deserialize(const TypeInfo& typeInfo, const WString& text)override
			{
				if (IValueSerializer* serializer = typeInfo.typeDescriptor->GetValueSerializer())
				{
					Value loadedValue;
					if (serializer->Parse(text, loadedValue))
					{
						return loadedValue;
					}
				}
				return Value();
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return GetDefaultConstructor(typeInfo.typeDescriptor) != 0;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (IMethodInfo* method = GetDefaultConstructor(typeInfo.typeDescriptor))
				{
					return method->Invoke(Value(), (Value_xs()));
				}
				else
				{
					env->scope->errors.Add(L"Failed to create \"" + typeInfo.typeName.ToString() + L"\" because no there is no default constructor.");
					return Value();
				}
			}

			bool IsInitializable(const TypeInfo& typeInfo)override
			{
				return false;
			}

			Ptr<GuiInstanceContextScope> InitializeInstance(const TypeInfo& typeInfo, description::Value instance)override
			{
				return 0;
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

			bool GetPropertyValue(PropertyValue& propertyValue)override
			{
				if (IPropertyInfo* prop = GetPropertyTypeCached(propertyValue).f1)
				{
					if (prop->IsReadable())
					{
						propertyValue.propertyValue = prop->GetValue(propertyValue.instanceValue);
						return true;
					}
				}
				return false;
			}

			bool SetPropertyValue(PropertyValue& propertyValue)override
			{
				PropertyType propertyType = GetPropertyTypeCached(propertyValue);
				if (propertyType.f1)
				{
					switch (propertyType.f0->support)
					{
					case GuiInstancePropertyInfo::SupportCollection:
						{
							Value value = propertyType.f1->GetValue(propertyValue.instanceValue);
							if (auto list = dynamic_cast<IValueList*>(value.GetRawPtr()))
							{
								list->Add(propertyValue.propertyValue);
								return true;
							}
						}
						break;
					case GuiInstancePropertyInfo::SupportAssign:
					case GuiInstancePropertyInfo::SupportArray:
						propertyValue.instanceValue.SetProperty(propertyValue.propertyName.ToString(), propertyValue.propertyValue);
						propertyType.f1->SetValue(propertyValue.instanceValue, propertyValue.propertyValue);
						return true;
					default:;
					}
				}
				return false;
			}

			//***********************************************************************************

			void CollectEventNames(const TypeInfo& typeInfo, ITypeDescriptor* typeDescriptor, collections::List<GlobalStringKey>& eventNames)
			{
				vint eventCount = typeDescriptor->GetEventCount();
				for (vint i = 0; i < eventCount; i++)
				{
					GlobalStringKey eventName = GlobalStringKey::Get(typeDescriptor->GetEvent(i)->GetName());
					if (!eventNames.Contains(eventName))
					{
						auto info = GetEventType(PropertyInfo(typeInfo, eventName));
						if (info && info->support != GuiInstanceEventInfo::NotSupport)
						{
							eventNames.Add(eventName);
						}
					}
				}

				vint parentCount = typeDescriptor->GetBaseTypeDescriptorCount();
				for (vint i = 0; i < parentCount; i++)
				{
					CollectEventNames(typeInfo, typeDescriptor->GetBaseTypeDescriptor(i), eventNames);
				}
			}

			//***********************************************************************************

			void GetEventNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& eventNames)override
			{
				CollectEventNames(typeInfo, typeInfo.typeDescriptor, eventNames);
			}

			EventType GetEventTypeCached(const PropertyInfo& eventInfo)
			{
				FieldKey key(eventInfo.typeInfo.typeDescriptor, eventInfo.propertyName);
				vint index = eventTypes.Keys().IndexOf(key);
				if (index == -1)
				{
					if (IEventInfo* ev = eventInfo.typeInfo.typeDescriptor->GetEventByName(eventInfo.propertyName.ToString(), true))
					{
#ifndef VCZH_DEBUG_NO_REFLECTION
						ITypeInfo	*handlerType = 0,
									*genericType = 0,
									*functionType = 0,
									*returnType = 0,
									*senderType = 0,
									*argumentType = 0;

						handlerType = ev->GetHandlerType();
						if (handlerType->GetDecorator() != ITypeInfo::SharedPtr) goto UNSUPPORTED;

						genericType = handlerType->GetElementType();
						if (genericType->GetDecorator() != ITypeInfo::Generic) goto UNSUPPORTED;

						functionType = genericType->GetElementType();
						if (functionType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
						if (functionType->GetTypeDescriptor() != description::GetTypeDescriptor<IValueFunctionProxy>()) goto UNSUPPORTED;

						if (genericType->GetGenericArgumentCount() != 3) goto UNSUPPORTED;
						returnType = genericType->GetGenericArgument(0);
						senderType = genericType->GetGenericArgument(1);
						argumentType = genericType->GetGenericArgument(2);
					
						if (returnType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
						if (returnType->GetTypeDescriptor() != description::GetTypeDescriptor<VoidValue>()) goto UNSUPPORTED;
					
						if (senderType->GetDecorator() != ITypeInfo::RawPtr) goto UNSUPPORTED;
						senderType = senderType->GetElementType();
						if (senderType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
						if (senderType->GetTypeDescriptor() != description::GetTypeDescriptor<compositions::GuiGraphicsComposition>()) goto UNSUPPORTED;
					
						if (argumentType->GetDecorator() != ITypeInfo::RawPtr) goto UNSUPPORTED;
						argumentType = argumentType->GetElementType();
						if (argumentType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
						if (!argumentType->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<compositions::GuiEventArgs>())) goto UNSUPPORTED;

						{
							auto result = GuiInstanceEventInfo::Assign(argumentType->GetTypeDescriptor());
							EventType value(result, ev);
							eventTypes.Add(key, value);
							return value;
						}
					UNSUPPORTED:
						{
							auto result = GuiInstanceEventInfo::Unsupported();
							EventType value(result, ev);
							eventTypes.Add(key, value);
							return value;
						}
#endif
					}

					EventType value(0, 0);
					eventTypes.Add(key, value);
					return value;
				}
				else
				{
					return eventTypes.Values()[index];
				}
			}

			Ptr<GuiInstanceEventInfo> GetEventType(const PropertyInfo& eventInfo)override
			{
				return GetEventTypeCached(eventInfo).f0;
			}

			bool SetEventValue(PropertyValue& propertyValue)override
			{
				EventType eventType = GetEventTypeCached(propertyValue);
				if (eventType.f0 && eventType.f0->support == GuiInstanceEventInfo::SupportAssign)
				{
					Ptr<IValueFunctionProxy> proxy=UnboxValue<Ptr<IValueFunctionProxy>>(propertyValue.propertyValue, Description<IValueFunctionProxy>::GetAssociatedTypeDescriptor(), L"function");
					eventType.f1->Attach(propertyValue.instanceValue, proxy);
					return true;
				}
				return false;
			}
		};

/***********************************************************************
GuiResourceInstanceLoader
***********************************************************************/

		class GuiResourceInstanceLoader : public Object, public IGuiInstanceLoader
		{
		protected:
			Ptr<GuiResource>						resource;
			Ptr<GuiInstanceContext>					context;
			GlobalStringKey							contextClassName;

			void InitializeContext(Ptr<GuiResourcePathResolver> resolver, List<WString>& errors)
			{
				context->ApplyStyles(resolver, errors);
			}
		public:
			GuiResourceInstanceLoader(Ptr<GuiResource> _resource, Ptr<GuiInstanceContext> _context)
				:resource(_resource)
				, context(_context)
			{
				if (context->className)
				{
					contextClassName = GlobalStringKey::Get(context->className.Value());
				}
			}

			GlobalStringKey GetTypeName()override
			{
				return contextClassName;
			}

			bool IsCreatable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == contextClassName;
			}

			description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
			{
				if (typeInfo.typeName == contextClassName)
				{
					if (auto typeDescriptor = GetGlobalTypeManager()->GetTypeDescriptor(typeInfo.typeName.ToString()))
					{
						InitializeContext(env->resolver, env->scope->errors);
						SortedList<GlobalStringKey> argumentNames;
						{
							List<GlobalStringKey> names;
							GetConstructorParameters(typeInfo, names);
							CopyFrom(argumentNames, names);
						}
						auto group = typeDescriptor->GetConstructorGroup();
						for (vint i = 0; i < group->GetMethodCount(); i++)
						{
							auto method = group->GetMethod(i);
							List<GlobalStringKey> parameterNames;
							for (vint j = 0; j < method->GetParameterCount(); j++)
							{
								parameterNames.Add(GlobalStringKey::Get(method->GetParameter(j)->GetName()));
							}

							auto f = [](GlobalStringKey a, GlobalStringKey b){return GlobalStringKey::Compare(a, b); };
							if (CompareEnumerable(argumentNames, From(parameterNames).OrderBy(f)) == 0)
							{
								Array<Value> arguments(constructorArguments.Count());
								for (vint j = 0; j < arguments.Count(); j++)
								{
									arguments[j] = constructorArguments[parameterNames[j]][0];
								}
								Value result = method->Invoke(Value(), arguments);

								if (auto partialClass = dynamic_cast<IGuiInstancePartialClass*>(result.GetRawPtr()))
								{
									if (auto partialScope = partialClass->GetScope())
									{
										CopyFrom(env->scope->errors, partialScope->errors, true);
									}
								}
								return result;
							}
						}
					}

					Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
					auto scope = LoadInstanceFromContext(context, resolver);

					if (scope)
					{
						CopyFrom(env->scope->errors, scope->errors, true);
						return scope->rootInstance;
					}
				}
				return Value();
			}

			bool IsInitializable(const TypeInfo& typeInfo)override
			{
				return typeInfo.typeName == contextClassName;
			}

			Ptr<GuiInstanceContextScope> InitializeInstance(const TypeInfo& typeInfo, description::Value instance)override
			{
				if (typeInfo.typeName == contextClassName)
				{
					Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
					List<WString> errors;
					InitializeContext(resolver, errors);

					auto scope = InitializeInstanceFromContext(context, resolver, instance);
					if (scope)
					{
						for (vint i = 0; i < errors.Count(); i++)
						{
							scope->errors.Insert(i, errors[i]);
						}
					}
					return scope;
				}
				return 0;
			}

			void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)
			{
				if (typeInfo.typeName == contextClassName)
				{
					FOREACH(Ptr<GuiInstanceParameter>, parameter, context->parameters)
					{
						if (description::GetTypeDescriptor(parameter->className.ToString()))
						{
							propertyNames.Add(parameter->name);
						}
					}
				}
			}

			Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)
			{
				if (propertyInfo.typeInfo.typeName == contextClassName)
				{
					FOREACH(Ptr<GuiInstanceParameter>, parameter, context->parameters)
					{
						if (parameter->name == propertyInfo.propertyName)
						{
							if (auto td = description::GetTypeDescriptor(parameter->className.ToString()))
							{
								auto info = GuiInstancePropertyInfo::Assign(td);
								info->required = true;
								info->scope = GuiInstancePropertyInfo::ViewModel;
								return info;
							}
						}
					}
				}
				return IGuiInstanceLoader::GetPropertyType(propertyInfo);
			}
		};

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
				ITypeDescriptor*					typeDescriptor;
				GlobalStringKey						parentTypeName;				// for virtual type only
				Ptr<IGuiInstanceLoader>				loader;

				List<ITypeDescriptor*>				parentTypes;				// all direct or indirect base types that does not has a type info
				List<VirtualTypeInfo*>				parentTypeInfos;			// type infos for all registered direct or indirect base types

				VirtualTypeInfo()
					:typeDescriptor(0)
				{
				}
			};
			typedef Dictionary<GlobalStringKey, Ptr<VirtualTypeInfo>>					VirtualTypeInfoMap;
			typedef Dictionary<WString, Ptr<GuiResource>>								ResourceMap;

			Ptr<IGuiInstanceLoader>					rootLoader;
			BinderMap								binders;
			EventBinderMap							eventBinders;
			VirtualTypeInfoMap						typeInfos;
			ResourceMap								resources;

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

			void GetClassesInResource(Ptr<GuiResourceFolder> folder, Dictionary<GlobalStringKey, Ptr<GuiInstanceContext>>& classes)
			{
				FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
				{
					if (auto context = item->GetContent().Cast<GuiInstanceContext>())
					{
						if (context->className)
						{
							auto contextClassName = GlobalStringKey::Get(context->className.Value());
							if (!classes.Keys().Contains(contextClassName))
							{
								classes.Add(contextClassName, context);
							}
						}
					}
				}
				FOREACH(Ptr<GuiResourceFolder>, subFolder, folder->GetFolders())
				{
					GetClassesInResource(subFolder, classes);
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
				instanceLoaderManager = 0;
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

			bool SetResource(const WString& name, Ptr<GuiResource> resource)override
			{
				vint index = resources.Keys().IndexOf(name);
				if (index != -1) return false;

				Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
				Dictionary<GlobalStringKey, Ptr<GuiInstanceContext>> classes;
				Dictionary<GlobalStringKey, GlobalStringKey> parentTypes;
				GetClassesInResource(resource, classes);

				FOREACH(Ptr<GuiInstanceContext>, context, classes.Values())
				{
					auto contextClassName = GlobalStringKey::Get(context->className.Value());
					if (typeInfos.Keys().Contains(contextClassName))
					{
						return false;
					}

					Ptr<GuiInstanceEnvironment> env = new GuiInstanceEnvironment(context, resolver);
					auto loadingSource = FindInstanceLoadingSource(env->context, context->instance.Obj());
					if (loadingSource.loader)
					{
						parentTypes.Add(contextClassName, loadingSource.typeName);
					}
				}
				
				FOREACH(GlobalStringKey, className, classes.Keys())
				{
					auto context = classes[className];
					vint index = parentTypes.Keys().IndexOf(className);
					if (index == -1) continue;
					auto parentType = parentTypes.Values()[index];

					Ptr<IGuiInstanceLoader> loader = new GuiResourceInstanceLoader(resource, context);
					if (GetGlobalTypeManager()->GetTypeDescriptor(context->className.Value()))
					{
						SetLoader(loader);
					}
					else
					{
						CreateVirtualType(parentType, loader);
					}
				}

				resources.Add(name, resource);
				return true;
			}

			Ptr<GuiResource> GetResource(const WString& name)override
			{
				vint index = resources.Keys().IndexOf(name);
				return index == -1 ? nullptr : resources.Values()[index];
			}
		};
		GUI_REGISTER_PLUGIN(GuiInstanceLoaderManager)
	}
}