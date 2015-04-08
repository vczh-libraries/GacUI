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
            while (index < tokens.Length && tokens[index].Length >= 3 && tokens[index].StartsWith("///"))
            {
                index++;
            }

            if (Parser.Token(tokens, ref index, "public") || Parser.Token(tokens, ref index, "protected") || Parser.Token(tokens, ref index, "private"))
            {
                index--;
                return null;
            }
            TemplateDecl templateDecl = null;
            if (Parser.Token(tokens, ref index, "template"))
            {
                templateDecl = new TemplateDecl
                {
                    TypeParameters = new List<string>(),
                    Specialization = new List<TypeDecl>(),
                };
                Parser.EnsureToken(tokens, ref index, "<");
                if (!Parser.Token(tokens, ref index, ">"))
                {
                    while (true)
                    {
                        string token = null;
                        Parser.SkipUntilInTemplate(tokens, ref index, out token, ",", ">", "=");

                        index -= 2;
                        templateDecl.TypeParameters.Add(Parser.EnsureId(tokens, ref index));
                        index++;

                        if (token == "=")
                        {
                            Parser.SkipUntilInTemplate(tokens, ref index, out token, ",", ">");
                        }

                        if (token == ">")
                        {
                            break;
                        }
                    }
                }
            }

            if (Parser.Token(tokens, ref index, "friend"))
            {
                int oldIndex = index - 1;
                string token = null;
                Parser.SkipUntil(tokens, ref index, out token, ";", "{");
                if (token == ";")
                {
                    return null;
                }
                else
                {
                    index = oldIndex;
                    tokens[index] = "static";
                }
            }

            if (Parser.Token(tokens, ref index, "namespace"))
            {
                if (templateDecl != null)
                {
                    throw new ArgumentException("Failed to parse.");
                }
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
                    if (templateDecl != null)
                    {
                        throw new ArgumentException("Failed to parse.");
                    }
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
                        if (templateDecl != null)
                        {
                            if (Parser.Token(tokens, ref index, "<"))
                            {
                                while (true)
                                {
                                    templateDecl.Specialization.Add(TypeDecl.EnsureTypeWithoutName(tokens, ref index));
                                    if (Parser.Token(tokens, ref index, ">"))
                                    {
                                        break;
                                    }
                                    Parser.EnsureToken(tokens, ref index, ",");
                                }
                            }
                        }
                        if (Parser.Token(tokens, ref index, "="))
                        {
                            SymbolDecl decl = new TypedefDecl
                            {
                                Name = name,
                                Type = TypeDecl.EnsureTypeWithoutName(tokens, ref index),
                            };
                            Parser.EnsureToken(tokens, ref index, ";");

                            if (templateDecl != null)
                            {
                                templateDecl.Element = decl;
                                decl = templateDecl;
                            }
                            return new SymbolDecl[] { decl };
                        }
                    }
                    if (templateDecl != null)
                    {
                        throw new ArgumentException("Failed to parse.");
                    }
                    Parser.SkipUntil(tokens, ref index, ";");
                }
            }
            else if (Parser.Token(tokens, ref index, "typedef"))
            {
                if (templateDecl != null)
                {
                    throw new ArgumentException("Failed to parse.");
                }
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
                if (templateDecl != null)
                {
                    throw new ArgumentException("Failed to parse.");
                }
                Parser.Token(tokens, ref index, "class");
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
                            Static = false,
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
                    var classDecl = new ClassDecl
                    {
                        ClassType =
                            tokens[index - 2] == "struct" ? ClassType.Struct :
                            tokens[index - 2] == "class" ? ClassType.Class :
                            ClassType.Union,
                        BaseTypes = new List<BaseTypeDecl>(),
                        Name = name,
                    };

                    if (templateDecl != null)
                    {
                        if (Parser.Token(tokens, ref index, "<"))
                        {
                            if (!Parser.Token(tokens, ref index, ">"))
                            {
                                while (true)
                                {
                                    int oldIndex = index;
                                    try
                                    {
                                        templateDecl.Specialization.Add(TypeDecl.EnsureTypeWithoutName(tokens, ref index));
                                    }
                                    catch (ArgumentException)
                                    {
                                        index = oldIndex;
                                        Parser.SkipUntilInTemplate(tokens, ref index, ",", ">");
                                        index--;

                                        templateDecl.Specialization.Add(new ConstantTypeDecl
                                            {
                                                Value = tokens
                                                    .Skip(oldIndex)
                                                    .Take(index - oldIndex)
                                                    .Aggregate((a, b) => a + " " + b),
                                            });
                                    }
                                    if (Parser.Token(tokens, ref index, ">"))
                                    {
                                        break;
                                    }
                                    Parser.EnsureToken(tokens, ref index, ",");
                                }
                            }
                        }
                    }

                    Parser.Token(tokens, ref index, "abstract");
                    if (Parser.Token(tokens, ref index, ":"))
                    {
                        while (true)
                        {
                            Access access = classDecl.ClassType == ClassType.Class ? Access.Private : Access.Public;
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
                            Parser.Token(tokens, ref index, "virtual");
                            classDecl.BaseTypes.Add(new BaseTypeDecl
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

                        Access access = classDecl.ClassType == ClassType.Class ? Access.Private : Access.Public;
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
                        ParseSymbols(tokens, ref index, classDecl, access);
                    }

                    SymbolDecl decl = classDecl;
                    if (templateDecl != null)
                    {
                        templateDecl.Element = decl;
                        decl = templateDecl;
                    }

                    if (Parser.Id(tokens, ref index, out name))
                    {
                        var varDecl = new VarDecl
                        {
                            Static = false,
                            Name = name,
                            Type = new RefTypeDecl
                            {
                                Name = classDecl.Name,
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
            else if (index < tokens.Length)
            {
                Function function = Function.Function;
                {
                    int oldIndex = index;
                    string name = null;
                    if (Parser.Id(tokens, ref index, out name))
                    {
                        if (Parser.Token(tokens, ref index, "("))
                        {
                            Parser.SkipUntil(tokens, ref index, ")");
                            if (Parser.Token(tokens, ref index, ";") || Parser.Token(tokens, ref index, "=") || Parser.Token(tokens, ref index, ":") || Parser.Token(tokens, ref index, "{"))
                            {
                                function = Function.Constructor;
                            }
                        }
                        index = oldIndex;
                    }
                    else if (Parser.Token(tokens, ref index, "~"))
                    {
                        function = Function.Destructor;
                    }
                }

                if (function == Function.Function)
                {
                    Virtual virtualFunction = Virtual.Normal;
                    Parser.Token(tokens, ref index, "extern");
                    Parser.Token(tokens, ref index, "mutable");
                    if (Parser.Token(tokens, ref index, "virtual"))
                    {
                        virtualFunction = Virtual.Virtual;
                    }
                    else if (Parser.Token(tokens, ref index, "static"))
                    {
                        virtualFunction = Virtual.Static;
                    }
                    Parser.Token(tokens, ref index, "inline");
                    Parser.Token(tokens, ref index, "__forceinline");

                    if (Parser.Token(tokens, ref index, "operator"))
                    {
                        TypeDecl returnType = null;
                        {
                            int oldIndex = index;
                            Parser.SkipUntilInTemplate(tokens, ref index, "(");
                            int modify = --index;

                            tokens[modify] = "$";
                            index = oldIndex;
                            returnType = TypeDecl.EnsureTypeWithoutName(tokens, ref index);
                            if (index != modify)
                            {
                                throw new ArgumentException("Failed to parse.");
                            }
                            tokens[modify] = "(";
                        }
                        var decl = new FuncDecl
                        {
                            Virtual = global::Parser.Virtual.Normal,
                            Name = "operator",
                            Function = function,
                        };

                        TypeDecl functionType = null;
                        Action<TypeDecl> continuation = null;
                        CallingConvention callingConvention = CallingConvention.Default;
                        TypeDecl.ParseTypeContinueAfterName(tokens, ref index, ref callingConvention, out functionType, out continuation);
                        continuation(returnType);
                        decl.Type = functionType;

                        if (!Parser.Token(tokens, ref index, ";"))
                        {
                            Parser.EnsureToken(tokens, ref index, "{");
                            Parser.SkipUntil(tokens, ref index, "}");
                        }

                        if (templateDecl != null)
                        {
                            templateDecl.Element = decl;
                            return new SymbolDecl[] { templateDecl };
                        }
                        else
                        {
                            return new SymbolDecl[] { decl };
                        }
                    }
                    else
                    {
                        string name = null;
                        TypeDecl type = null;
                        if (TypeDecl.ParseType(tokens, ref index, out type, out name))
                        {
                            if (name == null)
                            {
                                throw new ArgumentException("Failed to parse.");
                            }

                            if (type is FunctionTypeDecl)
                            {
                                if (Parser.Token(tokens, ref index, "="))
                                {
                                    if (Parser.Token(tokens, ref index, "0"))
                                    {
                                        virtualFunction = Virtual.Abstract;
                                    }
                                    else
                                    {
                                        Parser.EnsureToken(tokens, ref index, "default", "delete");
                                    }
                                }

                                var decl = new FuncDecl
                                {
                                    Virtual = virtualFunction,
                                    Name = name,
                                    Type = type,
                                    Function = Function.Function,
                                };
                                if (!Parser.Token(tokens, ref index, ";"))
                                {
                                    Parser.EnsureToken(tokens, ref index, "{");
                                    Parser.SkipUntil(tokens, ref index, "}");
                                }

                                if (templateDecl != null)
                                {
                                    templateDecl.Element = decl;
                                    return new SymbolDecl[] { templateDecl };
                                }
                                else
                                {
                                    return new SymbolDecl[] { decl };
                                }
                            }
                            else
                            {
                                if (virtualFunction != Virtual.Normal && virtualFunction != Virtual.Static)
                                {
                                    throw new ArgumentException("Failed to parse.");
                                }
                                if (Parser.Token(tokens, ref index, "="))
                                {
                                    Parser.SkipUntil(tokens, ref index, ";");
                                }
                                else
                                {
                                    Parser.EnsureToken(tokens, ref index, ";");
                                }

                                var decl = new VarDecl
                                {
                                    Static = virtualFunction == Virtual.Static,
                                    Name = name,
                                    Type = type,
                                };
                                return new SymbolDecl[] { decl };
                            }
                        }
                    }
                }
                else
                {
                    var decl = new FuncDecl
                    {
                        Virtual = global::Parser.Virtual.Normal,
                        Name = (function == Function.Constructor ? "" : "~") + Parser.EnsureId(tokens, ref index),
                        Function = function,
                    };

                    TypeDecl functionType = null;
                    Action<TypeDecl> continuation = null;
                    CallingConvention callingConvention = CallingConvention.Default;
                    TypeDecl.ParseTypeContinueAfterName(tokens, ref index, ref callingConvention, out functionType, out continuation);
                    continuation(new RefTypeDecl
                    {
                        Name = "void"
                    });
                    decl.Type = functionType;

                    if (Parser.Token(tokens, ref index, "="))
                    {
                        Parser.EnsureToken(tokens, ref index, "default", "delete");
                    }

                    if (!Parser.Token(tokens, ref index, ";"))
                    {
                        if (Parser.Token(tokens, ref index, ":"))
                        {
                            Parser.SkipUntil(tokens, ref index, "{");
                        }
                        else
                        {
                            Parser.EnsureToken(tokens, ref index, "{");
                        }
                        Parser.SkipUntil(tokens, ref index, "}");
                    }
                    return new SymbolDecl[] { decl };
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
        public List<string> TypeParameters { get; set; }
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

        public override string ToString()
        {
            string result = "template<" + this.TypeParameters.Aggregate("", (a, b) => a == "" ? b : a + ", " + b) + "> ";
            if (this.Specialization.Count > 0)
            {
                result += this.Specialization
                    .Select(t => t.ToString())
                    .Aggregate("", (a, b) => a == "" ? "specialization<" + b : a + ", " + b)
                    + "> ";
            }
            result += this.Element.ToString();
            return result;
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
        public bool Static { get; set; }

        public override string ToString()
        {
            return "var " + this.Name + " : " + this.Type.ToString();
        }
    }

    enum Virtual
    {
        Static,
        Normal,
        Virtual,
        Abstract,
    }

    enum Function
    {
        Constructor,
        Destructor,
        Function,
    }

    class FuncDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
        public Virtual Virtual { get; set; }
        public Function Function { get; set; }

        public override string ToString()
        {
            var result = "";
            switch (this.Function)
            {
                case global::Parser.Function.Constructor:
                    result = "ctor ";
                    break;
                case global::Parser.Function.Destructor:
                    result = "dtor ";
                    break;
                default:
                    switch (this.Virtual)
                    {
                        case global::Parser.Virtual.Virtual:
                            result = "virtual function ";
                            break;
                        case global::Parser.Virtual.Abstract:
                            result = "abstract function ";
                            break;
                        default:
                            result = "function ";
                            break;
                    }
                    break;
            }
            result += this.Name + " : " + this.Type.ToString();
            return result;
        }
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
