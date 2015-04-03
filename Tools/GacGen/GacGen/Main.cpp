#include "GacGen.h"

Array<WString>* arguments = 0;

int wmain(int argc, wchar_t* argv[])
{
	Array<WString> _arguments(argc - 1);
	for (vint i = 1; i < argc; i++)
	{
		_arguments[i - 1] = argv[i];
	}
	arguments = &_arguments;
	SetupWindowsDirect2DRenderer();
}

class GacGenException : public TypeDescriptorException
{
public:
	GacGenException(const WString& message)
		:TypeDescriptorException(message)
	{
	}
};

class ResourceMockTypeLoader : public Object, public ITypeLoader
{
protected:
	Ptr<GuiResource>							resource;
	Ptr<CodegenConfig>							config;
	Regex										regexClassName;

public:
	Dictionary<WString, Ptr<InstanceSchema>>	typeSchemas;
	List<WString>								typeSchemaOrder;
	Dictionary<WString, Ptr<Instance>>			instances;

private:

	class FieldInfo : public FieldInfoImpl
	{
	protected:

		Value GetValueInternal(const Value& thisObject)override
		{
			throw GacGenException(L"Field \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

		void SetValueInternal(Value& thisObject, const Value& newValue)override
		{
			throw GacGenException(L"Field \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

	public:
		FieldInfo(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, Ptr<ITypeInfo> _returnInfo)
			:FieldInfoImpl(_ownerTypeDescriptor, _name, _returnInfo)
		{
		}
	};

	class EventInfo : public EventInfoImpl
	{
	protected:
		void AttachInternal(DescriptableObject* thisObject, IEventHandler* eventHandler)override
		{
			throw GacGenException(L"Event \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

		void DetachInternal(DescriptableObject* thisObject, IEventHandler* eventHandler)override
		{
			throw GacGenException(L"Event \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

		void InvokeInternal(DescriptableObject* thisObject, collections::Array<Value>& arguments)override
		{
			throw GacGenException(L"Event \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

		Ptr<ITypeInfo> GetHandlerTypeInternal()override
		{
			auto voidType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
			voidType->SetTypeDescriptor(description::GetTypeDescriptor<void>());

			auto funcType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
			funcType->SetTypeDescriptor(description::GetTypeDescriptor<IValueFunctionProxy>());

			auto genericType = MakePtr<TypeInfoImpl>(ITypeInfo::Generic);
			genericType->SetElementType(funcType);
			genericType->AddGenericArgument(voidType);

			auto pointerType = MakePtr<TypeInfoImpl>(ITypeInfo::SharedPtr);
			pointerType->SetElementType(genericType);

			return pointerType;
		}
	public:
		EventInfo(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name)
			:EventInfoImpl(_ownerTypeDescriptor, _name)
		{
		}
	};

	class ConstructorInfo : public MethodInfoImpl
	{
	protected:
		
		Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
		{
			throw GacGenException(L"Constructor of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

		Value CreateFunctionProxyInternal(const Value& thisObject)override
		{
			throw GacGenException(L"Constructor of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

	public:
		ConstructorInfo(Ptr<ITypeInfo> _return)
			:MethodInfoImpl(nullptr, _return, true)
		{
		}
	};

	class MethodInfo : public MethodInfoImpl
	{
	protected:
		
		Value InvokeInternal(const Value& thisObject, collections::Array<Value>& arguments)override
		{
			throw GacGenException(L"Method \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

		Value CreateFunctionProxyInternal(const Value& thisObject)override
		{
			throw GacGenException(L"Method \"" + GetName() + L"\" of mocking type \"" + GetOwnerTypeDescriptor()->GetTypeName() + L"\" is not accessible.");
		}

	public:
		MethodInfo(Ptr<ITypeInfo> _return)
			:MethodInfoImpl(nullptr, _return, false)
		{
		}
	};

	class ClassTypeDescriptor : public TypeDescriptorImpl
	{
	protected:
		ResourceMockTypeLoader*					loader;
		Ptr<GuiInstanceDataSchema>				schema;

		void LoadInternal()override
		{
			FOREACH(Ptr<GuiInstancePropertySchame>, prop, schema->properties)
			{
				if (!IsPropertyExists(prop->name, false))
				{
					if (auto type = GetTypeInfoFromWorkflowType(loader->config, prop->typeName))
					{
						AddProperty(new FieldInfo(this, prop->name, type));
					}
				}
			}

			auto descriptorType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
			descriptorType->SetTypeDescriptor(this);

			auto pointerType = MakePtr<TypeInfoImpl>(ITypeInfo::SharedPtr);
			pointerType->SetElementType(descriptorType);

			AddConstructor(new ConstructorInfo(pointerType));
		}

	public:
		ClassTypeDescriptor(ResourceMockTypeLoader* _loader, Ptr<GuiInstanceDataSchema> _schema)
			:TypeDescriptorImpl(_schema->typeName, _schema->typeName)
			, loader(_loader)
			, schema(_schema)
		{
		}
	};

	class StructTypeDescriptor : public TypeDescriptorImpl
	{
	private:

		class Serializer : public Object, public virtual IValueSerializer
		{
		protected:
			StructTypeDescriptor*				ownerTypeDescriptor;

		public:
			Serializer(StructTypeDescriptor* _ownerTypeDescriptor)
				:ownerTypeDescriptor(_ownerTypeDescriptor)
			{
			}

			ITypeDescriptor* GetOwnerTypeDescriptor()override
			{
				return ownerTypeDescriptor;
			}

			bool Validate(const WString& text)override
			{
				return false;
			}

			bool Parse(const WString& input, Value& output)override
			{
				return false;
			}

			WString GetDefaultText()override
			{
				return L"";
			}

			bool HasCandidate()override
			{
				return false;
			}

			vint GetCandidateCount()override
			{
				return 0;
			}

			WString GetCandidate(vint index)override
			{
				return L"";
			}

			bool CanMergeCandidate()override
			{
				return false;
			}
		};

	protected:
		ResourceMockTypeLoader*					loader;
		Ptr<GuiInstanceDataSchema>				schema;
		Ptr<IValueSerializer>					serializer;

		void LoadInternal()override
		{
			FOREACH(Ptr<GuiInstancePropertySchame>, prop, schema->properties)
			{
				if (!IsPropertyExists(prop->name, false))
				{
					if (auto type = GetTypeInfoFromWorkflowType(loader->config, prop->typeName))
					{
						AddProperty(new FieldInfo(this, prop->name, type));
					}
				}
			}
		}

	public:
		StructTypeDescriptor(ResourceMockTypeLoader* _loader, Ptr<GuiInstanceDataSchema> _schema)
			:TypeDescriptorImpl(_schema->typeName, _schema->typeName)
			, loader(_loader)
			, schema(_schema)
		{
			serializer = new Serializer(this);
		}

		IValueSerializer* GetValueSerializer()override
		{
			return serializer.Obj();
		}
	};

	class InterfaceTypeDescriptor : public TypeDescriptorImpl
	{
	protected:
		ResourceMockTypeLoader*					loader;
		Ptr<GuiInstanceInterfaceSchema>			schema;

		void LoadInternal()override
		{
			auto voidType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
			voidType->SetTypeDescriptor(description::GetTypeDescriptor<void>());

			if (schema->parentType != L"")
			{
				if (auto td = description::GetTypeDescriptor(schema->parentType))
				{
					AddBaseType(td);
				}
			}

			FOREACH(Ptr<GuiInstancePropertySchame>, prop, schema->properties)
			{
				if (!IsPropertyExists(prop->name, false))
				{
					if (auto type = GetTypeInfoFromWorkflowType(loader->config, prop->typeName))
					{
						auto eventInfo = prop->observable ? MakePtr<EventInfo>(this, prop->name + L"Changed") : nullptr;
						auto getter = MakePtr<MethodInfo>(type);
						auto setter = prop->readonly ? nullptr : MakePtr<MethodInfo>(voidType);
						if (setter)
						{
							auto parameterInfo = new ParameterInfoImpl(setter.Obj(), L"value", type);
							setter->AddParameter(parameterInfo);
						}
						auto propInfo = MakePtr<PropertyInfoImpl>(this, prop->name, getter.Obj(), setter.Obj(), eventInfo.Obj());

						if (eventInfo)
						{
							AddEvent(eventInfo);
						}
						AddMethod(L"Get" + prop->name, getter);
						if (setter)
						{
							AddMethod(L"Set" + prop->name, setter);
						}
						AddProperty(propInfo);
					}
				}
			}

			FOREACH(Ptr<GuiInstanceMethodSchema>, method, schema->methods)
			{
				auto returnType = GetTypeInfoFromWorkflowType(loader->config, method->returnType);
				if (!returnType) returnType = voidType;

				auto methodInfo = MakePtr<MethodInfo>(returnType);
				FOREACH(Ptr<GuiInstancePropertySchame>, argument, method->arguments)
				{
					if (auto type = GetTypeInfoFromWorkflowType(loader->config, argument->typeName))
					{
						auto parameterInfo = new ParameterInfoImpl(methodInfo.Obj(), argument->name, type);
						methodInfo->AddParameter(parameterInfo);
					}
				}

				AddMethod(method->name, methodInfo);
			}
		}

	public:
		InterfaceTypeDescriptor(ResourceMockTypeLoader* _loader, Ptr<GuiInstanceInterfaceSchema> _schema)
			:TypeDescriptorImpl(_schema->typeName, _schema->typeName)
			, loader(_loader)
			, schema(_schema)
		{
		}
	};

	class InstanceTypeDescriptor : public TypeDescriptorImpl
	{
	protected:
		ResourceMockTypeLoader*					loader;
		ITypeDescriptor*						baseType;
		Ptr<GuiInstanceContext>					context;

		void LoadInternal()override
		{
			{
				auto descriptorType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
				descriptorType->SetTypeDescriptor(this);

				auto pointerType = MakePtr<TypeInfoImpl>(ITypeInfo::RawPtr);
				pointerType->SetElementType(descriptorType);

				auto ctor = new ConstructorInfo(pointerType);
				FOREACH(Ptr<GuiInstanceParameter>, parameter, context->parameters)
				{
					if (auto td = description::GetTypeDescriptor(parameter->className.ToString()))
					{
						auto descriptorType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
						descriptorType->SetTypeDescriptor(td);

						auto pointerType = MakePtr<TypeInfoImpl>(ITypeInfo::SharedPtr);
						pointerType->SetElementType(descriptorType);

						auto parameterInfo = new ParameterInfoImpl(ctor, parameter->name.ToString(), pointerType);
						ctor->AddParameter(parameterInfo);

						auto getter = MakePtr<MethodInfo>(pointerType);
						AddMethod(L"Get" + parameter->name.ToString(), getter);

						auto propInfo = MakePtr<PropertyInfoImpl>(this, parameter->name.ToString(), getter.Obj(), nullptr, nullptr);
						AddProperty(propInfo);
					}
				}
				AddConstructor(ctor);
			}

			AddBaseType(baseType);

			auto voidType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
			voidType->SetTypeDescriptor(description::GetTypeDescriptor<void>());

			FOREACH(Ptr<GuiInstanceProperty>, prop, context->properties)
			{
				if (!IsPropertyExists(prop->name.ToString(), false))
				{
					if (auto type = GetTypeInfoFromWorkflowType(loader->config, prop->typeName))
					{
						auto eventInfo = MakePtr<EventInfo>(this, prop->name.ToString() + L"Changed");
						auto getter = MakePtr<MethodInfo>(type);
						auto setter = prop->readonly ? nullptr : MakePtr<MethodInfo>(voidType);
						if (setter)
						{
							auto parameterInfo = new ParameterInfoImpl(setter.Obj(), L"value", type);
							setter->AddParameter(parameterInfo);
						}
						auto propInfo = MakePtr<PropertyInfoImpl>(this, prop->name.ToString(), getter.Obj(), setter.Obj(), eventInfo.Obj());

						AddEvent(eventInfo);
						AddMethod(L"Get" + prop->name.ToString(), getter);
						if (setter)
						{
							AddMethod(L"Set" + prop->name.ToString(), setter);
						}
						AddProperty(propInfo);
					}
				}
			}

			FOREACH(Ptr<GuiInstanceState>, state, context->states)
			{
				if (!IsPropertyExists(state->name.ToString(), false))
				{
					if (auto type = GetTypeInfoFromWorkflowType(loader->config, state->typeName))
					{
						AddProperty(new FieldInfo(this, state->name.ToString(), type));
					}
				}
			}
		}

	public:
		InstanceTypeDescriptor(ResourceMockTypeLoader* _loader, ITypeDescriptor* _baseType, Ptr<GuiInstanceContext> _context)
			:TypeDescriptorImpl(_context->className.Value(), _context->className.Value())
			, loader(_loader)
			, baseType(_baseType)
			, context(_context)
		{
		}
	};

public:
	ResourceMockTypeLoader(Ptr<GuiResource> _resource, Ptr<CodegenConfig> _config)
		:resource(_resource)
		, config(_config)
		, regexClassName(L"((<namespace>[^:]+)::)*(<type>[^:]+)")
	{
		Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());

		SearchAllSchemas(regexClassName, resource, typeSchemas, typeSchemaOrder);
		SearchAllInstances(regexClassName, resolver, resource, instances);
		
		FOREACH(Ptr<Instance>, instance, instances.Values())
		{
			Ptr<GuiInstanceEnvironment> env = new GuiInstanceEnvironment(instance->context, resolver);
			SearchAllEventHandlers(config, instances, instance, env, instance->eventHandlers);
		}
	}

	void Load(ITypeManager* manager)
	{
		FOREACH(Ptr<InstanceSchema>, schema, typeSchemas.Values())
		{
			Ptr<ITypeDescriptor> typeDescriptor;
			if (auto dataType = schema->schema.Cast<GuiInstanceDataSchema>())
			{
				if (dataType->referenceType)
				{
					typeDescriptor = new ClassTypeDescriptor(this, dataType);
				}
				else
				{
					typeDescriptor = new StructTypeDescriptor(this, dataType);
				}
			}
			else if (auto interfaceType = schema->schema.Cast<GuiInstanceInterfaceSchema>())
			{
				typeDescriptor = new InterfaceTypeDescriptor(this, interfaceType);
			}
			manager->SetTypeDescriptor(typeDescriptor->GetTypeName(), typeDescriptor);
		}

		FOREACH(Ptr<Instance>, instance, instances.Values())
		{
			if (instance->context->className && instance->baseType)
			{
				Ptr<ITypeDescriptor> typeDescriptor = new InstanceTypeDescriptor(this, instance->baseType, instance->context);
				manager->SetTypeDescriptor(typeDescriptor->GetTypeName(), typeDescriptor);
			}
		}
	}

	void Unload(ITypeManager* manager)
	{
	}
};

void GuiMain()
{
	Console::WriteLine(L"Vczh GacUI Resource Code Generator for C++");
	if (arguments->Count() != 1)
	{
		PrintErrorMessage(L"GacGen.exe only accept 1 input file.");
		return;
	}

	WString inputPath = arguments->Get(0);
	PrintSuccessMessage(L"gacgen> Making : " + inputPath);
	List<WString> errors;
	auto resource = GuiResource::LoadFromXml(arguments->Get(0), errors);
	if (!resource)
	{
		PrintErrorMessage(L"error> Failed to load resource.");
		return;
	}

	auto config = CodegenConfig::LoadConfig(resource);
	if (!config)
	{
		PrintErrorMessage(L"error> Failed to load config.");
		return;
	}

	auto typeLoader = MakePtr<ResourceMockTypeLoader>(resource, config);
	GetGlobalTypeManager()->AddTypeLoader(typeLoader);
	PrintSuccessMessage(L"gacgen> Compiling...");
	resource->Precompile(errors);
	FOREACH(WString, error, errors)
	{
		PrintErrorMessage(error);
	}
	GetInstanceLoaderManager()->SetResource(L"GACGEN", resource);

	FOREACH(Ptr<Instance>, instance, typeLoader->instances.Values())
	{
		if (instance->context->codeBehind)
		{
			WriteControlClassHeaderFile(config, instance);
			WriteControlClassCppFile(config, instance);
		}
	}
	WritePartialClassHeaderFile(config, typeLoader->typeSchemas, typeLoader->typeSchemaOrder, typeLoader->instances);
	WritePartialClassCppFile(config, typeLoader->typeSchemas, typeLoader->typeSchemaOrder, typeLoader->instances);
	WriteGlobalHeaderFile(config, typeLoader->instances);

	if (errors.Count() > 0)
	{
		PrintErrorMessage(L"Skip generate precompiled resource because there are compilation errors.");
	}
	else
	{
		if (config->precompiledOutput != L"")
		{
			WString fileName = config->precompiledOutput;
			auto xml = resource->SaveToXml(true);
			OPEN_FILE(L"Precompiled Xml Resource");
			XmlPrint(xml, writer);
		}
		if (config->precompiledBinary != L"")
		{
			WString fileName = config->precompiledBinary;
			OPEN_BINARY_FILE(L"Precompiled Binary Resource");
			resource->SavePrecompiledBinary(fileStream);
		}
		if (config->precompiledCompressed != L"")
		{
			WString fileName = config->precompiledCompressed;
			OPEN_BINARY_FILE(L"Precompiled Compressed Binary Resource");
			LzwEncoder encoder;
			EncoderStream encoderStream(fileStream, encoder);
			resource->SavePrecompiledBinary(encoderStream);
		}
	}
}