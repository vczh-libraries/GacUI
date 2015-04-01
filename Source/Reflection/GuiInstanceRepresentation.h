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

			virtual void							Accept(IVisitor* visitor) = 0;
			virtual Ptr<GuiValueRepr>				Clone() = 0;
			virtual void							FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource) = 0;
			virtual void							CollectUsedKey(collections::List<GlobalStringKey>& keys) = 0;
			virtual void							SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey) = 0;
			static Ptr<GuiValueRepr>				LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys);
		};

		class GuiTextRepr : public GuiValueRepr, public Description<GuiTextRepr>
		{
		public:
			static const vint						BinaryKey = 1;

			WString									text;

			void									Accept(IVisitor* visitor)override{visitor->Visit(this);}
			Ptr<GuiValueRepr>						Clone()override;
			void									FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource)override;
			void									CollectUsedKey(collections::List<GlobalStringKey>& keys)override;
			void									SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey)override;
			static Ptr<GuiTextRepr>					LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys, Ptr<GuiTextRepr> repr = 0);
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
			};

			struct EventValue : public Object, public Description<EventValue>
			{
				GlobalStringKey						binding;
				WString								value;
				bool								fromStyle = false;
			};
			
			typedef collections::Dictionary<GlobalStringKey, Ptr<SetterValue>>			SetteValuerMap;
			typedef collections::Dictionary<GlobalStringKey, Ptr<EventValue>>			EventHandlerMap;
		public:
			SetteValuerMap							setters;					// empty key means default property
			EventHandlerMap							eventHandlers;
			GlobalStringKey							instanceName;

			void									Accept(IVisitor* visitor)override{visitor->Visit(this);}
			void									CloneBody(Ptr<GuiAttSetterRepr> repr);
			Ptr<GuiValueRepr>						Clone()override;
			void									FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource)override;
			void									CollectUsedKey(collections::List<GlobalStringKey>& keys)override;
			void									SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey)override;
			static Ptr<GuiAttSetterRepr>			LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys, Ptr<GuiAttSetterRepr> repr = 0);
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
			void									FillXml(Ptr<parsing::xml::XmlElement> xml, bool serializePrecompiledResource)override;
			void									CollectUsedKey(collections::List<GlobalStringKey>& keys)override;
			void									SavePrecompiledBinary(stream::IStream& stream, collections::SortedList<GlobalStringKey>& keys, bool saveKey)override;
			static Ptr<GuiConstructorRepr>			LoadPrecompiledBinary(stream::IStream& stream, collections::List<GlobalStringKey>& keys, Ptr<GuiConstructorRepr> repr = 0);
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
		// C++:			<instance>->Get<name>
		class GuiInstanceParameter : public Object, public Description<GuiInstanceParameter>
		{
		public:
			GlobalStringKey							name;
			GlobalStringKey							className;
		};

		// Workflow:	<instance>.<name>
		// C++:			<instance>->Get<name>
		//				<instance>->Set<name>
		class GuiInstanceProperty : public Object, public Description<GuiInstanceProperty>
		{
		public:
			GlobalStringKey							name;
			WString									typeName;
			bool									readonly = false;
		};
		
		// Workflow:	<instance>.<name>
		// C++:			<instance>-><name>
		class GuiInstanceState : public Object, public Description<GuiInstanceState>
		{
		public:
			GlobalStringKey							name;
			WString									typeName;
		};

/***********************************************************************
Instance Context
***********************************************************************/

		class GuiInstanceStyleContext;

		class GuiInstanceContext : public Object, public Description<GuiInstanceContext>
		{
		public:
			typedef collections::List<Ptr<GuiInstanceNamespace>>						NamespaceList;
			typedef collections::Dictionary<GlobalStringKey, Ptr<IGuiInstanceCache>>	CacheMap;

			struct NamespaceInfo : public Object, public Description<NamespaceInfo>
			{
				GlobalStringKey						name;
				NamespaceList						namespaces;
			};
			typedef collections::Dictionary<GlobalStringKey, Ptr<NamespaceInfo>>		NamespaceMap;
			typedef collections::List<Ptr<GuiInstanceParameter>>						ParameterList;
			typedef collections::List<Ptr<GuiInstanceProperty>>							PropertyList;
			typedef collections::List<Ptr<GuiInstanceState>>							StateList;
			typedef collections::List<Ptr<GuiInstanceStyleContext>>						StyleContextList;

			class ElementName : public Object
			{
			public:
				WString								namespaceName;				// empty key means default namespace
				WString								category;
				WString								name;
				WString								binding;

				bool IsCtorName(){ return category==L"" && name!=L"" && binding==L""; }
				bool IsReferenceAttributeName(){ return namespaceName==L"" && category==L"ref" && name!=L"" && binding==L""; }
				bool IsPropertyAttributeName(){ return namespaceName==L"" && category==L"" && name!=L""; }
				bool IsPropertyElementName(){ return namespaceName==L"" && category==L"att" && name!=L""; }
				bool IsEventAttributeName(){ return namespaceName==L"" && category==L"ev" && name!=L""; }
				bool IsEventElementName(){ return namespaceName==L"" && category==L"ev" && name!=L""; }
			};
		public:
			Ptr<GuiConstructorRepr>					instance;
			NamespaceMap							namespaces;
			bool									codeBehind = true;
			Nullable<WString>						className;
			ParameterList							parameters;
			PropertyList							properties;
			StateList								states;
			collections::List<WString>				stylePaths;

			bool									appliedStyles = false;
			StyleContextList						styles;
			CacheMap								precompiledCaches;

			static void								CollectDefaultAttributes(GuiAttSetterRepr::ValueList& values, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			static void								CollectAttributes(GuiAttSetterRepr::SetteValuerMap& setters, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			static void								CollectEvents(GuiAttSetterRepr::EventHandlerMap& eventHandlers, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			static void								FillAttSetter(Ptr<GuiAttSetterRepr> setter, Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			static Ptr<GuiConstructorRepr>			LoadCtor(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			static Ptr<GuiInstanceContext>			LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors);
			static Ptr<GuiInstanceContext>			LoadPrecompiledBinary(stream::IStream& stream, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlDocument>			SaveToXml(bool serializePrecompiledResource);
			void									SavePrecompiledBinary(stream::IStream& stream);
			void									CollectUsedKey(collections::List<GlobalStringKey>& keys);
			bool									ApplyStyles(Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors);
		};

/***********************************************************************
Instance Style Context
***********************************************************************/

		class GuiInstanceStyle : public Object, public Description<GuiInstanceStyle>
		{
		public:
			Ptr<GuiIqQuery>							query;
			Ptr<GuiAttSetterRepr>					setter;

			static Ptr<GuiInstanceStyle>			LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlElement>			SaveToXml();
		};

		class GuiInstanceStyleContext : public Object, public Description<GuiInstanceStyleContext>
		{
			typedef collections::List<Ptr<GuiInstanceStyle>>		StyleList;
		public:
			StyleList								styles;

			static Ptr<GuiInstanceStyleContext>		LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlDocument>			SaveToXml();
		};
	}
}

#endif