using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocSymbol
{
    class ResolveTypeDeclVisitor : TypeDecl.IVisitor
    {
        public SymbolDecl Symbol { get; set; }
        public List<string> Errors { get; set; }
        public string Result { get; set; }

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

    class ResolveSymbolDeclVisitor : SymbolDecl.IVisitor
    {
        public List<string> Errors { get; set; }

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TemplateDecl decl)
        {
            if (decl.Specialization != null)
            {
                foreach (var type in decl.Specialization)
                {
                    type.Resolve(decl, this.Errors);
                }
            }
        }

        public void Visit(BaseTypeDecl decl)
        {
        }

        public void Visit(ClassDecl decl)
        {
            var templateDecl = decl.Parent as TemplateDecl;
            if (templateDecl != null)
            {
                Visit(templateDecl);
            }

            foreach (var baseType in decl.BaseTypes)
            {
                baseType.Type.Resolve(decl, this.Errors);
            }
        }

        public void Visit(VarDecl decl)
        {
            decl.Type.Resolve(decl, this.Errors);
        }

        public void Visit(FuncDecl decl)
        {
            var templateDecl = decl.Parent as TemplateDecl;
            if (templateDecl != null)
            {
                Visit(templateDecl);
            }

            decl.Type.Resolve(decl, this.Errors);
        }

        public void Visit(GroupedFieldDecl decl)
        {
        }

        public void Visit(EnumItemDecl decl)
        {
        }

        public void Visit(EnumDecl decl)
        {
        }

        public void Visit(TypedefDecl decl)
        {
            var templateDecl = decl.Parent as TemplateDecl;
            if (templateDecl != null)
            {
                Visit(templateDecl);
            }

            decl.Type.Resolve(decl, this.Errors);
        }
    }
}
