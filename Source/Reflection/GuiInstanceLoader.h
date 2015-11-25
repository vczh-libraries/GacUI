/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Loader

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONINSTANCELOADER
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONINSTANCELOADER

#include "../Controls/GuiApplication.h"
#include "../Controls/Styles/GuiThemeStyleFactory.h"
#include "GuiInstanceRepresentation.h"
#include "../GacWorkflowReferences.h"

namespace vl
{
	namespace presentation
	{
		using namespace reflection;

/***********************************************************************
Instance Environment
***********************************************************************/

		class GuiInstanceContextScope : public Object, public Description<GuiInstanceContextScope>
		{
			typedef collections::Dictionary<GlobalStringKey, description::Value>				ValueMap;
			typedef collections::List<WString>													ErrorList;
		public:
			GlobalStringKey							typeName;
			description::Value						rootInstance;
			ValueMap								referenceValues;
			ErrorList								errors;
		};

		class GuiInstanceEnvironment : public Object, public Description<GuiInstanceEnvironment>
		{
		public:
			Ptr<GuiInstanceContext>					context;
			WString									path;
			Ptr<GuiResourcePathResolver>			resolver;
			Ptr<GuiInstanceContextScope>			scope;

			GuiInstanceEnvironment(Ptr<GuiInstanceContext> _context, const WString& _path, Ptr<GuiResourcePathResolver> _resolver)
				:context(_context)
				, path(_path)
				, resolver(_resolver)
			{
				scope = new GuiInstanceContextScope;
			}
		};

/***********************************************************************
Instance Loader
***********************************************************************/

		class GuiInstancePropertyInfo : public IDescriptable, public Description<GuiInstancePropertyInfo>
		{
			typedef collections::List<description::ITypeDescriptor*>		TypeDescriptorList;
		public:
			enum Support
			{
				NotSupport,
				SupportAssign,
				SupportArray,
				SupportCollection,
				SupportSet,
			};

			enum PropertyScope
			{
				ViewModel,		// <ref.Parameter/>
				Constructor,	// constructor parameter that is not ViewModel
				Property,		// property of the class
			};

			Support									support;
			bool									tryParent;
			bool									required;
			PropertyScope							scope;
			TypeDescriptorList						acceptableTypes;

			GuiInstancePropertyInfo();
			~GuiInstancePropertyInfo();

			static Ptr<GuiInstancePropertyInfo>		Unsupported();
			static Ptr<GuiInstancePropertyInfo>		Assign(description::ITypeDescriptor* typeDescriptor = 0);
			static Ptr<GuiInstancePropertyInfo>		AssignWithParent(description::ITypeDescriptor* typeDescriptor = 0);
			static Ptr<GuiInstancePropertyInfo>		Collection(description::ITypeDescriptor* typeDescriptor = 0);
			static Ptr<GuiInstancePropertyInfo>		CollectionWithParent(description::ITypeDescriptor* typeDescriptor = 0);
			static Ptr<GuiInstancePropertyInfo>		Set(description::ITypeDescriptor* typeDescriptor = 0);
			static Ptr<GuiInstancePropertyInfo>		Array(description::ITypeDescriptor* typeDescriptor = 0);
		};

		class IGuiInstanceLoader : public IDescriptable, public Description<IGuiInstanceLoader>
		{
		public:
			struct TypeInfo
			{
				GlobalStringKey						typeName;
				description::ITypeDescriptor*		typeDescriptor;

				TypeInfo() :typeDescriptor(0){}
				TypeInfo(GlobalStringKey _typeName, description::ITypeDescriptor* _typeDescriptor)
					:typeName(_typeName)
					, typeDescriptor(_typeDescriptor)
				{
				}
			};

			struct PropertyInfo
			{
				TypeInfo							typeInfo;
				GlobalStringKey						propertyName;

				PropertyInfo(){}
				PropertyInfo(const TypeInfo& _typeInfo, GlobalStringKey _propertyName)
					:typeInfo(_typeInfo)
					, propertyName(_propertyName)
				{
				}
			};

			struct PropertyValue : PropertyInfo
			{
				description::Value					instanceValue;
				description::Value					propertyValue;

				PropertyValue(){}
				PropertyValue(const TypeInfo& _typeInfo, GlobalStringKey _propertyName, description::Value _instanceValue, description::Value _propertyValue = description::Value())
					:PropertyInfo(_typeInfo, _propertyName)
					, instanceValue(_instanceValue)
					, propertyValue(_propertyValue)
				{
				}
			};

			typedef collections::Group<GlobalStringKey, Ptr<workflow::WfExpression>>	ArgumentMap;

			virtual GlobalStringKey					GetTypeName() = 0;

			virtual void							GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames);
			virtual void							GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames);
			virtual void							GetPairedProperties(const PropertyInfo& propertyInfo, collections::List<GlobalStringKey>& propertyNames);
			virtual Ptr<GuiInstancePropertyInfo>	GetPropertyType(const PropertyInfo& propertyInfo);

			virtual bool							CanCreate(const TypeInfo& typeInfo);
			virtual Ptr<workflow::WfStatement>		CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments);
			virtual Ptr<workflow::WfStatement>		AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments);
			virtual Ptr<workflow::WfExpression>		GetParameter(const PropertyInfo& propertyInfo, GlobalStringKey variableName);
		};

/***********************************************************************
Instance Binder
***********************************************************************/

		class IGuiInstanceBinder : public IDescriptable, public Description<IGuiInstanceBinder>
		{
		public:
			virtual GlobalStringKey					GetBindingName() = 0;
			virtual bool							ApplicableToConstructorArgument() = 0;
			virtual Ptr<workflow::WfStatement>		GenerateInstallStatement(GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, const WString& code, collections::List<WString>& errors) = 0;
		};

		class IGuiInstanceEventBinder : public IDescriptable, public Description<IGuiInstanceEventBinder>
		{
		public:
			virtual GlobalStringKey					GetBindingName() = 0;
			virtual Ptr<workflow::WfStatement>		GenerateInstallStatement(GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& code, collections::List<WString>& errors) = 0;
		};

/***********************************************************************
Instance Loader Manager
***********************************************************************/

		class IGuiInstanceLoaderManager : public IDescriptable, public Description<IGuiInstanceLoaderManager>
		{
		public:
			virtual bool								AddInstanceBinder(Ptr<IGuiInstanceBinder> binder) = 0;
			virtual IGuiInstanceBinder*					GetInstanceBinder(GlobalStringKey bindingName) = 0;
			virtual bool								AddInstanceEventBinder(Ptr<IGuiInstanceEventBinder> binder) = 0;
			virtual IGuiInstanceEventBinder*			GetInstanceEventBinder(GlobalStringKey bindingName) = 0;
			virtual bool								CreateVirtualType(GlobalStringKey parentType, Ptr<IGuiInstanceLoader> loader) = 0;
			virtual bool								SetLoader(Ptr<IGuiInstanceLoader> loader) = 0;
			virtual IGuiInstanceLoader*					GetLoader(GlobalStringKey typeName) = 0;
			virtual IGuiInstanceLoader*					GetParentLoader(IGuiInstanceLoader* loader) = 0;
			virtual description::ITypeDescriptor*		GetTypeDescriptorForType(GlobalStringKey typeName) = 0;
			virtual void								GetVirtualTypes(collections::List<GlobalStringKey>& typeNames) = 0;
			virtual GlobalStringKey						GetParentTypeForVirtualType(GlobalStringKey virtualType) = 0;
			virtual bool								SetResource(const WString& name, Ptr<GuiResource> resource) = 0;
			virtual Ptr<GuiResource>					GetResource(const WString& name) = 0;
		};

		struct InstanceLoadingSource
		{
			IGuiInstanceLoader*						loader;
			GlobalStringKey							typeName;
			Ptr<GuiResourceItem>					item;
			Ptr<GuiInstanceContext>					context;

			InstanceLoadingSource()
				:loader(0)
			{
			}

			InstanceLoadingSource(IGuiInstanceLoader* _loader, GlobalStringKey _typeName)
				:loader(_loader)
				, typeName(_typeName)
			{
			}

			InstanceLoadingSource(Ptr<GuiResourceItem> _item)
				:loader(0)
				, item(_item)
				, context(item->GetContent().Cast<GuiInstanceContext>())
			{
			}

			operator bool()const
			{
				return loader != 0 || context;
			}
		};

		extern IGuiInstanceLoaderManager*			GetInstanceLoaderManager();
		extern InstanceLoadingSource				FindInstanceLoadingSource(Ptr<GuiInstanceContext> context, GuiConstructorRepr* ctor);

/***********************************************************************
Instance Scope Wrapper
***********************************************************************/

		class IGuiInstancePartialClass
		{
		public:
			virtual Ptr<GuiInstanceContextScope> GetScope() = 0;
		};

		template<typename T>
		class GuiInstancePartialClass : public IGuiInstancePartialClass
		{
		private:
			GlobalStringKey							className;
			Ptr<GuiInstanceContextScope>			scope;

		protected:
			bool InitializeFromResource()
			{
				if (scope) return false;
				if (auto loader = GetInstanceLoaderManager()->GetLoader(className))
				{
					IGuiInstanceLoader::TypeInfo typeInfo(className, description::GetTypeDescriptor<T>());
					if (loader->IsInitializable(typeInfo))
					{
						auto value = description::Value::From(dynamic_cast<T*>(this));
						if ((scope = loader->InitializeInstance(typeInfo, value)))
						{
#ifdef _DEBUG
							CHECK_ERROR(scope->errors.Count() == 0, L"vl::presentation::GuiInstancePartialClass<T>::InitializeFromResource()#There is something wrong with the resource.");
#endif
							return true;
						}
					}
#ifdef _DEBUG
					else
					{
						CHECK_FAIL(L"vl::presentation::GuiInstancePartialClass<T>::InitializeFromResource()#Cannot initialize this instance from the resource.");
					}
#endif
				}
				return false;
			}

			template<typename TControl>
			void LoadInstanceReference(const WString& name, TControl*& reference)
			{
				reference = 0;
				vint index = scope->referenceValues.Keys().IndexOf(GlobalStringKey::Get(name));
				if (index == -1)
				{
					scope->errors.Add(L"Failed to find instance reference \"" + name + L"\".");
					return;
				}

				auto value = scope->referenceValues.Values()[index];
				auto td = description::GetTypeDescriptor<TControl>();
				if (!value.GetTypeDescriptor() || !value.GetTypeDescriptor()->CanConvertTo(td))
				{
					scope->errors.Add(L"Failed to convert instance reference \"" + name + L"\" to \"" + td->GetTypeName() + L"\".");
					return;
				}

				reference = description::UnboxValue<TControl*>(value);
			}
		public:
			GuiInstancePartialClass(const WString& _className)
				:className(GlobalStringKey::Get(_className))
			{
			}

			virtual ~GuiInstancePartialClass()
			{
			}

			Ptr<GuiInstanceContextScope> GetScope()
			{
				return scope;
			}
		};

#define GUI_INSTANCE_REFERENCE(NAME) LoadInstanceReference(L ## #NAME, this->NAME)
	}
}

#endif