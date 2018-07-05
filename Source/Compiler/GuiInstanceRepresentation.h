/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Representation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCEREPRESENTATION
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCEREPRESENTATION

#include "../Resources/GuiResource.h"
#include "InstanceQuery/GuiInstanceQuery_Parser.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
Instance Representation
***********************************************************************/

		class GuiTextRepr;
		class GuiAttSetterRepr;
		class GuiConstructorRepr;
		class IGuiInstanceCache;

		class GuiValueRepr : public Object, public Description<GuiValueRepr>
		{
		public:
			class IVisitor : public IDescriptable, public Description<IVisitor>
			{
			public:
				virtual void						Visit(GuiTextRepr* repr)=0;
				virtual void						Visit(GuiAttSetterRepr* repr)=0;
				virtual void						Visit(GuiConstructorRepr* repr)=0;
			};

			bool									fromStyle = false;
			GuiResourceTextPos						tagPosition;

			void									CloneBody(Ptr<GuiValueRepr> repr);

			virtual void							Accept(IVisitor* visitor) = 0;
			virtual Ptr<GuiValueRepr>				Clone() = 0;
			virtual void							FillXml(Ptr<parsing::xml::XmlElement> xml) = 0;
		};

		class GuiTextRepr : public GuiValueRepr, public Description<GuiTextRepr>
		{
		public:
			static const vint						BinaryKey = 1;

			WString									text;

			void									Accept(IVisitor* visitor)override{visitor->Visit(this);}
			Ptr<GuiValueRepr>						Clone()override;
			void									FillXml(Ptr<parsing::xml::XmlElement> xml)override;
		};

		class GuiAttSetterRepr : public GuiValueRepr, public Description<GuiAttSetterRepr>
		{
		public:
			typedef collections::List<Ptr<GuiValueRepr>>						ValueList;
			static const vint						BinaryKey = 2;

			struct SetterValue : public Object, public Description<SetterValue>
			{
				GlobalStringKey						binding;
				ValueList							values;
				GuiResourceTextPos					attPosition;
			};

			struct EventValue : public Object, public Description<EventValue>
			{
				GlobalStringKey						binding;
				WString								value;
				bool								fromStyle = false;
				GuiResourceTextPos					attPosition;
				GuiResourceTextPos					valuePosition;
			};

			struct EnvVarValue : public Object, public Description<EnvVarValue>
			{
				WString								value;
				bool								fromStyle = false;
				GuiResourceTextPos					attPosition;
				GuiResourceTextPos					valuePosition;
			};
			
			typedef collections::Dictionary<GlobalStringKey, Ptr<SetterValue>>			SetteValuerMap;
			typedef collections::Dictionary<GlobalStringKey, Ptr<EventValue>>			EventHandlerMap;
			typedef collections::Dictionary<GlobalStringKey, Ptr<EnvVarValue>>			EnvironmentVariableMap;
		public:
			SetteValuerMap							setters;					// empty key means default property
			EventHandlerMap							eventHandlers;
			EnvironmentVariableMap					environmentVariables;
			GlobalStringKey							instanceName;

			void									Accept(IVisitor* visitor)override{visitor->Visit(this);}
			void									CloneBody(Ptr<GuiAttSetterRepr> repr);
			Ptr<GuiValueRepr>						Clone()override;
			void									FillXml(Ptr<parsing::xml::XmlElement> xml)override;
		};

		class GuiConstructorRepr : public GuiAttSetterRepr, public Description<GuiConstructorRepr>
		{
		public:
			static const vint						BinaryKey = 3;

			GlobalStringKey							typeNamespace;
			GlobalStringKey							typeName;
			Nullable<WString>						styleName;

			void									Accept(IVisitor* visitor)override{visitor->Visit(this);}
			Ptr<GuiValueRepr>						Clone()override;
			void									FillXml(Ptr<parsing::xml::XmlElement> xml)override;
		};

/***********************************************************************
Instance Namespace
***********************************************************************/

		class GuiInstanceNamespace : public Object, public Description<GuiInstanceNamespace>
		{
		public:
			WString									prefix;
			WString									postfix;
		};

		// Workflow:	<name>
		// C++:			<instance>->Get<name>()
		class GuiInstanceParameter : public Object, public Description<GuiInstanceParameter>
		{
		public:
			GlobalStringKey							name;
			GlobalStringKey							className;
			GuiResourceTextPos						tagPosition;
			GuiResourceTextPos						classPosition;
		};

		class GuiInstanceLocalized : public Object, public Description<GuiInstanceLocalized>
		{
		public:
			GlobalStringKey							name;
			GlobalStringKey							className;
			GuiResourceTextPos						tagPosition;
			GuiResourceTextPos						classPosition;
			bool									defaultStrings = false;
		};

/***********************************************************************
Instance Context
***********************************************************************/

		class GuiInstanceStyleContext;

		class GuiInstanceContext : public Object, public Description<GuiInstanceContext>
		{
		public:
			typedef collections::List<Ptr<GuiInstanceNamespace>>						NamespaceList;

			struct NamespaceInfo : public Object, public Description<NamespaceInfo>
			{
				GlobalStringKey						name;
				NamespaceList						namespaces;
				GuiResourceTextPos					attPosition;
			};
			typedef collections::Dictionary<GlobalStringKey, Ptr<NamespaceInfo>>		NamespaceMap;
			typedef collections::List<Ptr<GuiInstanceParameter>>						ParameterList;
			typedef collections::List<Ptr<GuiInstanceLocalized>>						LocalizedList;
			typedef collections::List<Ptr<GuiInstanceStyleContext>>						StyleContextList;

			class ElementName : public Object
			{
			public:
				WString								namespaceName;				// empty key means default namespace
				WString								category;
				WString								name;
				WString								binding;

				bool IsCtorName() { return category == L"" && name != L"" && binding == L""; }
				bool IsReferenceAttributeName() { return namespaceName == L"" && category == L"ref" && name != L"" && binding == L""; }
				bool IsEnvironmentAttributeName() { return namespaceName == L"" && category == L"env" && name != L""; }
				bool IsPropertyAttributeName() { return namespaceName == L"" && category == L"" && name != L""; }
				bool IsPropertyElementName() { return namespaceName == L"" && category == L"att" && name != L""; }
				bool IsEventAttributeName() { return namespaceName == L"" && category == L"ev" && name != L""; }
				bool IsEventElementName() { return namespaceName == L"" && category == L"ev" && name != L""; }
			};
		public:
			Ptr<GuiConstructorRepr>					instance;
			NamespaceMap							namespaces;
			bool									codeBehind = true;
			WString									className;
			collections::List<WString>				stylePaths;

			ParameterList							parameters;
			LocalizedList							localizeds;
			WString									memberScript;
			WString									ctorScript;
			WString									dtorScript;

			GuiResourceTextPos						tagPosition;
			GuiResourceTextPos						classPosition;
			GuiResourceTextPos						stylePosition;
			GuiResourceTextPos						memberPosition;
			GuiResourceTextPos						ctorPosition;
			GuiResourceTextPos						dtorPosition;

			bool									appliedStyles = false;
			StyleContextList						styles;

			static void								CollectDefaultAttributes(Ptr<GuiResourceItem> resource, GuiAttSetterRepr::ValueList& values, Ptr<parsing::xml::XmlElement> xml, GuiResourceError::List& errors);
			static void								CollectAttributes(Ptr<GuiResourceItem> resource, GuiAttSetterRepr::SetteValuerMap& setters, Ptr<parsing::xml::XmlElement> xml, GuiResourceError::List& errors);
			static void								CollectEvents(Ptr<GuiResourceItem> resource, GuiAttSetterRepr::EventHandlerMap& eventHandlers, Ptr<parsing::xml::XmlElement> xml, GuiResourceError::List& errors);
			static void								FillAttSetter(Ptr<GuiResourceItem> resource, Ptr<GuiAttSetterRepr> setter, Ptr<parsing::xml::XmlElement> xml, GuiResourceError::List& errors);
			static Ptr<GuiConstructorRepr>			LoadCtor(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlElement> xml, GuiResourceError::List& errors);
			static Ptr<GuiInstanceContext>			LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlDocument>			SaveToXml();
			bool									ApplyStyles(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors);
		};

/***********************************************************************
Instance Style Context
***********************************************************************/

		class GuiInstanceStyle : public Object, public Description<GuiInstanceStyle>
		{
		public:
			Ptr<GuiIqQuery>							query;
			Ptr<GuiAttSetterRepr>					setter;

			static Ptr<GuiInstanceStyle>			LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlElement> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlElement>			SaveToXml();
		};

		class GuiInstanceStyleContext : public Object, public Description<GuiInstanceStyleContext>
		{
			typedef collections::List<Ptr<GuiInstanceStyle>>		StyleList;
		public:
			StyleList								styles;

			static Ptr<GuiInstanceStyleContext>		LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors);
			Ptr<parsing::xml::XmlDocument>			SaveToXml();
		};
	}
}

#endif