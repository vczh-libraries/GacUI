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
	Ptr<GuiResource>								resource;
	Ptr<CodegenConfig>								config;
	Regex											regexClassName;

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
	Ptr<WfLexicalScopeManager>						schemaManager;
	Dictionary<WString, Ptr<WfClassDeclaration>>	typeSchemas;
	List<WString>									typeSchemaOrder;
	Dictionary<WString, Ptr<Instance>>				instances;

protected:

	bool HasSchemaError(Dictionary<vint, WString>& schemaPathMap)
	{
		if (schemaManager->errors.Count() == 0)
		{
			return false;
		}

		FOREACH(Ptr<ParsingError>, error, schemaManager->errors)
		{
			if (schemaPathMap.Keys().Contains(error->codeRange.codeIndex))
			{
				PrintErrorMessage(
					L"Schema: " + schemaPathMap[error->codeRange.codeIndex] +
					L", Row: " + itow(error->codeRange.start.row + 1) +
					L", Column: " + itow(error->codeRange.start.column + 1) +
					L", Message: " + error->errorMessage);
			}
		}
		return true;
	}

public:
	ResourceMockTypeLoader(Ptr<GuiResource> _resource, Ptr<CodegenConfig> _config)
		:resource(_resource)
		, config(_config)
		, regexClassName(L"((<namespace>[^:]+)::)*(<type>[^:]+)")
	{
		Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(resource, resource->GetWorkingDirectory());
		{
			List<WString> schemaPaths;
			List<Ptr<GuiInstanceSharedScript>> schemas;
			Dictionary<vint, WString> schemaPathMap;
			SearchAllSchemas(regexClassName, resource, schemaPaths, schemas);

			if (schemas.Count() > 0)
			{
				if (!config->workflowTable)
				{
					config->workflowTable = WfLoadTable();
				}
				schemaManager = new WfLexicalScopeManager(config->workflowTable);

				FOREACH_INDEXER(Ptr<GuiInstanceSharedScript>, schema, schemaIndex, schemas)
				{
					vint codeIndex = schemaManager->AddModule(schema->code);
					schemaPathMap.Add(codeIndex, schemaPaths[schemaIndex]);
				}

				if (!HasSchemaError(schemaPathMap))
				{
					schemaManager->Rebuild(false);
					if (!HasSchemaError(schemaPathMap))
					{
						List<Ptr<WfDeclaration>> decls;
						CopyFrom(
							decls,
							From(schemaManager->GetModules())
								.SelectMany([](Ptr<WfModule> module)
								{
									return LazyList<Ptr<WfDeclaration>>(module->declarations);
								})
							);
						for (vint i = 0; i < decls.Count(); i++)
						{
							auto decl = decls[i];
							if (auto nsDecl = decl.Cast<WfNamespaceDeclaration>())
							{
								CopyFrom(decls, nsDecl->declarations, true);
							}
							else if (auto classDecl = decl.Cast<WfClassDeclaration>())
							{
								if (classDecl->kind == WfClassKind::Interface)
								{
									auto td = schemaManager->declarationTypes[classDecl.Obj()];
									auto typeName = td->GetTypeName();
									typeSchemas.Add(typeName, classDecl);
									typeSchemaOrder.Add(typeName);
								}
							}
						}
					}
				}
			}
		}
		SearchAllInstances(regexClassName, resolver, resource, instances);

		FOREACH(Ptr<Instance>, instance, instances.Values())
		{
			Ptr<GuiInstanceEnvironment> env = new GuiInstanceEnvironment(instance->context, resolver);
			SearchAllEventHandlers(config, instances, instance, env, instance->eventHandlers);
		}
	}

	void Load(ITypeManager* manager)
	{
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

	if (config->cppOutput)
	{
		filesystem::Folder(resource->GetWorkingDirectory() + config->cppOutput->output).Create(true);
		FOREACH(Ptr<Instance>, instance, typeLoader->instances.Values())
		{
			if (instance->context->codeBehind)
			{
				WriteControlClassHeaderFile(config, instance);
				WriteControlClassCppFile(config, instance);
			}
		}
		WritePartialClassHeaderFile(config, typeLoader->schemaManager, typeLoader->typeSchemas, typeLoader->typeSchemaOrder, typeLoader->instances);
		WritePartialClassCppFile(config, typeLoader->schemaManager, typeLoader->typeSchemas, typeLoader->typeSchemaOrder, typeLoader->instances);
		WriteGlobalHeaderFile(config, typeLoader->instances);
	}

	if (errors.Count() > 0)
	{
		PrintErrorMessage(L"Skip generate precompiled resource because there are compilation errors.");
	}
	else if (config->resOutput)
	{
		filesystem::Folder(resource->GetWorkingDirectory() + config->resOutput->output).Create(true);
		if (config->resOutput->precompiledOutput != L"")
		{
			WString fileName = config->resOutput->output + config->resOutput->precompiledOutput;
			auto xml = resource->SaveToXml(true);
			OPEN_FILE(L"Precompiled Xml Resource");
			XmlPrint(xml, writer);
		}
		if (config->resOutput->precompiledBinary != L"")
		{
			WString fileName = config->resOutput->output + config->resOutput->precompiledBinary;
			OPEN_BINARY_FILE(L"Precompiled Binary Resource");
			resource->SavePrecompiledBinary(fileStream);
		}
		if (config->resOutput->precompiledCompressed != L"")
		{
			WString fileName = config->resOutput->output + config->resOutput->precompiledCompressed;
			OPEN_BINARY_FILE(L"Precompiled Compressed Binary Resource");
			LzwEncoder encoder;
			EncoderStream encoderStream(fileStream, encoder);
			resource->SavePrecompiledBinary(encoderStream);
		}
	}
}