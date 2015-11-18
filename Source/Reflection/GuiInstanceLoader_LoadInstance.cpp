#include "GuiInstanceLoader.h"
#include "TypeDescriptors/GuiReflectionEvents.h"
#include "../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace reflection::description;

/***********************************************************************
Helper Functions Declarations
***********************************************************************/

		struct FillInstanceBindingSetter
		{
			IGuiInstanceBinder*					binder;
			IGuiInstanceLoader*					loader;
			GuiAttSetterRepr*					bindingTarget;
			IGuiInstanceLoader::PropertyValue	propertyValue;

			FillInstanceBindingSetter()
				:binder(0)
				, loader(0)
				, bindingTarget(0)
			{
			}
		};

		struct FillInstanceEventSetter
		{
			IGuiInstanceEventBinder*			binder;
			IGuiInstanceLoader*					loader;
			GuiAttSetterRepr*					bindingTarget;
			Ptr<GuiInstanceEventInfo>			eventInfo;
			IGuiInstanceLoader::PropertyValue	propertyValue;
			WString								handlerName;

			FillInstanceEventSetter()
				:binder(0)
				, loader(0)
				, bindingTarget(0)
			{
			}
		};

		bool LoadInstancePropertyValue(
			Ptr<GuiInstanceEnvironment> env,
			GuiAttSetterRepr* attSetter,
			GlobalStringKey binding,
			IGuiInstanceLoader::PropertyValue propertyValue,
			List<Ptr<GuiValueRepr>>& input,
			IGuiInstanceLoader* propertyLoader,
			bool constructorArgument,
			List<Pair<Value, IGuiInstanceLoader*>>& output,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters
			);

		void FillInstance(
			description::Value createdInstance,
			Ptr<GuiInstanceEnvironment> env,
			GuiAttSetterRepr* attSetter,
			IGuiInstanceLoader* loader,
			bool skipDefaultProperty,
			GlobalStringKey typeName,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters
			);

		description::Value CreateInstance(
			Ptr<GuiInstanceEnvironment> env,
			GuiConstructorRepr* ctor,
			description::ITypeDescriptor* expectedType,
			GlobalStringKey& typeName,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters,
			bool isRootInstance
			);

		void ExecuteParameters(
			Ptr<GuiInstanceEnvironment> env
			);

		bool PrepareBindingContext(
			Ptr<GuiInstanceEnvironment> env,
			collections::List<GlobalStringKey>& contextNames,
			const WString& dependerType,
			const GlobalStringKey& dependerName
			);

		void ExecuteBindingSetters(
			Ptr<GuiInstanceEnvironment> env,
			List<FillInstanceBindingSetter>& bindingSetters
			);

		void ExecuteEventSetters(
			description::Value createdInstance,
			Ptr<GuiInstanceEnvironment> env,
			List<FillInstanceEventSetter>& eventSetters
			);

		void InitializeInstanceFromConstructor(
			Ptr<GuiInstanceEnvironment> env,
			GuiConstructorRepr* ctor,
			IGuiInstanceLoader* instanceLoader,
			GlobalStringKey typeName,
			description::Value instance,
			bool deserialized,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters
			);

		namespace visitors
		{

/***********************************************************************
LoadValueVisitor
***********************************************************************/

			class LoadValueVisitor : public Object, public GuiValueRepr::IVisitor
			{
			public:
				Ptr<GuiInstanceEnvironment>				env;
				List<ITypeDescriptor*>&					acceptableTypes;
				List<FillInstanceBindingSetter>&		bindingSetters;
				List<FillInstanceEventSetter>&			eventSetters;
				bool									result;
				Value									loadedValue;

				LoadValueVisitor(Ptr<GuiInstanceEnvironment> _env, List<ITypeDescriptor*>& _acceptableTypes, List<FillInstanceBindingSetter>& _bindingSetters, List<FillInstanceEventSetter>& _eventSetters)
					:env(_env)
					, acceptableTypes(_acceptableTypes)
					, bindingSetters(_bindingSetters)
					, eventSetters(_eventSetters)
					, result(false)
				{
				}

				void Visit(GuiTextRepr* repr)override
				{
					FOREACH(ITypeDescriptor*, typeDescriptor, acceptableTypes)
					{
						if (IValueSerializer* serializer = typeDescriptor->GetValueSerializer())
						{
							if (serializer->Parse(repr->text, loadedValue))
							{
								result = true;
								return;
							}
						}
					}
					
					FOREACH(ITypeDescriptor*, typeDescriptor, acceptableTypes)
					{
						env->scope->errors.Add(
							L"Failed to deserialize object of type \"" +
							typeDescriptor->GetTypeName() +
							L"\" from string \"" +
							repr->text +
							L"\".");
					}
				}

				void Visit(GuiAttSetterRepr* repr)override
				{
				}

				void Visit(GuiConstructorRepr* repr)override
				{
					vint errorCount = env->scope->errors.Count();
					FOREACH(ITypeDescriptor*, typeDescriptor, acceptableTypes)
					{
						GlobalStringKey _typeName;
						loadedValue = CreateInstance(env, repr, typeDescriptor, _typeName, bindingSetters, eventSetters, false);
						if (!loadedValue.IsNull())
						{
							for (vint i = env->scope->errors.Count() - 1; i >= errorCount; i--)
							{
								if (wcsstr(env->scope->errors[i].Buffer(), L"because the expected type is"))
								{
									env->scope->errors.RemoveAt(i);
								}
							}
							result = true;
							return;
						}
					}
				}

				static bool LoadValue(Ptr<GuiValueRepr> valueRepr, Ptr<GuiInstanceEnvironment> env, List<ITypeDescriptor*>& acceptableTypes, List<FillInstanceBindingSetter>& bindingSetters, List<FillInstanceEventSetter>& eventSetters, Value& loadedValue)
				{
					LoadValueVisitor visitor(env, acceptableTypes, bindingSetters, eventSetters);
					valueRepr->Accept(&visitor);
					if (visitor.result)
					{
						loadedValue = visitor.loadedValue;
					}
					return visitor.result;
				}
			};

		}
		using namespace visitors;

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
LoadInstancePropertyValue
***********************************************************************/

		bool LoadInstancePropertyValue(
			Ptr<GuiInstanceEnvironment> env,
			GuiAttSetterRepr* attSetter,
			GlobalStringKey binding,
			IGuiInstanceLoader::PropertyValue propertyValue,
			List<Ptr<GuiValueRepr>>& input,
			IGuiInstanceLoader* propertyLoader,
			bool constructorArgument,
			List<Pair<Value, IGuiInstanceLoader*>>& output,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters
			)
		{
			GlobalStringKey instanceType;
			if (propertyValue.instanceValue.IsNull())
			{
				instanceType = propertyLoader->GetTypeName();
			}
			else
			{
				instanceType = GlobalStringKey::Get(propertyValue.instanceValue.GetTypeDescriptor()->GetTypeName());
			}

			vint loadedValueCount = 0;
			// try to look for a loader to handle this property
			while (propertyLoader && loadedValueCount < input.Count())
			{
				if (auto propertyInfo = propertyLoader->GetPropertyType(propertyValue))
				{
					if (constructorArgument)
					{
						if (propertyInfo->scope == GuiInstancePropertyInfo::Property)
						{
							return false;
						}
					}
					else
					{
						if (propertyInfo->scope == GuiInstancePropertyInfo::Constructor)
						{
							return false;
						}
					}
					if (propertyInfo->support == GuiInstancePropertyInfo::NotSupport)
					{
						env->scope->errors.Add(
							L"Property \"" +
							propertyValue.propertyName.ToString() +
							L"\" of type \"" +
							propertyValue.instanceValue.GetTypeDescriptor()->GetTypeName() +
							L"\" is not supported.");
						return false;
					}

					switch (propertyInfo->support)
					{
					case GuiInstancePropertyInfo::SupportSet:
						if (input.Count() != 1)
						{
							env->scope->errors.Add(
								L"Collection property \"" +
								propertyValue.propertyName.ToString() +
								L"\" of type \"" +
								instanceType.ToString() +
								L"\" can only be assigned with a single value.");
							return false;
						}
						if (constructorArgument) return false;
						if (binding != GlobalStringKey::_Set)
						{
							env->scope->errors.Add(
								L"Collection property \"" +
								propertyValue.propertyName.ToString() +
								L"\" of type \"" +
								instanceType.ToString() +
								L"\" can only be retrived using binding \"set\".");
							return false;
						}
						{
							// set binding: get the property value and apply another property list on it
							if(Ptr<GuiAttSetterRepr> propertyAttSetter=input[0].Cast<GuiAttSetterRepr>())
							{
								if(propertyLoader->GetPropertyValue(propertyValue) && propertyValue.propertyValue.GetRawPtr())
								{
									input[0] = 0;
									loadedValueCount++;

									ITypeDescriptor* propertyTypeDescriptor=propertyValue.propertyValue.GetRawPtr()->GetTypeDescriptor();
									auto propertyTypeKey = GlobalStringKey::Get(propertyTypeDescriptor->GetTypeName());
									IGuiInstanceLoader* propertyInstanceLoader=GetInstanceLoaderManager()->GetLoader(propertyTypeKey);
									if(propertyInstanceLoader)
									{
										FillInstance(propertyValue.propertyValue, env, propertyAttSetter.Obj(), propertyInstanceLoader, false, propertyTypeKey, bindingSetters, eventSetters);
									}
								}
							}
						}
						break;
					case GuiInstancePropertyInfo::SupportCollection:
						if (binding != GlobalStringKey::Empty)
						{
							env->scope->errors.Add(
								L"Collection property \"" +
								propertyValue.propertyName.ToString() +
								L"\" of type \"" +
								instanceType.ToString() +
								L"\" cannot be assigned using binding.");
							return false;
						}
						{
							FOREACH_INDEXER(Ptr<GuiValueRepr>, valueRepr, index, input)
							{
								if (valueRepr)
								{
									// default binding: set the value directly
									vint errorCount = env->scope->errors.Count();
									if (LoadValueVisitor::LoadValue(valueRepr, env, propertyInfo->acceptableTypes, bindingSetters, eventSetters, propertyValue.propertyValue))
									{
										input[index] = 0;
										loadedValueCount++;
										output.Add(Pair<Value, IGuiInstanceLoader*>(propertyValue.propertyValue, propertyLoader));
									}
									else if (propertyInfo->tryParent)
									{
										env->scope->errors.RemoveRange(errorCount, env->scope->errors.Count() - errorCount);
									}
								}
							}
						}
						break;
					case GuiInstancePropertyInfo::SupportAssign:
						if (input.Count() != 1)
						{
							env->scope->errors.Add(
								L"Assignable property \"" +
								propertyValue.propertyName.ToString() +
								L"\" of type \"" +
								instanceType.ToString() +
								L"\" cannot be assigned using multiple values.");
							return false;
						}
						if (binding == GlobalStringKey::_Set)
						{
							env->scope->errors.Add(
								L"Assignable property \"" +
								propertyValue.propertyName.ToString() +
								L"\" of type \"" +
								instanceType.ToString() +
								L"\" cannot be retrived using binding \"set\".");
							return false;
						}
						{
							FOREACH_INDEXER(Ptr<GuiValueRepr>, valueRepr, index, input)
							{
								if (valueRepr)
								{
									bool canRemoveLoadedValue = false;
									if (binding == GlobalStringKey::Empty)
									{
										// default binding: set the value directly
										if (LoadValueVisitor::LoadValue(valueRepr, env, propertyInfo->acceptableTypes, bindingSetters, eventSetters, propertyValue.propertyValue))
										{
											canRemoveLoadedValue = true;
											output.Add(Pair<Value, IGuiInstanceLoader*>(propertyValue.propertyValue, propertyLoader));
										}
									}
									else if (IGuiInstanceBinder* binder=GetInstanceLoaderManager()->GetInstanceBinder(binding))
									{
										List<GlobalStringKey> contextNames;
										binder->GetRequiredContexts(contextNames);
										bool success = PrepareBindingContext(env, contextNames, L"property binding", binder->GetBindingName());

										if (success)
										{
											// other binding: provide the property value to the specified binder
											List<ITypeDescriptor*> binderExpectedTypes;
											binder->GetExpectedValueTypes(binderExpectedTypes);
											if (LoadValueVisitor::LoadValue(valueRepr, env, binderExpectedTypes, bindingSetters, eventSetters, propertyValue.propertyValue))
											{
												canRemoveLoadedValue = true;
												if (constructorArgument)
												{
													auto translatedValue = binder->GetValue(env, propertyValue.propertyValue);
													if (translatedValue.IsNull())
													{
														env->scope->errors.Add(
															L"Assignable property \"" +
															propertyValue.propertyName.ToString() +
															L"\" of type \"" +
															instanceType.ToString() +
															L"\" cannot be assigned using binding \"" +
															binding.ToString() +
															L"\" because the value translation failed.");
													}
													else
													{
														output.Add(Pair<Value, IGuiInstanceLoader*>(translatedValue, propertyLoader));
													}
												}
												else if (propertyInfo->scope == GuiInstancePropertyInfo::Property)
												{
													FillInstanceBindingSetter bindingSetter;
													bindingSetter.binder = binder;
													bindingSetter.loader = propertyLoader;
													bindingSetter.bindingTarget = attSetter;
													bindingSetter.propertyValue = propertyValue;
													bindingSetters.Add(bindingSetter);
												}
											}
										}
									}
									else
									{
										env->scope->errors.Add(
											L"Assignable property \"" +
											propertyValue.propertyName.ToString() +
											L"\" of type \"" +
											instanceType.ToString() +
											L"\" cannot be assigned using binding \"" +
											binding.ToString() +
											L"\" because the appropriate IGuiInstanceBinder for this binding cannot be found.");
									}

									if (canRemoveLoadedValue)
									{
										input[index] = 0;
										loadedValueCount++;
									}
								}
							}
						}
						break;
					case GuiInstancePropertyInfo::SupportArray:
						if (binding != GlobalStringKey::Empty)
						{
							env->scope->errors.Add(
								L"Array property \"" +
								propertyValue.propertyName.ToString() +
								L"\" of type \"" +
								instanceType.ToString() +
								L"\" cannot be assigned using binding.");
							return false;
						}
						{
							auto list = IValueList::Create();
							FOREACH_INDEXER(Ptr<GuiValueRepr>, valueRepr, index, input)
							{
								// default binding: add the value to the list
								if (LoadValueVisitor::LoadValue(valueRepr, env, propertyInfo->acceptableTypes, bindingSetters, eventSetters, propertyValue.propertyValue))
								{
									input[index] = 0;
									loadedValueCount++;
									list->Add(propertyValue.propertyValue);
								}
							}

							// set the whole list to the property
							output.Add(Pair<Value, IGuiInstanceLoader*>(Value::From(list), propertyLoader));
						}
						break;
					default:;
					}

					if (!propertyInfo->tryParent)
					{
						break;
					}
				}
				if (constructorArgument)
				{
					break;
				}
				else
				{
					propertyLoader = GetInstanceLoaderManager()->GetParentLoader(propertyLoader);
				}
			}

			return true;
		}

/***********************************************************************
FillInstance
***********************************************************************/

		void FillInstance(
			description::Value createdInstance,
			Ptr<GuiInstanceEnvironment> env,
			GuiAttSetterRepr* attSetter,
			IGuiInstanceLoader* loader,
			bool skipDefaultProperty,
			GlobalStringKey typeName,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters
			)
		{
			IGuiInstanceLoader::TypeInfo typeInfo(typeName, createdInstance.GetTypeDescriptor());
			// reverse loop to set the default property (name == L"") after all other properties
			for (vint i = attSetter->setters.Count() - 1; i >= 0; i--)
			{
				GlobalStringKey propertyName = attSetter->setters.Keys()[i];
				if (propertyName == GlobalStringKey::Empty && skipDefaultProperty)
				{
					continue;
				}

				auto propertyValue=attSetter->setters.Values()[i];
				IGuiInstanceLoader* propertyLoader=loader;
				IGuiInstanceLoader::PropertyValue cachedPropertyValue(
					typeInfo,
					propertyName,
					createdInstance
					);
				List<Ptr<GuiValueRepr>> input;
				List<Pair<Value, IGuiInstanceLoader*>> output;

				// extract all loaded property values
				CopyFrom(input, propertyValue->values);
				LoadInstancePropertyValue(env, attSetter, propertyValue->binding, cachedPropertyValue, input, propertyLoader, false, output, bindingSetters, eventSetters);

				// if there is no binding, set all values into the specified property
				if (propertyValue->binding == GlobalStringKey::Empty)
				{
					for (vint i = 0; i < output.Count(); i++)
					{
						auto value = output[i].key;
						auto valueLoader = output[i].value;
						cachedPropertyValue.propertyValue = value;
						if (!valueLoader->SetPropertyValue(cachedPropertyValue))
						{
							value.DeleteRawPtr();
						}
					}
				}
			}

			// attach events
			FOREACH_INDEXER(GlobalStringKey, eventName, index, attSetter->eventHandlers.Keys())
			{
				auto handler = attSetter->eventHandlers.Values()[index];

				IGuiInstanceLoader::PropertyInfo propertyInfo(
					typeInfo,
					eventName
					);

				// get the loader to attach the event
				Ptr<GuiInstanceEventInfo> eventInfo;
				IGuiInstanceLoader* eventLoader = loader;
				{
					while (eventLoader)
					{
						if ((eventInfo = eventLoader->GetEventType(propertyInfo)))
						{
							if (eventInfo->support == GuiInstanceEventInfo::NotSupport)
							{
								eventInfo = 0;
							}
							break;
						}
						eventLoader = GetInstanceLoaderManager()->GetParentLoader(eventLoader);
					}
				}

				IGuiInstanceEventBinder* binder = 0;
				if (handler->binding != GlobalStringKey::Empty)
				{
					binder = GetInstanceLoaderManager()->GetInstanceEventBinder(handler->binding);
					if (!binder)
					{
						env->scope->errors.Add(
							L"Failed to attach event \"" +
							eventName.ToString() +
							L"\" of type \"" +
							typeName.ToString() +
							L"\" with the handler \"" +
							handler->value +
							L"\" using event binding \"" +
							handler->binding.ToString() +
							L"\" because the appropriate IGuiInstanceEventBinder for this binding cannot be found.");
						continue;
					}
				}

				if (eventInfo)
				{
					FillInstanceEventSetter eventSetter;
					eventSetter.binder = binder;
					eventSetter.loader = eventLoader;
					eventSetter.bindingTarget = attSetter;
					eventSetter.eventInfo = eventInfo;
					eventSetter.propertyValue.typeInfo = propertyInfo.typeInfo;
					eventSetter.propertyValue.propertyName = propertyInfo.propertyName;
					eventSetter.propertyValue.instanceValue = createdInstance;
					eventSetter.handlerName = handler->value;
					eventSetters.Add(eventSetter);
				}
				else
				{
					env->scope->errors.Add(
						L"Failed to attach event \"" +
						eventName.ToString() +
						L"\" of type \"" +
						typeName.ToString() +
						L"\" with the handler \"" +
						handler->value +
						L"\" using event binding \"" +
						handler->binding.ToString() +
						L"\" because no IGuiInstanceLoader supports this event.");
				}
			}
		}

/***********************************************************************
CreateInstance
***********************************************************************/

		description::Value CreateInstance(
			Ptr<GuiInstanceEnvironment> env,
			GuiConstructorRepr* ctor,
			description::ITypeDescriptor* expectedType,
			GlobalStringKey& typeName,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters,
			bool isRootInstance
			)
		{
			// search for a correct loader
			InstanceLoadingSource source=FindInstanceLoadingSource(env->context, ctor);
			Value instance;
			IGuiInstanceLoader* instanceLoader = 0;
			bool deserialized = false;

			if(source.loader)
			{
				// found the correct loader, prepare a TypeInfo
				IGuiInstanceLoader* loader=source.loader;
				instanceLoader = source.loader;
				typeName = source.typeName;
				ITypeDescriptor* typeDescriptor = GetInstanceLoaderManager()->GetTypeDescriptorForType(source.typeName);
				
				// see if the constructor contains only a single text value
				Ptr<GuiTextRepr> singleTextValue;
				{
					vint index = ctor->setters.Keys().IndexOf(GlobalStringKey::Empty);
					if (index != -1)
					{
						auto setterValue = ctor->setters.Values()[index];
						if (setterValue->values.Count() == 1)
						{
							singleTextValue = setterValue->values[0].Cast<GuiTextRepr>();
						}
					}
					else
					{
						singleTextValue = new GuiTextRepr;
						singleTextValue->text = L"";
					}
				}

				// if the target type is not the expected type, fail
				if (!expectedType || expectedType==GetTypeDescriptor<Value>() || typeDescriptor->CanConvertTo(expectedType))
				{
					// traverse the loader and all ancestors to load the type
					IGuiInstanceLoader::TypeInfo typeInfo(typeName, typeDescriptor);
					bool foundLoader = false;
					while(!foundLoader && loader && instance.IsNull())
					{
						if (singleTextValue && loader->IsDeserializable(typeInfo))
						{
							foundLoader = true;
							// if the loader support deserialization and this is a single text value constructor
							// then choose deserialization
							instance = loader->Deserialize(typeInfo, singleTextValue->text);
							if (!instance.IsNull())
							{
								deserialized = true;
							}
							else
							{
								env->scope->errors.Add(
									L"Failed to deserialize object of type \"" +
									source.typeName.ToString() +
									L"\" from string \"" +
									singleTextValue->text +
									L"\".");
							}
						}
						else if (loader->IsCreatable(typeInfo))
						{
							foundLoader = true;
							// find all constructor parameters
							List<GlobalStringKey> constructorParameters;
							List<GlobalStringKey> requiredParameters;
							loader->GetConstructorParameters(typeInfo, constructorParameters);
							
							// see if all parameters exists
							Group<GlobalStringKey, Value> constructorArguments;
							FOREACH(GlobalStringKey, propertyName, constructorParameters)
							{
								IGuiInstanceLoader::PropertyInfo propertyInfo(typeInfo, propertyName);
								auto info = loader->GetPropertyType(propertyInfo);
								vint index = ctor->setters.Keys().IndexOf(propertyName);

								if (info->scope != GuiInstancePropertyInfo::Property)
								{
									if (info->required)
									{
										if (index == -1)
										{
											// if a required parameter doesn't exist, fail
											env->scope->errors.Add(
												L"Failed to create object of type \"" +
												source.typeName.ToString() +
												L"\" because the required constructor parameter \"" +
												propertyName.ToString() +
												L"\" is missing.");
											goto SKIP_CREATE_INSTANCE;
										}
										requiredParameters.Add(propertyName);
									}

									if (index != -1)
									{
										auto setterValue = ctor->setters.Values()[index];
										if (setterValue->binding != GlobalStringKey::Empty)
										{
											if (IGuiInstanceBinder* binder = GetInstanceLoaderManager()->GetInstanceBinder(setterValue->binding))
											{
												if (!binder->ApplicableToConstructorArgument())
												{
													// if the constructor argument uses binding, fail
													env->scope->errors.Add(
														L"Failed to create object of type \"" +
														source.typeName.ToString() +
														L"\" because the required constructor parameter \"" +
														propertyName.ToString() +
														L"\" is not allowed to use binding \"" +
														setterValue->binding.ToString() +
														L"\" which does not applicable to constructor parameters.");
													goto SKIP_CREATE_INSTANCE;
												}
											}
											else
											{
												env->scope->errors.Add(
													L"Failed to create object of type \"" +
													source.typeName.ToString() +
													L"\" because the required constructor parameter \"" +
													propertyName.ToString() +
													L"\" is not allowed to use binding \"" +
													setterValue->binding.ToString() +
													L"\" because the appropriate IGuiInstanceBinder for this binding cannot be found.");
												goto SKIP_CREATE_INSTANCE;
											}
										}

										// load the parameter
										List<Ptr<GuiValueRepr>> input;
										List<Pair<Value, IGuiInstanceLoader*>> output;
										IGuiInstanceLoader::PropertyValue propertyValue(typeInfo, propertyName, Value());

										CopyFrom(input, setterValue->values);
										LoadInstancePropertyValue(env, ctor, setterValue->binding, propertyValue, input, loader, true, output, bindingSetters, eventSetters);

										for (vint i = 0; i < output.Count(); i++)
										{
											constructorArguments.Add(propertyName, output[i].key);
										}
									}
								}
							}
							
							// check if all required parameters exist
							FOREACH(GlobalStringKey, propertyName, requiredParameters)
							{
								if (!constructorArguments.Contains(propertyName))
								{
									env->scope->errors.Add(
										L"Failed to create object of type \"" +
										source.typeName.ToString() +
										L"\" because the required constructor parameter \"" +
										propertyName.ToString() +
										L"\" is missing.");
									goto SKIP_CREATE_INSTANCE;
								}
							}

							// create the instance
							instance = loader->CreateInstance(env, typeInfo, constructorArguments);
						SKIP_CREATE_INSTANCE:
							// delete all arguments if the constructing fails
							if (instance.IsNull())
							{
								for (vint i = 0; i < constructorArguments.Count(); i++)
								{
									FOREACH(Value, value, constructorArguments.GetByIndex(i))
									{
										value.DeleteRawPtr();
									}
								}
							}
						}
						loader = GetInstanceLoaderManager()->GetParentLoader(loader);
					}

					if (instance.IsNull())
					{
						env->scope->errors.Add(
							L"Failed to create object of type \"" +
							source.typeName.ToString() +
							L"\".");
					}
				}
				else
				{
					env->scope->errors.Add(
						L"Failed to create object of type \"" +
						source.typeName.ToString() +
						L"\" because the expected type is \"" +
						expectedType->GetTypeName() +
						L"\".");
				}
			}
			else if(source.context)
			{
				// found another instance in the resource
				if (Ptr<GuiInstanceContextScope> scope = LoadInstanceFromContext(source.context, env->resolver, expectedType))
				{
					typeName = scope->typeName;
					instance = scope->rootInstance;
					instanceLoader = GetInstanceLoaderManager()->GetLoader(typeName);
				}
				else
				{
					auto contextCtor = source.context->instance;
					env->scope->errors.Add(
						L"Failed to find type \"" +
						(contextCtor->typeNamespace == GlobalStringKey::Empty
							? contextCtor->typeName.ToString()
							: contextCtor->typeNamespace.ToString() + L":" + contextCtor->typeName.ToString()
							) +
						L"\".");
				}
			}
			else
			{
				env->scope->errors.Add(
					L"Failed to find type \"" +
					(ctor->typeNamespace == GlobalStringKey::Empty
						? ctor->typeName.ToString()
						: ctor->typeNamespace.ToString() + L":" + ctor->typeName.ToString()
						) +
					L"\".");
			}

			if(instance.GetRawPtr() && instanceLoader)
			{
				if (isRootInstance)
				{
					env->scope->rootInstance = instance;
					ExecuteParameters(env);
				}
				InitializeInstanceFromConstructor(env, ctor, instanceLoader, typeName, instance, deserialized, bindingSetters, eventSetters);
			}
			return instance;
		}

/***********************************************************************
ExecuteBindingSetters
***********************************************************************/

		void ExecuteParameters(Ptr<GuiInstanceEnvironment> env)
		{
			auto td = env->scope->rootInstance.GetTypeDescriptor();
			FOREACH(Ptr<GuiInstanceParameter>, parameter, env->context->parameters)
			{
				auto info = td->GetPropertyByName(parameter->name.ToString(), true);
				if (!info)
				{
					env->scope->errors.Add(L"Cannot find parameter \"" + parameter->name.ToString() + L"\" in properties of \"" + td->GetTypeName() + L"\".");
					continue;
				}

				auto parameterTd = GetTypeDescriptor(parameter->className.ToString());
				if (!parameterTd)
				{
					env->scope->errors.Add(L"Cannot find type \"" + parameter->className.ToString() + L"\" of parameter \"" + parameter->name.ToString() + L"\".");
				}

				auto value = info->GetValue(env->scope->rootInstance);
				if (parameterTd && (!value.GetTypeDescriptor() || !value.GetTypeDescriptor()->CanConvertTo(parameterTd)))
				{
					env->scope->errors.Add(L"Value of parameter \"" + parameter->name.ToString() + L"\" is not \"" + parameterTd->GetTypeName() + L"\" which is required.");
				}

				if (env->scope->referenceValues.Keys().Contains(parameter->name))
				{
					env->scope->errors.Add(L"Parameter \"" + parameter->name.ToString() + L"\" conflict with an existing named object.");
				}
				else
				{
					env->scope->referenceValues.Add(parameter->name, value);
				}
			}
		}

/***********************************************************************
ExecuteBindingSetters
***********************************************************************/

		bool PrepareBindingContext(
			Ptr<GuiInstanceEnvironment> env,
			collections::List<GlobalStringKey>& contextNames,
			const WString& dependerType,
			const GlobalStringKey& dependerName
			)
		{
			bool success = true;
			FOREACH(GlobalStringKey, contextName, contextNames)
			{
				if (!env->scope->bindingContexts.Keys().Contains(contextName))
				{
					auto factory = GetInstanceLoaderManager()->GetInstanceBindingContextFactory(contextName);
					if (factory)
					{
						env->scope->bindingContexts.Add(contextName, factory->CreateContext());
					}
					else
					{
						env->scope->errors.Add(
							L"Failed to create binding context \"" +
							contextName.ToString() +
							L"\" which is required by " +
							dependerType +
							L" \"" +
							dependerName.ToString() +
							L"\".");
						success = false;
					}
				}
			}
			return success;
		}

		void ExecuteBindingSetters(
			Ptr<GuiInstanceEnvironment> env,
			List<FillInstanceBindingSetter>& bindingSetters
			)
		{
			// set all binding attributes
			FOREACH(FillInstanceBindingSetter, bindingSetter, bindingSetters)
			{
				List<GlobalStringKey> contextNames;
				bindingSetter.binder->GetRequiredContexts(contextNames);
				bool success = PrepareBindingContext(env, contextNames, L"property binding", bindingSetter.binder->GetBindingName());

				if (bindingSetter.binder->RequireInstanceName())
				{
					if (bindingSetter.bindingTarget->instanceName == GlobalStringKey::Empty)
					{
						auto name = GlobalStringKey::Get(L"<temp>" + itow(env->scope->referenceValues.Count()));
						bindingSetter.bindingTarget->instanceName = name;
					}

					auto name = bindingSetter.bindingTarget->instanceName;
					auto value = bindingSetter.propertyValue.instanceValue;
					if (!env->scope->referenceValues.Keys().Contains(bindingSetter.bindingTarget->instanceName))
					{
						env->scope->referenceValues.Add(name, value);
					}
				}

				if (!success || !bindingSetter.binder->SetPropertyValue(env, bindingSetter.loader, bindingSetter.bindingTarget->instanceName, bindingSetter.propertyValue))
				{
					auto value = bindingSetter.propertyValue.propertyValue;
					env->scope->errors.Add(
						L"Failed to set property \"" +
						bindingSetter.propertyValue.propertyName.ToString() +
						L"\" of \"" +
						bindingSetter.propertyValue.instanceValue.GetTypeDescriptor()->GetTypeName() +
						L"\" using binding \"" +
						bindingSetter.binder->GetBindingName().ToString() +
						L"\" and value \"" +
						(
							value.GetValueType() == Value::Null ? WString(L"null") :
							value.GetValueType() == Value::Text ? value.GetText() :
							(L"<" + value.GetTypeDescriptor()->GetTypeName() + L">")
						) +
						L"\".");
					bindingSetter.propertyValue.propertyValue.DeleteRawPtr();
				}
			}

			// initialize all binding context
			FOREACH(Ptr<IGuiInstanceBindingContext>, context, env->scope->bindingContexts.Values())
			{
				context->Initialize(env);
			}
		}

/***********************************************************************
ExecuteBindingSetters
***********************************************************************/

		void ExecuteEventSetters(
			description::Value createdInstance,
			Ptr<GuiInstanceEnvironment> env,
			List<FillInstanceEventSetter>& eventSetters
			)
		{
#ifndef VCZH_DEBUG_NO_REFLECTION
			// set all event attributes
			FOREACH(FillInstanceEventSetter, eventSetter, eventSetters)
			{
				if (eventSetter.binder)
				{
					List<GlobalStringKey> contextNames;
					eventSetter.binder->GetRequiredContexts(contextNames);
					auto propertyValue = eventSetter.propertyValue;
					propertyValue.propertyValue = BoxValue(eventSetter.handlerName);
					bool success = PrepareBindingContext(env, contextNames, L"event binding", eventSetter.binder->GetBindingName());

					if (eventSetter.binder->RequireInstanceName())
					{
						if (eventSetter.bindingTarget->instanceName == GlobalStringKey::Empty)
						{
							auto name = GlobalStringKey::Get(L"<temp>" + itow(env->scope->referenceValues.Count()));
							eventSetter.bindingTarget->instanceName = name;
						}

						auto name = eventSetter.bindingTarget->instanceName;
						auto value = eventSetter.propertyValue.instanceValue;
						if (!env->scope->referenceValues.Keys().Contains(eventSetter.bindingTarget->instanceName))
						{
							env->scope->referenceValues.Add(name, value);
						}
					}

					if (!success || !eventSetter.binder->AttachEvent(env, eventSetter.loader, eventSetter.bindingTarget->instanceName, propertyValue))
					{
						env->scope->errors.Add(
							L"Failed to attach event \"" +
							propertyValue.propertyName.ToString() +
							L"\" of type \"" +
							propertyValue.instanceValue.GetTypeDescriptor()->GetTypeName() +
							L"\" with the handler \"" +
							propertyValue.propertyValue.GetText() +
							L"\" using event binding \"" +
							eventSetter.binder->GetBindingName().ToString() +
							L"\".");
					}
				}
				else if (auto group = createdInstance.GetTypeDescriptor()->GetMethodGroupByName(eventSetter.handlerName, true))
				{
					// find a correct method
					vint count = group->GetMethodCount();
					IMethodInfo* selectedMethod = 0;
					for (vint i = 0; i < count; i++)
					{
						auto method = group->GetMethod(i);
						if (method->GetParameterCount() != 2) goto UNSUPPORTED;

						{
							auto returnType = method->GetReturn();
							auto senderType = method->GetParameter(0)->GetType();
							auto argumentType = method->GetParameter(1)->GetType();
					
							if (returnType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
							if (returnType->GetTypeDescriptor() != description::GetTypeDescriptor<VoidValue>()) goto UNSUPPORTED;
					
							if (senderType->GetDecorator() != ITypeInfo::RawPtr) goto UNSUPPORTED;
							senderType = senderType->GetElementType();
							if (senderType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
							if (senderType->GetTypeDescriptor() != description::GetTypeDescriptor<compositions::GuiGraphicsComposition>()) goto UNSUPPORTED;
					
							if (argumentType->GetDecorator() != ITypeInfo::RawPtr) goto UNSUPPORTED;
							argumentType = argumentType->GetElementType();
							if (argumentType->GetDecorator() != ITypeInfo::TypeDescriptor) goto UNSUPPORTED;
							if (argumentType->GetTypeDescriptor() != eventSetter.eventInfo->argumentType) goto UNSUPPORTED;

							selectedMethod = method;
							break;
						}

					UNSUPPORTED:
						continue;
					}

					if (selectedMethod)
					{
						Value proxy = selectedMethod->CreateFunctionProxy(createdInstance);
						if (!proxy.IsNull())
						{
							auto propertyValue = eventSetter.propertyValue;
							propertyValue.propertyValue = proxy;
							eventSetter.loader->SetEventValue(propertyValue);
						}
					}
					else
					{
						env->scope->errors.Add(
							L"Event handler \"" +
							eventSetter.handlerName +
							L"\" of \"" +
							env->context->instance->typeName.ToString() +
							L"\" exists but the type does not match the event \"" +
							eventSetter.propertyValue.propertyName.ToString() +
							L"\".");
					}
				}
				else
				{
					env->scope->errors.Add(
						L"Failed to find event handler \"" +
						eventSetter.handlerName +
						L"\" of \"" +
						env->context->instance->typeName.ToString() +
						L"\" when setting event \"" +
						eventSetter.propertyValue.propertyName.ToString() +
						L"\".");
				}
			}
#endif
		}

/***********************************************************************
LoadInstance
***********************************************************************/

		Ptr<GuiInstanceContextScope> LoadInstanceFromContext(
			Ptr<GuiInstanceContext> context,
			Ptr<GuiResourcePathResolver> resolver,
			description::ITypeDescriptor* expectedType
			)
		{
			Ptr<GuiInstanceEnvironment> env = new GuiInstanceEnvironment(context, resolver);
			List<FillInstanceBindingSetter> bindingSetters;
			List<FillInstanceEventSetter> eventSetters;
			Value instance = CreateInstance(env, context->instance.Obj(), expectedType, env->scope->typeName, bindingSetters, eventSetters, true);
			
			if (!instance.IsNull())
			{
				ExecuteBindingSetters(env, bindingSetters);
				ExecuteEventSetters(instance, env, eventSetters);
				return env->scope;
			}
			return 0;
		}

		Ptr<GuiInstanceContextScope> LoadInstance(
			Ptr<GuiResource> resource,
			const WString& instancePath,
			description::ITypeDescriptor* expectedType
			)
		{
			Ptr<GuiInstanceContext> context=resource->GetValueByPath(instancePath).Cast<GuiInstanceContext>();
			if (context)
			{
				Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
				return LoadInstanceFromContext(context, resolver, expectedType);
			}
			return 0;
		}

/***********************************************************************
InitializeInstance
***********************************************************************/

		void InitializeInstanceFromConstructor(
			Ptr<GuiInstanceEnvironment> env,
			GuiConstructorRepr* ctor,
			IGuiInstanceLoader* instanceLoader,
			GlobalStringKey typeName,
			description::Value instance,
			bool deserialized,
			List<FillInstanceBindingSetter>& bindingSetters,
			List<FillInstanceEventSetter>& eventSetters
			)
		{
			// fill all attributes
			FillInstance(instance, env, ctor, instanceLoader, deserialized, typeName, bindingSetters, eventSetters);

			if (ctor->instanceName != GlobalStringKey::Empty)
			{
				if (env->scope->referenceValues.Keys().Contains(ctor->instanceName))
				{
					env->scope->errors.Add(L"Parameter \"" + ctor->instanceName.ToString() + L"\" conflict with an existing named object.");
				}
				else
				{
					env->scope->referenceValues.Add(ctor->instanceName, instance);
				}
			}
		}

		Ptr<GuiInstanceContextScope> InitializeInstanceFromContext(
			Ptr<GuiInstanceContext> context,
			Ptr<GuiResourcePathResolver> resolver,
			description::Value instance
			)
		{
			List<FillInstanceBindingSetter> bindingSetters;
			List<FillInstanceEventSetter> eventSetters;

			// search for a correct loader
			GuiConstructorRepr* ctor = context->instance.Obj();
			Ptr<GuiInstanceEnvironment> env = new GuiInstanceEnvironment(context, resolver);
			InstanceLoadingSource source = FindInstanceLoadingSource(env->context, ctor);

			// initialize the instance
			if(source.loader)
			{
				env->scope->rootInstance = instance;
				ExecuteParameters(env);
				InitializeInstanceFromConstructor(env, ctor, source.loader, source.typeName, instance, false, bindingSetters, eventSetters);
				ExecuteBindingSetters(env, bindingSetters);
				ExecuteEventSetters(instance, env, eventSetters);
				return env->scope;
			}
			return 0;
		}

		Ptr<GuiInstanceContextScope> InitializeInstance(
			Ptr<GuiResource> resource,
			const WString& instancePath,
			description::Value instance
			)
		{
			if (instance.GetRawPtr())
			{
				Ptr<GuiInstanceContext> context=resource->GetValueByPath(instancePath).Cast<GuiInstanceContext>();
				if (context)
				{
					Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
					return InitializeInstanceFromContext(context, resolver, instance);
				}
			}
			return 0;
		}
	}
}