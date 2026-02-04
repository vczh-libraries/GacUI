# Generated APIs

If you enable everything (by default) in**Parser.xml**, AST will generates following APIs for creating and accessing AST types:
- For any**ClassType**there is a**MakeClassType**class, call its default constructor and you can create the whole AST tree with this fluent interface. After all fields are prepared the created object can be obtained by static_cast\<Ptr\<ClassType\>\>.
- Each base class has a**ClassType::IVisitor**interface and a**Accept**function. The**Accept**function accepts an implementation of the**IVisitor**interface, and one member in**IVisitor**will be called according to the actual type of**Accept**'s "this argument".
- **your::namespaces::empty_visitor::ClassTypeVisitor**for each**ClassType::IVisitor**. It has all members implemented doing nothing. You can use this class to implement your own algorithm when only a few types need to be handled.
- **your::namespaces::copy_visitor::AstVisitor**.**Ast**here is the AST group's name. This visitor implements all**IVisitor**interfaces in one class. After passing it to**Accept**, its**result**field has a copy of the whole AST tree.
- **your::namespaces::json_visitor::AstVisitor**.**Ast**here is the AST group's name. This visitor implements all**IVisitor**interfaces in one class. After passing it to**Accept**, its**result**field has a serialized JSON document. The JSON document also follows the design of AST.
- **your::namespaces::traverse_visitor::AstVisitor**.**Ast**here is the AST group's name. This visitor implements all**IVisitor**interfaces in one class. After passing it to**Accept**, every object in the AST tree will calls a**Traverse**function from the top base type**ParsingAstBase**all the way to its actual type  in order, and**Finishing**when all members of an AST object are traversed, from its actual type all the way to the top base type**ParsingAstBase**. There is also a**Traverse**function for**ParsingToken**.

For any**RuleName**decorated with a**@parser**, a**ParseRuleName**function is generated in the**your::namespaces::Parser**class. Here**Parser**is the name of the parser defined in**Parser.xml**. You can either put a string or a List\<RegexToken\> to this function. It returns an AST tree when succeeded, otherwise it triggers the**OnError**event. In this event, you can choose to stop the parse function to throw an exception, and it will just returns null.

For the predefined XML parser, you are not recommended to call the**Parser**class directly, because some post processing is required to fix the AST tree. Instead pass this**Parser**class to**XmlParseDocument**or**XmlParseElement**.

