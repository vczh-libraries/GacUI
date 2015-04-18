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

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TypeParameterDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(TemplateDecl decl)
        {
        }

        public void Visit(BaseTypeDecl decl)
        {
        }

        public void Visit(ClassDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(VarDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(FuncDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(GroupedFieldDecl decl)
        {
        }

        public void Visit(EnumItemDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(EnumDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(TypedefDecl decl)
        {
            this.Result = decl.NameKeyOfScopeParent + "::" + decl.Name;
        }
    }

    class GenerateSymbolDeclOverrideKeyVisitor : SymbolDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TypeParameterDecl decl)
        {
            this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name;
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
                this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name;
            }
            else
            {
                var postfix = "<" + template.Specialization.Aggregate("", (a, b) => a == "" ? b.ToString() : a + "," + b.ToString()) + ">";
                this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString() + (postfix == "<>" ? "" : postfix);
            }
        }

        public void Visit(VarDecl decl)
        {
            this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(FuncDecl decl)
        {
            var func = (FunctionTypeDecl)decl.Type;
            var postfix = "(" + func.Parameters.Aggregate("", (a, b) => a == "" ? b.Type.ToString() : a + "," + b.Type.ToString()) + ")" + (func.Const ? "const" : "");
            var template = decl.Parent as TemplateDecl;
            if (template == null)
            {
                this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name + postfix;
            }
            else
            {
                this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString() + postfix;
            }
        }

        public void Visit(GroupedFieldDecl decl)
        {
        }

        public void Visit(EnumItemDecl decl)
        {
            this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(EnumDecl decl)
        {
            this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name;
        }

        public void Visit(TypedefDecl decl)
        {
            var template = decl.Parent as TemplateDecl;
            if (template == null)
            {
                this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name;
            }
            else
            {
                var postfix = "<" + template.Specialization.Aggregate("", (a, b) => a == "" ? b.ToString() : a + "," + b.ToString()) + ">";
                this.Result = decl.OverloadKeyOfScopeParent + "::" + decl.Name + "`" + template.TypeParameters.Count.ToString() + (postfix == "<>" ? "" : postfix);
            }
        }
    }

    class GenerateSymbolDeclContentKeyVisitor : SymbolDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TypeParameterDecl decl)
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
            string header = decl.Parent is TemplateDecl
                ? decl.Parent.ToString()
                : decl.ToString();
            this.Result = header + (decl.Children == null ? "" : decl.Children.Aggregate("", (a, b) => a + "\r\n" + b.GenerateChildContentKey()));
        }

        public void Visit(VarDecl decl)
        {
            this.Result = decl.ToString();
        }

        public void Visit(FuncDecl decl)
        {
            string header = decl.Parent is TemplateDecl
                ? decl.Parent.ToString()
                : decl.ToString();
            this.Result = header + (decl.Children == null ? "" : decl.Children.Aggregate("", (a, b) => a + "\r\n" + b.GenerateChildContentKey()));
        }

        public void Visit(GroupedFieldDecl decl)
        {
        }

        public void Visit(EnumItemDecl decl)
        {
        }

        public void Visit(EnumDecl decl)
        {
            this.Result = decl.ToString() + decl.Children.Aggregate("", (a, b) => a + "\r\n" + b.GenerateChildContentKey());
        }

        public void Visit(TypedefDecl decl)
        {
            this.Result = decl.Parent is TemplateDecl
                ? decl.Parent.ToString()
                : decl.ToString();
        }
    }

    class GenerateSymbolDeclChildContentKeyVisitor : SymbolDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TypeParameterDecl decl)
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
            this.Result = decl.Parent is TemplateDecl
                ? decl.Parent.ToString()
                : decl.ToString();
        }

        public void Visit(VarDecl decl)
        {
            this.Result = decl.ToString();
        }

        public void Visit(FuncDecl decl)
        {
            this.Result = decl.Parent is TemplateDecl
                ? decl.Parent.ToString()
                : decl.ToString();
        }

        public void Visit(GroupedFieldDecl decl)
        {
            this.Result = decl.ToString() + decl.Children.Aggregate("", (a, b) => a + "\r\n" + b.GenerateChildContentKey());
        }

        public void Visit(EnumItemDecl decl)
        {
            this.Result = decl.ToString();
        }

        public void Visit(EnumDecl decl)
        {
            this.Result = decl.ToString();
        }

        public void Visit(TypedefDecl decl)
        {
            this.Result = decl.Parent is TemplateDecl
                ? decl.Parent.ToString()
                : decl.ToString();
        }
    }
}
