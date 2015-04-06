using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    public enum Access
    {
        Public,
        Protected,
        Private,
    }

    public class SymbolDecl
    {
        public Access Access { get; set; }
        public string Name { get; set; }
        public List<SymbolDecl> Children { get; set; }

        public SymbolDecl()
        {
            this.Access = global::Parser.Access.Public;
        }

        public static void ParseSymbols(string[] tokens, ref int index, SymbolDecl parent)
        {
            if (parent.Children == null)
            {
                parent.Children = new List<SymbolDecl>();
            }

            while (true)
            {
                if (Parser.Token(tokens, ref index, "namespace"))
                {
                    var decl = new NamespaceDecl();
                    decl.Name = Parser.EnsureId(tokens, ref index);
                    parent.Children.Add(decl);

                    Parser.EnsureToken(tokens, ref index, "{");
                    ParseSymbols(tokens, ref index, decl);
                    Parser.EnsureToken(tokens, ref index, "}");
                }
                else
                {
                    break;
                }
            }
        }
    }

    class NamespaceDecl : SymbolDecl
    {
        public override string ToString()
        {
            return "namespace " + this.Name;
        }
    }

    class UsingNamespaceDecl : SymbolDecl
    {
        public List<string> Path { get; set; }
    }

    class TemplateDecl : SymbolDecl
    {
        public List<SymbolDecl> TypeParameters { get; set; }
        public List<TypeDecl> Specialization { get; set; }
        public SymbolDecl Element
        {
            get
            {
                return this.Children[0];
            }
        }
    }

    class ClassDecl : SymbolDecl
    {
        public List<TypeDecl> BaseTypes { get; set; }
    }

    class VarDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }

    class FuncDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }

    class EnumItemDecl : SymbolDecl
    {
    }

    class EnumDecl : SymbolDecl
    {
    }

    class TypedefDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }
}
