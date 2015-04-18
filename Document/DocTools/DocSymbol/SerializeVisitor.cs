using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocSymbol
{
    class SerializeTypeDeclVisitor : TypeDecl.IVisitor
    {
        public XElement Element;

        private void Serialize(TypeDecl decl)
        {
            if (decl.ReferencingNameKey != null)
            {
                this.Element.Add(new XAttribute("ReferencingNameKey", decl.ReferencingNameKey));
            }
        }

        public void Visit(RefTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Name", decl.Name));
        }

        public void Visit(SubTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Name", decl.Name));
            this.Element.Add(new XElement("Parent", decl.Parent.Serialize()));
        }

        public void Visit(DecorateTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Decoration", decl.Decoration.ToString()));
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(ArrayTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(FunctionTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("CallingConvention", decl.CallingConvention.ToString()));
            this.Element.Add(new XAttribute("Const", decl.Const.ToString()));
            this.Element.Add(new XElement("ReturnType", decl.ReturnType.Serialize()));
            this.Element.Add(new XElement("Parameters", decl.Parameters.Select(x => x.Serialize())));
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
            this.Element.Add(new XElement("ClassType", decl.ClassType.Serialize()));
        }

        public void Visit(GenericTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
            this.Element.Add(new XElement("TypeArguments", decl.TypeArguments.Select(x => x.Serialize())));
        }

        public void Visit(DeclTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Expression", decl.Expression));
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(ConstantTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Value", decl.Value));
        }
    }

    class SerializeSymbolDeclVisitor : SymbolDecl.IVisitor
    {
        public XElement Element;

        private void Serialize(SymbolDecl decl, bool serializeChildren = true)
        {
            this.Element.Add(new XAttribute("Access", decl.Access.ToString()));
            if (decl.Name != null)
            {
                this.Element.Add(new XAttribute("Name", decl.Name));
            }
            if (decl.Document != null)
            {
                this.Element.Add(new XAttribute("Document", decl.Document));
            }
            if (decl.Tags != null)
            {
                this.Element.Add(new XAttribute("Tags", decl.Tags));
            }
            if (decl.NameKey != null)
            {
                this.Element.Add(new XAttribute("NameKey", decl.NameKey));
            }
            if (decl.OverloadKey != null)
            {
                this.Element.Add(new XAttribute("OverloadKey", decl.OverloadKey));
            }
            if (serializeChildren && decl.Children != null)
            {
                this.Element.Add(new XElement("Children", decl.Children.Select(x => x.Serialize())));
            }
        }

        public void Visit(GlobalDecl decl)
        {
            Serialize(decl);
        }

        public void Visit(NamespaceDecl decl)
        {
            Serialize(decl);
        }

        public void Visit(UsingNamespaceDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Path", decl.Path.Select(x => new XElement("Item", new XAttribute("Value", x)))));
        }

        public void Visit(TypeParameterDecl decl)
        {
            Serialize(decl, true);
        }

        public void Visit(TemplateDecl decl)
        {
            Serialize(decl, false);
            this.Element.Add(new XElement("TypeParameters", decl.TypeParameters.Select(x => x.Serialize())));
            this.Element.Add(new XElement("Specialization", decl.Specialization.Select(x => x.Serialize())));
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(BaseTypeDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Type", decl.Type.Serialize()));
        }

        public void Visit(ClassDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("ClassType", decl.ClassType.ToString()));
            this.Element.Add(new XElement("BaseTypes", decl.BaseTypes.Select(x => x.Serialize())));
        }

        public void Visit(VarDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Static", decl.Static.ToString()));
            this.Element.Add(new XElement("Type", decl.Type.Serialize()));
        }

        public void Visit(FuncDecl decl)
        {
            Serialize(decl, false);
            this.Element.Add(new XAttribute("Virtual", decl.Virtual.ToString()));
            this.Element.Add(new XAttribute("Function", decl.Function.ToString()));
            this.Element.Add(new XElement("Type", decl.Type.Serialize()));
        }

        public void Visit(GroupedFieldDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XAttribute("Grouping", decl.Grouping.ToString()));
        }

        public void Visit(EnumItemDecl decl)
        {
            Serialize(decl);
        }

        public void Visit(EnumDecl decl)
        {
            Serialize(decl);
        }

        public void Visit(TypedefDecl decl)
        {
            Serialize(decl);
            this.Element.Add(new XElement("Type", decl.Type.Serialize()));
        }
    }
}
