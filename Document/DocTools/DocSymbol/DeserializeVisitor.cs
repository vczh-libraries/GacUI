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

        private void Deserialize(TypeDecl decl)
        {
            if (this.Element.Attribute("ReferencingNameKey") != null)
            {
                decl.ReferencingNameKey = this.Element.Attribute("ReferencingNameKey").Value;
            }
        }

        private static T LoadEnum<T>(string name)
        {
            return (T)typeof(T).GetField(name, BindingFlags.Public | BindingFlags.Static).GetValue(null);
        }

        public void Visit(RefTypeDecl decl)
        {
            Deserialize(decl);
            decl.Name = Element.Attribute("Name").Value;
        }

        public void Visit(SubTypeDecl decl)
        {
            Deserialize(decl);
            decl.Name = Element.Attribute("Name").Value;
            decl.Parent = TypeDecl.Deserialize(this.Element.Element("Parent").Elements().First());
        }

        public void Visit(DecorateTypeDecl decl)
        {
            Deserialize(decl);
            decl.Decoration = LoadEnum<Decoration>(this.Element.Attribute("Decoration").Value);
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
        }

        public void Visit(ArrayTypeDecl decl)
        {
            Deserialize(decl);
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
            decl.Expression = this.Element.Attribute("Expression").Value;
        }

        public void Visit(FunctionTypeDecl decl)
        {
            Deserialize(decl);
            decl.CallingConvention = LoadEnum<CallingConvention>(this.Element.Attribute("CallingConvention").Value);
            decl.Const = bool.Parse(this.Element.Attribute("Const").Value);
            decl.ReturnType = TypeDecl.Deserialize(this.Element.Element("ReturnType").Elements().First());
            decl.Parameters = this.Element.Element("Parameters").Elements().Select(x => (VarDecl)SymbolDecl.Deserialize(x)).ToList();
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            Deserialize(decl);
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
            decl.ClassType = TypeDecl.Deserialize(this.Element.Element("ClassType").Elements().First());
        }

        public void Visit(GenericTypeDecl decl)
        {
            Deserialize(decl);
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
            decl.TypeArguments = this.Element.Element("TypeArguments").Elements().Select(x => TypeDecl.Deserialize(x)).ToList();
        }

        public void Visit(DeclTypeDecl decl)
        {
            Deserialize(decl);
            decl.Expression = this.Element.Attribute("Expression").Value;
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            Deserialize(decl);
            decl.Element = TypeDecl.Deserialize(this.Element.Element("Element").Elements().First());
        }

        public void Visit(ConstantTypeDecl decl)
        {
            Deserialize(decl);
            decl.Value = this.Element.Attribute("Value").Value;
        }
    }

    class DeserializeSymbolDeclVisitor : SymbolDecl.IVisitor
    {
        public XElement Element;

        private static T LoadEnum<T>(string name)
        {
            return (T)typeof(T).GetField(name, BindingFlags.Public | BindingFlags.Static).GetValue(null);
        }

        private void Deserialize(SymbolDecl decl)
        {
            decl.Access = LoadEnum<Access>(this.Element.Attribute("Access").Value);
            if (this.Element.Attribute("Name") != null)
            {
                decl.Name = this.Element.Attribute("Name").Value;
            }
            if (this.Element.Attribute("Document") != null)
            {
                decl.Document = this.Element.Attribute("Document").Value;
            }
            if (this.Element.Attribute("Tags") != null)
            {
                decl.Tags = this.Element.Attribute("Tags").Value;
            }
            if (this.Element.Attribute("NameKey") != null)
            {
                decl.NameKey = this.Element.Attribute("NameKey").Value;
            }
            if (this.Element.Attribute("OverloadKey") != null)
            {
                decl.OverloadKey = this.Element.Attribute("OverloadKey").Value;
            }
            if (this.Element.Element("Children") != null)
            {
                decl.Children = this.Element.Element("Children").Elements().Select(x => SymbolDecl.Deserialize(x)).ToList();
            }
        }

        public void Visit(GlobalDecl decl)
        {
            Deserialize(decl);
        }

        public void Visit(NamespaceDecl decl)
        {
            Deserialize(decl);
        }

        public void Visit(UsingNamespaceDecl decl)
        {
            Deserialize(decl);
            decl.Path = this.Element.Element("Path").Elements("Item").Select(x => x.Attribute("Value").Value).ToList();
        }

        public void Visit(TypeParameterDecl decl)
        {
            Deserialize(decl);
        }

        public void Visit(TemplateDecl decl)
        {
            Deserialize(decl);
            decl.TypeParameters = this.Element.Element("TypeParameters").Elements().Select(x => (TypeParameterDecl)SymbolDecl.Deserialize(x)).ToList();
            decl.Specialization = this.Element.Element("Specialization").Elements().Select(x => TypeDecl.Deserialize(x)).ToList();
            decl.Element = SymbolDecl.Deserialize(this.Element.Element("Element").Elements().First());
        }

        public void Visit(BaseTypeDecl decl)
        {
            Deserialize(decl);
            decl.Type = TypeDecl.Deserialize(this.Element.Element("Type").Elements().First());
        }

        public void Visit(ClassDecl decl)
        {
            Deserialize(decl);
            decl.ClassType = LoadEnum<ClassType>(this.Element.Attribute("ClassType").Value);
            decl.BaseTypes = this.Element.Element("BaseTypes").Elements().Select(x => (BaseTypeDecl)SymbolDecl.Deserialize(x)).ToList();
        }

        public void Visit(VarDecl decl)
        {
            Deserialize(decl);
            decl.Static = bool.Parse(this.Element.Attribute("Static").Value);
            decl.Type = TypeDecl.Deserialize(this.Element.Element("Type").Elements().First());
        }

        public void Visit(FuncDecl decl)
        {
            Deserialize(decl);
            decl.Virtual = LoadEnum<Virtual>(this.Element.Attribute("Virtual").Value);
            decl.Function = LoadEnum<Function>(this.Element.Attribute("Function").Value);
            decl.Type = TypeDecl.Deserialize(this.Element.Element("Type").Elements().First());
        }

        public void Visit(GroupedFieldDecl decl)
        {
            Deserialize(decl);
            decl.Grouping = LoadEnum<Grouping>(this.Element.Attribute("Grouping").Value);
        }

        public void Visit(EnumItemDecl decl)
        {
            Deserialize(decl);
        }

        public void Visit(EnumDecl decl)
        {
            Deserialize(decl);
        }

        public void Visit(TypedefDecl decl)
        {
            Deserialize(decl);
            decl.Type = TypeDecl.Deserialize(this.Element.Element("Type").Elements().First());
        }
    }
}
