using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocSymbol
{
    class DeserializeTypeDeclVisitor : TypeDecl.IVisitor
    {
        public XElement Element;

        private static T LoadEnum<T>(string name)
        {
            return (T)typeof(T).GetField(name, BindingFlags.Public | BindingFlags.Static).GetValue(null);
        }

        public void Visit(RefTypeDecl decl)
        {
            decl.Name = Element.Attribute("Name").Value;
        }

        public void Visit(SubTypeDecl decl)
        {
            decl.Name = Element.Attribute("Name").Value;
            decl.Parent = TypeDecl.Deserialize(this.Element.Element("Parent").Elements().First());
        }

        public void Visit(DecorateTypeDecl decl)
        {
            decl.Decoration = LoadEnum<Decoration>(this.Element.Attribute("Decoration").Value);
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
        }

        public void Visit(ArrayTypeDecl decl)
        {
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
        }

        public void Visit(FunctionTypeDecl decl)
        {
            decl.CallingConvention = LoadEnum<CallingConvention>(this.Element.Attribute("CallingConvention").Value);
            decl.Const = bool.Parse(this.Element.Attribute("Const").Value);
            decl.ReturnType = TypeDecl.Deserialize(this.Element.Element("ReturnType").Elements().First());
            decl.Parameters = this.Element.Element("Parameters").Elements().Select(x => (VarDecl)SymbolDecl.Deserialize(x)).ToList();
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
            decl.ClassType = TypeDecl.Deserialize(this.Element.Element("ClassType").Elements().First());
        }

        public void Visit(GenericTypeDecl decl)
        {
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
            decl.TypeArguments = this.Element.Element("TypeArguments").Elements().Select(x => TypeDecl.Deserialize(x)).ToList();
        }

        public void Visit(DeclTypeDecl decl)
        {
            decl.Expression = this.Element.Attribute("Expression").Value;
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
        }

        public void Visit(ConstantTypeDecl decl)
        {
            decl.Value = this.Element.Attribute("Value").Value;
        }
    }

    class DeserializeSymbolDeclVisitor : SymbolDecl.IVisitor
    {
        public XElement Element;

        public void Visit(GlobalDesc decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(NamespaceDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(UsingNamespaceDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(TemplateDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(BaseTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(ClassDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(VarDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(FuncDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(GroupedFieldDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(EnumItemDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(EnumDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(TypedefDecl decl)
        {
            throw new NotImplementedException();
        }
    }
}
