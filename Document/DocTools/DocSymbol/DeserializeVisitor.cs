using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocSymbol
{
    class DeserializeTypeDeclVisitor : TypeDecl.IVisitor
    {
        public XElement Element;

        public void Visit(RefTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(SubTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(DecorateTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(ArrayTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(FunctionTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(GenericTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(DeclTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            throw new NotImplementedException();
        }

        public void Visit(ConstantTypeDecl decl)
        {
            throw new NotImplementedException();
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
