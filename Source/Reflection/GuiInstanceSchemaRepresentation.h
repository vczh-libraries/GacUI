/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Instance Schema Representation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHEMAREPRESENTATION
#define VCZH_PRESENTATION_REFLECTION_GUIINSTANCESCHEMAREPRESENTATION

#include "../Resources/GuiResource.h"

namespace vl
{
	namespace presentation
	{
		class GuiInstancePropertySchame :public Object, public Description<GuiInstancePropertySchame>
		{
		public:
			WString										name;
			WString										typeName;
			bool										readonly = false;
			bool										observable = false;

			static Ptr<GuiInstancePropertySchame>		LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();
		};

		class GuiInstanceTypeSchema : public Object, public Description<GuiInstanceTypeSchema>
		{
			typedef collections::List<Ptr<GuiInstancePropertySchame>>	PropertyList;
		public:
			WString										typeName;
			WString										parentType;
			PropertyList								properties;

			void										LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			virtual Ptr<parsing::xml::XmlElement>		SaveToXml() = 0;
		};

/***********************************************************************
Instance Struct/Class Schema
***********************************************************************/

		class GuiInstanceDataSchema : public GuiInstanceTypeSchema, public Description<GuiInstanceDataSchema>
		{
		public:
			bool										referenceType = false;

			static Ptr<GuiInstanceDataSchema>			LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml()override;
		};

/***********************************************************************
Instance Interface Schema
***********************************************************************/

		class GuiInstanceMethodSchema : public Object, public Description<GuiInstanceMethodSchema>
		{
			typedef collections::List<Ptr<GuiInstancePropertySchame>>	PropertyList;
		public:
			WString										name;
			WString										returnType;
			PropertyList								arguments;

			static Ptr<GuiInstanceMethodSchema>			LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml();
		};

		class GuiInstanceInterfaceSchema : public GuiInstanceTypeSchema, public Description<GuiInstanceInterfaceSchema>
		{
			typedef collections::List<Ptr<GuiInstanceMethodSchema>>		MethodList;
		public:
			MethodList									methods;

			static Ptr<GuiInstanceInterfaceSchema>		LoadFromXml(Ptr<parsing::xml::XmlElement> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlElement>				SaveToXml()override;
		};

/***********************************************************************
Instance Schema Representation
***********************************************************************/

		class GuiInstanceSchema : public Object, public Description<GuiInstanceSchema>
		{
			typedef collections::List<Ptr<GuiInstanceTypeSchema>>		TypeSchemaList;
		public:
			TypeSchemaList								schemas;

			static Ptr<GuiInstanceSchema>				LoadFromXml(Ptr<parsing::xml::XmlDocument> xml, collections::List<WString>& errors);
			Ptr<parsing::xml::XmlDocument>				SaveToXml();
		};
	}
}

#endif