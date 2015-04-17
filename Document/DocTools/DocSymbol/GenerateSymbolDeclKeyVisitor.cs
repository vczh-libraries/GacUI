using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocSymbol
{
    class GenerateSymbolDeclNameKeyVisitor : SymbolDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(GlobalDesc decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
            this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TemplateDecl decl)
        {
        }

        public void Visit(BaseTypeDecl decl)
        {
        }

        public void Visit(ClassDecl decl)
        {
            var template = decl.Parent as TemplateDecl;
            if (template == null)
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
            }
            else
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString();
            }
        }

        public void Visit(VarDecl decl)
        {
            this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(FuncDecl decl)
        {
            var template = decl.Parent as TemplateDecl;
            if (template == null)
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
            }
            else
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString();
            }
        }

        public void Visit(GroupedFieldDecl decl)
        {
        }

        public void Visit(EnumItemDecl decl)
        {
            this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(EnumDecl decl)
        {
            this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(TypedefDecl decl)
        {
            this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
        }
    }

    class GenerateSymbolDeclOverrideKeyVisitor : SymbolDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(GlobalDesc decl)
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
        }

        public void Visit(BaseTypeDecl decl)
        {
        }

        public void Visit(ClassDecl decl)
        {
            var template = decl.Parent as TemplateDecl;
            if (template == null)
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name;
            }
            else
            {
                var postfix = "<" + template.Specialization.Aggregate("", (a, b) => a == "" ? b.ToString() : a + "," + b.ToString()) + ">";
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString() + (postfix == "<>" ? "" : postfix);
            }
        }

        public void Visit(VarDecl decl)
        {
        }

        public void Visit(FuncDecl decl)
        {
            var func = (FunctionTypeDecl)decl.Type;
            var postfix = "(" + func.Parameters.Aggregate("", (a, b) => a == "" ? b.Type.ToString() : a + "," + b.Type.ToString()) + ")" + (func.Const ? "const" : "");
            var template = decl.Parent as TemplateDecl;
            if (template == null)
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name + postfix;
            }
            else
            {
                this.Result = decl.KeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString() + postfix;
            }
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
        }
    }
}
