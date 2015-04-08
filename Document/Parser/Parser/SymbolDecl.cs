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

        static SymbolDecl[] ParseSymbol(string[] tokens, ref int index)
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

                var elements = ParseSymbol(tokens, ref index);
                if (elements != null)
                {
                    decl.Element = elements[0];
                    return new SymbolDecl[] { decl };
                }
            }
            else if (Parser.Token(tokens, ref index, "namespace"))
            {
                var decl = new NamespaceDecl();
                decl.Name = Parser.EnsureId(tokens, ref index);

                Parser.EnsureToken(tokens, ref index, "{");
                ParseSymbols(tokens, ref index, decl);
                Parser.EnsureToken(tokens, ref index, "}");

                return new SymbolDecl[] { decl };
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

                    return new SymbolDecl[] { decl };
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
                            return new SymbolDecl[] { decl };
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
                return new SymbolDecl[] { decl };
            }
            else if (Parser.Token(tokens, ref index, "enum"))
            {
                string name = Parser.EnsureId(tokens, ref index);
                if (Parser.Token(tokens, ref index, ":"))
                {
                    TypeDecl.EnsureTypeWithoutName(tokens, ref index);
                }
                if (!Parser.Token(tokens, ref index, ";"))
                {
                    Parser.EnsureToken(tokens, ref index, "{");
                    var decl = new EnumDecl
                    {
                        Name = name,
                        Children = new List<SymbolDecl>(),
                    };

                    while (true)
                    {
                        if (Parser.Token(tokens, ref index, "}"))
                        {
                            break;
                        }

                        decl.Children.Add(new EnumItemDecl
                        {
                            Name = Parser.EnsureId(tokens, ref index),
                        });

                        string token = null;
                        Parser.SkipUntil(tokens, ref index, out token, ",", "}");
                        if (token == "}")
                        {
                            break;
                        }
                    }

                    if (Parser.Id(tokens, ref index, out name))
                    {
                        var varDecl = new VarDecl
                        {
                            Name = name,
                            Type = new RefTypeDecl
                            {
                                Name = decl.Name,
                            },
                        };
                        Parser.EnsureToken(tokens, ref index, ";");
                        return new SymbolDecl[] { decl, varDecl };
                    }
                    else
                    {
                        Parser.EnsureToken(tokens, ref index, ";");
                        return new SymbolDecl[] { decl };
                    }
                }
            }
            else if (Parser.Token(tokens, ref index, "struct") || Parser.Token(tokens, ref index, "class") || Parser.Token(tokens, ref index, "union"))
            {
                string name = Parser.EnsureId(tokens, ref index);
                if (!Parser.Token(tokens, ref index, ";"))
                {
                    var decl = new ClassDecl
                    {
                        ClassType =
                            tokens[index - 2] == "struct" ? ClassType.Struct :
                            tokens[index - 2] == "class" ? ClassType.Class :
                            ClassType.Union,
                        BaseTypes = new List<BaseTypeDecl>(),
                        Name = name,
                    };

                    if (Parser.Token(tokens, ref index, ":"))
                    {
                        while (true)
                        {
                            Access access = decl.ClassType == ClassType.Class ? Access.Private : Access.Public;
                            if (Parser.Token(tokens, ref index, "private"))
                            {
                                access = Access.Private;
                            }
                            else if (Parser.Token(tokens, ref index, "protected"))
                            {
                                access = Access.Protected;
                            }
                            else if (Parser.Token(tokens, ref index, "public"))
                            {
                                access = Access.Public;
                            }
                            decl.BaseTypes.Add(new BaseTypeDecl
                            {
                                Access = access,
                                Type = TypeDecl.EnsureTypeWithoutName(tokens, ref index),
                            });
                            if (!Parser.Token(tokens, ref index, ","))
                            {
                                break;
                            }
                        }
                    }

                    Parser.EnsureToken(tokens, ref index, "{");
                    while (true)
                    {
                        if (Parser.Token(tokens, ref index, "}"))
                        {
                            break;
                        }

                        Access access = decl.ClassType == ClassType.Class ? Access.Private : Access.Public;
                        if (Parser.Token(tokens, ref index, "private"))
                        {
                            access = Access.Private;
                            Parser.EnsureToken(tokens, ref index, ":");
                        }
                        else if (Parser.Token(tokens, ref index, "protected"))
                        {
                            access = Access.Protected;
                            Parser.EnsureToken(tokens, ref index, ":");
                        }
                        else if (Parser.Token(tokens, ref index, "public"))
                        {
                            access = Access.Public;
                            Parser.EnsureToken(tokens, ref index, ":");
                        }
                        ParseSymbols(tokens, ref index, decl, access);
                    }

                    if (Parser.Id(tokens, ref index, out name))
                    {
                        var varDecl = new VarDecl
                        {
                            Name = name,
                            Type = new RefTypeDecl
                            {
                                Name = decl.Name,
                            },
                        };
                        Parser.EnsureToken(tokens, ref index, ";");
                        return new SymbolDecl[] { decl, varDecl };
                    }
                    else
                    {
                        Parser.EnsureToken(tokens, ref index, ";");
                        return new SymbolDecl[] { decl };
                    }
                }
            }
            return null;
        }

        public static void ParseSymbols(string[] tokens, ref int index, SymbolDecl parent, Access access = Access.Public)
        {
            if (parent.Children == null)
            {
                parent.Children = new List<SymbolDecl>();
            }

            while (true)
            {
                int oldIndex = index;
                var decls = ParseSymbol(tokens, ref index);
                if (decls != null)
                {
                    foreach (var decl in decls)
                    {
                        decl.Access = access;
                    }
                    parent.Children.AddRange(decls);
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

    enum ClassType
    {
        Class,
        Struct,
        Union,
    }

    class BaseTypeDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }

        public override string ToString()
        {
            switch (this.Access)
            {
                case global::Parser.Access.Private:
                    return "private " + this.Type.ToString();
                case global::Parser.Access.Protected:
                    return "protected " + this.Type.ToString();
                default:
                    return "public " + this.Type.ToString();
            }
        }
    }

    class ClassDecl : SymbolDecl
    {
        public ClassType ClassType { get; set; }
        public List<BaseTypeDecl> BaseTypes { get; set; }

        public override string ToString()
        {
            string result = "";
            switch (this.ClassType)
            {
                case global::Parser.ClassType.Class:
                    result = "class " + this.Name;
                    break;
                case global::Parser.ClassType.Struct:
                    result = "struct " + this.Name;
                    break;
                case global::Parser.ClassType.Union:
                    result = "union " + this.Name;
                    break;
            }
            result += this.BaseTypes.Aggregate("", (a, b) => a == "" ? " : " + b.ToString() : a + ", " + b.ToString());
            return result;
        }
    }

    class VarDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }

        public override string ToString()
        {
            return "var " + this.Name + " : " + this.Type.ToString();
        }
    }

    class FuncDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
    }

    class EnumItemDecl : SymbolDecl
    {
        public override string ToString()
        {
            return "enum_item " + this.Name;
        }
    }

    class EnumDecl : SymbolDecl
    {
        public override string ToString()
        {
            return "enum " + this.Name;
        }
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
