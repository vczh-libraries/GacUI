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

        static SymbolDecl ParseSymbol(string[] tokens, ref int index)
        {
            if (Parser.Token(tokens, ref index, "friend"))
            {
                Parser.SkipUntil(tokens, ref index, ";");
            }
            else if (Parser.Token(tokens, ref index, "template"))
            {
                var decl = new TemplateDecl();
                Parser.EnsureToken(tokens, ref index, "<");
                Parser.SkipUntil(tokens, ref index, ">");

                var element = ParseSymbol(tokens, ref index);
                if (element != null)
                {
                    decl.Element = element;
                    return decl;
                }
            }
            else if (Parser.Token(tokens, ref index, "namespace"))
            {
                var decl = new NamespaceDecl();
                decl.Name = Parser.EnsureId(tokens, ref index);

                Parser.EnsureToken(tokens, ref index, "{");
                ParseSymbols(tokens, ref index, decl);
                Parser.EnsureToken(tokens, ref index, "}");

                return decl;
            }
            else if (Parser.Token(tokens, ref index, "using"))
            {
                if (Parser.Token(tokens, ref index, "namespace"))
                {
                    var decl = new UsingNamespaceDecl();
                    decl.Path = new List<string>();
                    decl.Path.Add(Parser.EnsureId(tokens, ref index));

                    while (!Parser.Token(tokens, ref index, ";"))
                    {
                        Parser.EnsureToken(tokens, ref index, ":");
                        Parser.EnsureToken(tokens, ref index, ":");
                        decl.Path.Add(Parser.EnsureId(tokens, ref index));
                    }

                    return decl;
                }
                else
                {
                    string name = null;
                    if (Parser.Id(tokens, ref index, out name))
                    {
                        if (Parser.Token(tokens, ref index, "="))
                        {
                            var decl = new TypedefDecl
                            {
                                Name = name,
                                Type = TypeDecl.EnsureTypeWithoutName(tokens, ref index),
                            };
                            Parser.EnsureToken(tokens, ref index, ";");
                            return decl;
                        }
                    }
                    Parser.SkipUntil(tokens, ref index, ";");
                }
            }
            else if (Parser.Token(tokens, ref index, "typedef"))
            {
                string name = null;
                var type = TypeDecl.EnsureTypeWithName(tokens, ref index, out name);
                Parser.EnsureToken(tokens, ref index, ";");

                var decl = new TypedefDecl();
                decl.Name = name;
                decl.Type = type;
                return decl;
            }
            return null;
        }

        public static void ParseSymbols(string[] tokens, ref int index, SymbolDecl parent)
        {
            if (parent.Children == null)
            {
                parent.Children = new List<SymbolDecl>();
            }

            while (true)
            {
                int oldIndex = index;
                var decl = ParseSymbol(tokens, ref index);
                if (decl != null)
                {
                    parent.Children.Add(decl);
                }
                else if (index == oldIndex)
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

        public override string ToString()
        {
            return "using namespace " + this.Path.Aggregate("", (a, b) => a == "" ? b : a + "::" + b);
        }
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
            set
            {
                if (this.Children == null)
                {
                    this.Children = new List<SymbolDecl>();
                }
                this.Children.Clear();
                this.Children.Add(value);
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

        public override string ToString()
        {
            return "typedef " + this.Name + " : " + this.Type.ToString();
        }
    }
}
