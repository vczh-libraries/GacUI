using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocSymbol
{
    class PrintSymbolDeclVisitor : SymbolDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(GlobalDecl decl)
        {
            this.Result = "";
        }

        public void Visit(NamespaceDecl decl)
        {
            this.Result = "namespace " + decl.Name;
        }

        public void Visit(UsingNamespaceDecl decl)
        {
            this.Result = "using namespace " + decl.Path.Aggregate("", (a, b) => a == "" ? b : a + "::" + b);
        }

        public static string PrintTemplateDecl(TemplateDecl decl)
        {
            string result = "template<" + decl.TypeParameters.Aggregate("", (a, b) => a == "" ? b.Name : a + ", " + b.Name) + "> ";
            if (decl.Specialization.Count > 0)
            {
                result += decl.Specialization
                    .Select(t => t.ToString())
                    .Aggregate("", (a, b) => a == "" ? "specialization<" + b : a + ", " + b)
                    + "> ";
            }
            return result;
        }

        public void Visit(TypeParameterDecl decl)
        {
            this.Result = "typename " + decl.Name;
        }

        public void Visit(TemplateDecl decl)
        {
            this.Result = PrintTemplateDecl(decl) + decl.Element.ToString();
        }

        public void Visit(BaseTypeDecl decl)
        {
            switch (decl.Access)
            {
                case global::DocSymbol.Access.Private:
                    this.Result = "private " + decl.Type.ToString();
                    break;
                case global::DocSymbol.Access.Protected:
                    this.Result = "protected " + decl.Type.ToString();
                    break;
                default:
                    this.Result = "public " + decl.Type.ToString();
                    break;
            }
        }

        public void Visit(ClassDecl decl)
        {
            string result = "";
            switch (decl.ClassType)
            {
                case global::DocSymbol.ClassType.Class:
                    result = "class " + decl.Name;
                    break;
                case global::DocSymbol.ClassType.Struct:
                    result = "struct " + decl.Name;
                    break;
                case global::DocSymbol.ClassType.Union:
                    result = "union " + decl.Name;
                    break;
            }
            result += decl.BaseTypes.Aggregate("", (a, b) => a == "" ? " : " + b.ToString() : a + ", " + b.ToString());
            this.Result = result;
        }

        public void Visit(VarDecl decl)
        {
            this.Result = "var " + decl.Name + " : " + decl.Type.ToString();
        }

        public void Visit(FuncDecl decl)
        {
            var result = "";
            switch (decl.Function)
            {
                case global::DocSymbol.Function.Constructor:
                    result = "ctor ";
                    break;
                case global::DocSymbol.Function.Destructor:
                    result = "dtor ";
                    break;
                default:
                    switch (decl.Virtual)
                    {
                        case global::DocSymbol.Virtual.Virtual:
                            result = "virtual function ";
                            break;
                        case global::DocSymbol.Virtual.Abstract:
                            result = "abstract function ";
                            break;
                        default:
                            result = "function ";
                            break;
                    }
                    break;
            }
            result += decl.Name + " : " + decl.Type.ToString();
            this.Result = result;
        }

        public void Visit(GroupedFieldDecl decl)
        {
            this.Result = decl.Grouping == global::DocSymbol.Grouping.Struct ? "struct" : "union";
        }

        public void Visit(EnumItemDecl decl)
        {
            this.Result = "enum_item " + decl.Name;
        }

        public void Visit(EnumDecl decl)
        {
            this.Result = "enum " + decl.Name;
        }

        public void Visit(TypedefDecl decl)
        {
            this.Result = "typedef " + decl.Name + " : " + decl.Type.ToString();
        }
    }
}
