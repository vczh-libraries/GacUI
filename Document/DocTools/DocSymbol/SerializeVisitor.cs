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

        public void Visit(RefTypeDecl decl)
        {
            this.Element.Add(new XAttribute("Name", decl.Name));
        }

        public void Visit(SubTypeDecl decl)
        {
            this.Element.Add(new XAttribute("Name", decl.Name));
            this.Element.Add(new XElement("Parent", decl.Parent.Serialize()));
        }

        public void Visit(DecorateTypeDecl decl)
        {
            this.Element.Add(new XAttribute("Decoration", decl.Decoration.ToString()));
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(ArrayTypeDecl decl)
        {
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(FunctionTypeDecl decl)
        {
            this.Element.Add(new XAttribute("CallingConvention", decl.CallingConvention.ToString()));
            this.Element.Add(new XAttribute("Const", decl.Const.ToString()));
            this.Element.Add(new XElement("ReturnType", decl.ReturnType.Serialize()));
            this.Element.Add(new XElement("Parameters", decl.Parameters.Select(x => x.Serialize())));
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
            this.Element.Add(new XElement("ClassType", decl.ClassType.Serialize()));
        }

        public void Visit(GenericTypeDecl decl)
        {
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
            this.Element.Add(new XElement("TypeArguments", decl.TypeArguments.Select(x => x.Serialize())));
        }

        public void Visit(DeclTypeDecl decl)
        {
            this.Element.Add(new XAttribute("Expression", decl.Expression));
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            this.Element.Add(new XElement("Element", decl.Element.Serialize()));
        }

        public void Visit(ConstantTypeDecl decl)
        {
            this.Element.Add(new XAttribute("Value", decl.Value));
        }
    }

    class SerializeSymbolDeclVisitor : SymbolDecl.IVisitor
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
