/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Loader

Interfaces:
***********************************************************************/

#ifdef VCZH_DEBUG_NO_REFLECTION
static_assert(false, "Don't use GacUICompiler.(h|cpp) if VCZH_DEBUG_NO_REFLECTION is defined.");
#endif

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOADER
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCELOADER

#include "../../Import/VlppWorkflowCompiler.h"
#include "../Controls/GuiApplication.h"
#include "../Controls/Templates/GuiThemeStyleFactory.h"
#include "GuiInstanceRepresentation.h"

namespace vl
{
	namespace presentation
	{
		namespace types
		{
			struct ResolvingResult;
		}

/***********************************************************************
Instance Loader
***********************************************************************/

		class GuiInstancePropertyInfo : public IDescriptable, public Description<GuiInstancePropertyInfo>
		{
			typedef collections::List<Ptr<description::ITypeInfo>>		TypeInfoList;
		public:
			enum Support
			{
				NotSupport,
				SupportAssign,
				SupportArray,
				SupportCollection,
				SupportSet,
			};

			enum PropertyUsage
			{
				ConstructorArgument,
				Property,
			};

			enum PropertyBindability
			{
				Bindable,
				NotBindable,
			};

			enum PropertyMergability
			{
				MergeWithParent,
				NotMerge,
			};

			Support									support = NotSupport;
			PropertyUsage							usage = Property;
			PropertyBindability						bindability = NotBindable;
			PropertyMergability						mergability = NotMerge;
			TypeInfoList							acceptableTypes;

			static Ptr<GuiInstancePropertyInfo>		Unsupported();
			static Ptr<GuiInstancePropertyInfo>		Assign(Ptr<description::ITypeInfo> typeInfo);
			static Ptr<GuiInstancePropertyInfo>		AssignWithParent(Ptr<description::ITypeInfo> typeInfo);
			static Ptr<GuiInstancePropertyInfo>		Collection(Ptr<description::ITypeInfo> typeInfo);
			static Ptr<GuiInstancePropertyInfo>		CollectionWithParent(Ptr<description::ITypeInfo> typeInfo);
			static Ptr<GuiInstancePropertyInfo>		Set(Ptr<description::ITypeInfo> typeInfo);
			static Ptr<GuiInstancePropertyInfo>		Array(Ptr<description::ITypeInfo> typeInfo);
		};

		class IGuiInstanceLoader : public IDescriptable, public Description<IGuiInstanceLoader>
		{
		public:
			struct TypeInfo
			{
				GlobalStringKey						typeName;
				Ptr<description::ITypeInfo>			typeInfo = nullptr;

				TypeInfo()
				{
				}

				TypeInfo(GlobalStringKey _typeName, Ptr<description::ITypeInfo> _typeInfo)
					:typeName(_typeName)
					, typeInfo(_typeInfo)
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

			struct ArgumentInfo
			{
				Ptr<workflow::WfExpression>			expression;
				Ptr<description::ITypeInfo>			typeInfo;
				GuiResourceTextPos					attPosition;
				GuiResourceTextPos					valuePosition;				// only apply to text value
			};

			typedef collections::Group<GlobalStringKey, ArgumentInfo>	ArgumentMap;

			virtual GlobalStringKey							GetTypeName() = 0;
			virtual void									ClearReflectionCache();

			virtual void									GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames);
			virtual void									GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames);
			virtual void									GetPairedProperties(const PropertyInfo& propertyInfo, collections::List<GlobalStringKey>& propertyNames);
			virtual Ptr<GuiInstancePropertyInfo>			GetPropertyType(const PropertyInfo& propertyInfo);

			virtual bool									CanCreate(const TypeInfo& typeInfo);
			virtual Ptr<workflow::WfBaseConstructorCall>	CreateRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors);
			virtual Ptr<workflow::WfStatement>				InitializeRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors);
			virtual Ptr<workflow::WfStatement>				CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors);
			virtual Ptr<workflow::WfStatement>				AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors);
			virtual Ptr<workflow::WfExpression>				GetParameter(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, GuiResourceTextPos attPosition, GuiResourceError::List& errors);
		};

/***********************************************************************
Instance Binder
***********************************************************************/

		class IGuiInstanceBinder : public IDescriptable, public Description<IGuiInstanceBinder>
		{
		public:
			virtual GlobalStringKey					GetBindingName() = 0;
			virtual bool							ApplicableToConstructorArgument() = 0;
			virtual bool							RequirePropertyExist() = 0;
			virtual Ptr<workflow::WfExpression>		GenerateConstructorArgument(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors) = 0;
			virtual Ptr<workflow::WfStatement>		GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IPropertyInfo* propertyInfo, IGuiInstanceLoader* loader, const IGuiInstanceLoader::PropertyInfo& prop, Ptr<GuiInstancePropertyInfo> propInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors) = 0;
		};

		class IGuiInstanceEventBinder : public IDescriptable, public Description<IGuiInstanceEventBinder>
		{
		public:
			virtual GlobalStringKey					GetBindingName() = 0;
			virtual Ptr<workflow::WfStatement>		GenerateInstallStatement(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, GlobalStringKey variableName, description::IEventInfo* eventInfo, const WString& code, GuiResourceTextPos position, GuiResourceError::List& errors) = 0;
		};

		class IGuiInstanceDeserializer : public IDescriptable, public Description<IGuiInstanceDeserializer>
		{
		public:
			virtual bool							CanDeserialize(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo) = 0;
			virtual description::ITypeInfo*			DeserializeAs(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo) = 0;
			virtual Ptr<workflow::WfExpression>		Deserialize(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo, Ptr<workflow::WfExpression> valueExpression, GuiResourceTextPos tagPosition, GuiResourceError::List& errors) = 0;
		};

/***********************************************************************
Instance Loader Manager
***********************************************************************/

		class IGuiInstanceLoaderManager : public IDescriptable, public Description<IGuiInstanceLoaderManager>
		{
		public:
			virtual bool							AddInstanceBinder(Ptr<IGuiInstanceBinder> binder) = 0;
			virtual IGuiInstanceBinder*				GetInstanceBinder(GlobalStringKey bindingName) = 0;
			virtual bool							AddInstanceEventBinder(Ptr<IGuiInstanceEventBinder> binder) = 0;
			virtual IGuiInstanceEventBinder*		GetInstanceEventBinder(GlobalStringKey bindingName) = 0;
			virtual bool							AddInstanceDeserializer(Ptr<IGuiInstanceDeserializer> deserializer) = 0;
			virtual IGuiInstanceDeserializer*		GetInstanceDeserializer(const IGuiInstanceLoader::PropertyInfo& propertyInfo, description::ITypeInfo* typeInfo) = 0;

			virtual bool							CreateVirtualType(GlobalStringKey parentType, Ptr<IGuiInstanceLoader> loader) = 0;
			virtual bool							SetLoader(Ptr<IGuiInstanceLoader> loader) = 0;
			virtual IGuiInstanceLoader*				GetLoader(GlobalStringKey typeName) = 0;
			virtual IGuiInstanceLoader*				GetParentLoader(IGuiInstanceLoader* loader) = 0;
			virtual Ptr<description::ITypeInfo>		GetTypeInfoForType(GlobalStringKey typeName) = 0;
			virtual void							GetVirtualTypes(collections::List<GlobalStringKey>& typeNames) = 0;
			virtual GlobalStringKey					GetParentTypeForVirtualType(GlobalStringKey virtualType) = 0;
			virtual void							ClearReflectionCache() = 0;
		};

		extern IGuiInstanceLoaderManager*			GetInstanceLoaderManager();

/***********************************************************************
Helper Functions
***********************************************************************/

		extern void									SplitTypeName(const WString& input, collections::List<WString>& fragments);
		extern void									SplitBySemicolon(const WString& input, collections::List<WString>& fragments);
	}
}

#endif