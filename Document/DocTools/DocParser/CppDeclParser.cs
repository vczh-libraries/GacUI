using DocSymbol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocParser
{
    public static class CppDeclParser
    {
        static SymbolDecl[] ParseSymbolInternal(string[] tokens, ref int index)
        {
            if (CppParser.Token(tokens, ref index, "public") || CppParser.Token(tokens, ref index, "protected") || CppParser.Token(tokens, ref index, "private"))
            {
                index--;
                return null;
            }
            TemplateDecl templateDecl = null;
            if (CppParser.Token(tokens, ref index, "template"))
            {
                templateDecl = new TemplateDecl
                {
                    TypeParameters = new List<TypeParameterDecl>(),
                    Specialization = new List<TypeDecl>(),
                };
                CppParser.EnsureToken(tokens, ref index, "<");
                if (!CppParser.Token(tokens, ref index, ">"))
                {
                    while (true)
                    {
                        string token = null;
                        CppParser.SkipUntilInTemplate(tokens, ref index, out token, ",", ">", "=");

                        index -= 2;
                        templateDecl.TypeParameters.Add(new TypeParameterDecl
                        {
                            Name = CppParser.EnsureId(tokens, ref index),
                        });
                        index++;

                        if (token == "=")
                        {
                            CppParser.SkipUntilInTemplate(tokens, ref index, out token, ",", ">");
                        }

                        if (token == ">")
                        {
                            break;
                        }
                    }
                }
            }

            if (CppParser.Token(tokens, ref index, "friend"))
            {
                int oldIndex = index - 1;
                string token = null;
                CppParser.SkipUntil(tokens, ref index, out token, ";", "{");
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

            if (CppParser.Token(tokens, ref index, "namespace"))
            {
                if (templateDecl != null)
                {
                    throw new ArgumentException("Failed to parse.");
                }
                var decl = new NamespaceDecl();
                decl.Name = CppParser.EnsureId(tokens, ref index);

                CppParser.EnsureToken(tokens, ref index, "{");
                ParseSymbols(tokens, ref index, decl);
                CppParser.EnsureToken(tokens, ref index, "}");

                return new SymbolDecl[] { decl };
            }
            else if (CppParser.Token(tokens, ref index, "using"))
            {
                if (CppParser.Token(tokens, ref index, "namespace"))
                {
                    if (templateDecl != null)
                    {
                        throw new ArgumentException("Failed to parse.");
                    }
                    var decl = new UsingNamespaceDecl();
                    decl.Path = new List<string>();
                    decl.Path.Add(CppParser.EnsureId(tokens, ref index));

                    while (!CppParser.Token(tokens, ref index, ";"))
                    {
                        CppParser.EnsureToken(tokens, ref index, ":");
                        CppParser.EnsureToken(tokens, ref index, ":");
                        decl.Path.Add(CppParser.EnsureId(tokens, ref index));
                    }

                    return new SymbolDecl[] { decl };
                }
                else
                {
                    string name = null;
                    if (CppParser.Id(tokens, ref index, out name))
                    {
                        if (templateDecl != null)
                        {
                            if (CppParser.Token(tokens, ref index, "<"))
                            {
                                while (true)
                                {
                                    templateDecl.Specialization.Add(CppTypeParser.EnsureTypeWithoutName(tokens, ref index));
                                    if (CppParser.Token(tokens, ref index, ">"))
                                    {
                                        break;
                                    }
                                    CppParser.EnsureToken(tokens, ref index, ",");
                                }
                            }
                        }
                        if (CppParser.Token(tokens, ref index, "="))
                        {
                            SymbolDecl decl = new TypedefDecl
                            {
                                Name = name,
                                Type = CppTypeParser.EnsureTypeWithoutName(tokens, ref index),
                            };
                            CppParser.EnsureToken(tokens, ref index, ";");

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
                    CppParser.SkipUntil(tokens, ref index, ";");
                }
            }
            else if (CppParser.Token(tokens, ref index, "typedef"))
            {
                if (templateDecl != null)
                {
                    throw new ArgumentException("Failed to parse.");
                }
                string name = null;
                var type = CppTypeParser.EnsureTypeWithName(tokens, ref index, out name);
                CppParser.EnsureToken(tokens, ref index, ";");

                var decl = new TypedefDecl();
                decl.Name = name;
                decl.Type = type;
                return new SymbolDecl[] { decl };
            }
            else if (CppParser.Token(tokens, ref index, "enum"))
            {
                if (templateDecl != null)
                {
                    throw new ArgumentException("Failed to parse.");
                }
                bool enumClass = CppParser.Token(tokens, ref index, "class");
                string name = CppParser.EnsureId(tokens, ref index);
                if (CppParser.Token(tokens, ref index, ":"))
                {
                    CppTypeParser.EnsureTypeWithoutName(tokens, ref index);
                }
                if (!CppParser.Token(tokens, ref index, ";"))
                {
                    CppParser.EnsureToken(tokens, ref index, "{");
                    var decl = new EnumDecl
                    {
                        Name = name,
                        EnumClass = enumClass,
                        Children = new List<SymbolDecl>(),
                    };

                    while (true)
                    {
                        if (CppParser.Token(tokens, ref index, "}"))
                        {
                            break;
                        }

                        var document = "";
                        while (index < tokens.Length && tokens[index].Length >= 3 && tokens[index].StartsWith("///"))
                        {
                            var line = tokens[index];
                            document += line.StartsWith("/// ") || line.StartsWith("///\t")
                                ? line.Substring(4)
                                : line.Substring(3);
                            document += "\r\n";
                            index++;
                        }
                        decl.Children.Add(new EnumItemDecl
                        {
                            Name = CppParser.EnsureId(tokens, ref index),
                            Document = document,
                        });

                        string token = null;
                        CppParser.SkipUntil(tokens, ref index, out token, ",", "}");
                        if (token == "}")
                        {
                            break;
                        }
                    }

                    if (CppParser.Id(tokens, ref index, out name))
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
                        CppParser.EnsureToken(tokens, ref index, ";");
                        return new SymbolDecl[] { decl, varDecl };
                    }
                    else
                    {
                        CppParser.EnsureToken(tokens, ref index, ";");
                        return new SymbolDecl[] { decl };
                    }
                }
            }
            else if (CppParser.Token(tokens, ref index, "struct") || CppParser.Token(tokens, ref index, "class") || CppParser.Token(tokens, ref index, "union"))
            {
                if (CppParser.Token(tokens, ref index, "{"))
                {
                    if (tokens[index - 2] == "class")
                    {
                        throw new ArgumentException("Failed to parse.");
                    }

                    var decl = new GroupedFieldDecl
                    {
                        Grouping = tokens[index - 2] == "struct" ? Grouping.Struct : Grouping.Union,
                    };
                    ParseSymbols(tokens, ref index, decl);
                    CppParser.EnsureToken(tokens, ref index, "}");
                    CppParser.EnsureToken(tokens, ref index, ";");
                    return new SymbolDecl[] { decl };
                }
                else
                {
                    string name = CppParser.EnsureId(tokens, ref index);
                    if (!CppParser.Token(tokens, ref index, ";"))
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
                            if (CppParser.Token(tokens, ref index, "<"))
                            {
                                if (!CppParser.Token(tokens, ref index, ">"))
                                {
                                    while (true)
                                    {
                                        int oldIndex = index;
                                        try
                                        {
                                            templateDecl.Specialization.Add(CppTypeParser.EnsureTypeWithoutName(tokens, ref index));
                                        }
                                        catch (ArgumentException)
                                        {
                                            index = oldIndex;
                                            CppParser.SkipUntilInTemplate(tokens, ref index, ",", ">");
                                            index--;

                                            templateDecl.Specialization.Add(new ConstantTypeDecl
                                            {
                                                Value = tokens
                                                    .Skip(oldIndex)
                                                    .Take(index - oldIndex)
                                                    .Aggregate((a, b) => a + " " + b),
                                            });
                                        }
                                        if (CppParser.Token(tokens, ref index, ">"))
                                        {
                                            break;
                                        }
                                        CppParser.EnsureToken(tokens, ref index, ",");
                                    }
                                }
                            }
                        }

                        CppParser.Token(tokens, ref index, "abstract");
                        if (CppParser.Token(tokens, ref index, ":"))
                        {
                            while (true)
                            {
                                Access access = classDecl.ClassType == ClassType.Class ? Access.Private : Access.Public;
                                CppParser.Token(tokens, ref index, "virtual");
                                if (CppParser.Token(tokens, ref index, "private"))
                                {
                                    access = Access.Private;
                                }
                                else if (CppParser.Token(tokens, ref index, "protected"))
                                {
                                    access = Access.Protected;
                                }
                                else if (CppParser.Token(tokens, ref index, "public"))
                                {
                                    access = Access.Public;
                                }
                                CppParser.Token(tokens, ref index, "virtual");
                                classDecl.BaseTypes.Add(new BaseTypeDecl
                                {
                                    Access = access,
                                    Type = CppTypeParser.EnsureTypeWithoutName(tokens, ref index),
                                });
                                if (!CppParser.Token(tokens, ref index, ","))
                                {
                                    break;
                                }
                            }
                        }

                        CppParser.EnsureToken(tokens, ref index, "{");
                        while (true)
                        {
                            if (CppParser.Token(tokens, ref index, "}"))
                            {
                                break;
                            }

                            Access access = classDecl.ClassType == ClassType.Class ? Access.Private : Access.Public;
                            if (CppParser.Token(tokens, ref index, "private"))
                            {
                                access = Access.Private;
                                CppParser.EnsureToken(tokens, ref index, ":");
                            }
                            else if (CppParser.Token(tokens, ref index, "protected"))
                            {
                                access = Access.Protected;
                                CppParser.EnsureToken(tokens, ref index, ":");
                            }
                            else if (CppParser.Token(tokens, ref index, "public"))
                            {
                                access = Access.Public;
                                CppParser.EnsureToken(tokens, ref index, ":");
                            }
                            ParseSymbols(tokens, ref index, classDecl, access);
                        }

                        SymbolDecl decl = classDecl;
                        if (templateDecl != null)
                        {
                            templateDecl.Element = decl;
                            decl = templateDecl;
                        }

                        if (CppParser.Id(tokens, ref index, out name))
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
                            CppParser.EnsureToken(tokens, ref index, ";");
                            return new SymbolDecl[] { decl, varDecl };
                        }
                        else
                        {
                            CppParser.EnsureToken(tokens, ref index, ";");
                            return new SymbolDecl[] { decl };
                        }
                    }
                }
            }
            else if (!CppParser.Token(tokens, ref index, ";"))
            {
                Function function = Function.Function;
                {
                    int oldIndex = index;
                    string name = null;
                    if (CppParser.Id(tokens, ref index, out name))
                    {
                        if (CppParser.Token(tokens, ref index, "("))
                        {
                            CppParser.SkipUntil(tokens, ref index, ")");
                            if (CppParser.Token(tokens, ref index, ";") || CppParser.Token(tokens, ref index, "=") || CppParser.Token(tokens, ref index, ":") || CppParser.Token(tokens, ref index, "{"))
                            {
                                function = Function.Constructor;
                            }
                        }
                        index = oldIndex;
                    }
                    else if (CppParser.Token(tokens, ref index, "~"))
                    {
                        function = Function.Destructor;
                    }
                }

                if (function == Function.Function)
                {
                    Virtual virtualFunction = Virtual.Normal;
                    CppParser.Token(tokens, ref index, "extern");
                    CppParser.Token(tokens, ref index, "mutable");
                    if (CppParser.Token(tokens, ref index, "virtual"))
                    {
                        virtualFunction = Virtual.Virtual;
                    }
                    else if (CppParser.Token(tokens, ref index, "static"))
                    {
                        virtualFunction = Virtual.Static;
                    }
                    CppParser.Token(tokens, ref index, "inline");
                    CppParser.Token(tokens, ref index, "__forceinline");

                    if (CppParser.Token(tokens, ref index, "operator"))
                    {
                        TypeDecl returnType = null;
                        {
                            int oldIndex = index;
                            CppParser.SkipUntilInTemplate(tokens, ref index, "(");
                            int modify = --index;

                            tokens[modify] = "$";
                            index = oldIndex;
                            returnType = CppTypeParser.EnsureTypeWithoutName(tokens, ref index);
                            if (index != modify)
                            {
                                throw new ArgumentException("Failed to parse.");
                            }
                            tokens[modify] = "(";
                        }
                        var decl = new FuncDecl
                        {
                            Virtual = Virtual.Normal,
                            Name = "operator",
                            Function = function,
                        };

                        TypeDecl functionType = null;
                        Action<TypeDecl> continuation = null;
                        CallingConvention callingConvention = CallingConvention.Default;
                        CppTypeParser.ParseTypeContinueAfterName(tokens, ref index, ref callingConvention, out functionType, out continuation);
                        continuation(returnType);
                        decl.Type = functionType;

                        if (!CppParser.Token(tokens, ref index, ";"))
                        {
                            CppParser.EnsureToken(tokens, ref index, "{");
                            CppParser.SkipUntil(tokens, ref index, "}");
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
                        if (CppTypeParser.ParseType(tokens, ref index, out type, out name))
                        {
                            if (name == null)
                            {
                                throw new ArgumentException("Failed to parse.");
                            }

                            if (type is FunctionTypeDecl)
                            {
                                if (CppParser.Token(tokens, ref index, "="))
                                {
                                    if (CppParser.Token(tokens, ref index, "0"))
                                    {
                                        virtualFunction = Virtual.Abstract;
                                    }
                                    else
                                    {
                                        CppParser.EnsureToken(tokens, ref index, "default", "delete");
                                    }
                                }

                                var decl = new FuncDecl
                                {
                                    Virtual = virtualFunction,
                                    Name = name,
                                    Type = type,
                                    Function = Function.Function,
                                };
                                if (!CppParser.Token(tokens, ref index, ";"))
                                {
                                    CppParser.EnsureToken(tokens, ref index, "{");
                                    CppParser.SkipUntil(tokens, ref index, "}");
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
                                if (CppParser.Token(tokens, ref index, "="))
                                {
                                    CppParser.SkipUntil(tokens, ref index, ";");
                                }
                                else
                                {
                                    CppParser.EnsureToken(tokens, ref index, ";");
                                }

                                if (!(type is ClassMemberTypeDecl))
                                {
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
                }
                else
                {
                    var decl = new FuncDecl
                    {
                        Virtual = Virtual.Normal,
                        Name = (function == Function.Constructor ? "" : "~") + CppParser.EnsureId(tokens, ref index),
                        Function = function,
                    };

                    TypeDecl functionType = null;
                    Action<TypeDecl> continuation = null;
                    CallingConvention callingConvention = CallingConvention.Default;
                    CppTypeParser.ParseTypeContinueAfterName(tokens, ref index, ref callingConvention, out functionType, out continuation);
                    continuation(new RefTypeDecl
                    {
                        Name = "void"
                    });
                    decl.Type = functionType;

                    if (CppParser.Token(tokens, ref index, "="))
                    {
                        CppParser.EnsureToken(tokens, ref index, "default", "delete");
                    }

                    if (!CppParser.Token(tokens, ref index, ";"))
                    {
                        if (CppParser.Token(tokens, ref index, ":"))
                        {
                            CppParser.SkipUntil(tokens, ref index, "{");
                        }
                        else
                        {
                            CppParser.EnsureToken(tokens, ref index, "{");
                        }
                        CppParser.SkipUntil(tokens, ref index, "}");
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
            }
            return null;
        }
        static SymbolDecl[] ParseSymbol(string[] tokens, ref int index)
        {
            string document = null;
            while (index < tokens.Length && tokens[index].Length >= 3 && tokens[index].StartsWith("///"))
            {
                var token = tokens[index];
                if (document == null)
                {
                    document = "";
                }

                document += token.StartsWith("/// ") || token.StartsWith("///\t")
                    ? token.Substring(4)
                    : token.Substring(3);
                document += "\r\n";
                index++;
            }

            var symbols = ParseSymbolInternal(tokens, ref index);
            if (symbols != null && document != null)
            {
                symbols[0].Document = document;
            }
            return symbols;
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
}
