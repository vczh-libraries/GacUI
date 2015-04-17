using DocSymbol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocParser
{
    public static class CppTypeParser
    {
        static bool ParseMiniType(string[] tokens, ref int index, out TypeDecl decl)
        {
            decl = null;
            if (CppParser.Token(tokens, ref index, "const"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Const,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (CppParser.Token(tokens, ref index, "volatile"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Volatile,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (CppParser.Token(tokens, ref index, "signed"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Signed,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (CppParser.Token(tokens, ref index, "unsigned"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Unsigned,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (CppParser.Token(tokens, ref index, "decltype"))
            {
                CppParser.EnsureToken(tokens, ref index, "(");
                int oldIndex = index;
                CppParser.SkipUntil(tokens, ref index, ")");

                decl = new DeclTypeDecl
                {
                    Expression = tokens
                        .Skip(oldIndex)
                        .Take(index - 1 - oldIndex)
                        .Aggregate((a, b) => a + " " + b),
                };
                return true;
            }
            else if (CppParser.Token(tokens, ref index, "false") || CppParser.Token(tokens, ref index, "true"))
            {
                decl = new ConstantTypeDecl
                {
                    Value = tokens[index - 1],
                };
                return true;
            }
            else
            {
                if (index < tokens.Length)
                {
                    int value = 0;
                    if (int.TryParse(tokens[index], out value))
                    {
                        index++;
                        decl = new ConstantTypeDecl
                        {
                            Value = tokens[index - 1],
                        };
                        return true;
                    }
                }

                string token = null;
                CppParser.Token(tokens, ref index, "typename");
                if (CppParser.Id(tokens, ref index, out token))
                {
                    decl = new RefTypeDecl
                    {
                        Name = token,
                    };

                    if (token != "operator")
                    {
                        while (true)
                        {
                            if (CppParser.Token(tokens, ref index, "<"))
                            {
                                var genericDecl = new GenericTypeDecl
                                {
                                    Element = decl,
                                    TypeArguments = new List<TypeDecl>(),
                                };
                                decl = genericDecl;

                                if (!CppParser.Token(tokens, ref index, ">"))
                                {
                                    while (true)
                                    {
                                        genericDecl.TypeArguments.Add(EnsureTypeWithoutName(tokens, ref index));

                                        if (CppParser.Token(tokens, ref index, ">"))
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            CppParser.EnsureToken(tokens, ref index, ",");
                                        }
                                    }
                                }
                            }
                            else if (CppParser.Token(tokens, ref index, ":"))
                            {
                                CppParser.EnsureToken(tokens, ref index, ":");
                                CppParser.Token(tokens, ref index, "template");
                                if (CppParser.Id(tokens, ref index, out token))
                                {
                                    decl = new SubTypeDecl
                                    {
                                        Parent = decl,
                                        Name = token,
                                    };
                                }
                                else
                                {
                                    index -= 2;
                                    break;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        static TypeDecl EnsureMiniType(string[] tokens, ref int index)
        {
            TypeDecl decl = null;
            if (!ParseMiniType(tokens, ref index, out decl))
            {
                throw new ArgumentException("Failed to parse.");
            }
            return decl;
        }
        static void ParseTypeContinueBeforeName(string[] tokens, ref int index, out TypeDecl decl, out Action<TypeDecl> continuation, out string name)
        {
            decl = null;
            continuation = null;
            name = null;

            while (true)
            {
                Decoration? decoration = null;
                if (CppParser.Token(tokens, ref index, "const"))
                {
                    decoration = Decoration.Const;
                }
                else if (CppParser.Token(tokens, ref index, "volatile"))
                {
                    decoration = Decoration.Volatile;
                }
                else if (CppParser.Token(tokens, ref index, "*"))
                {
                    decoration = Decoration.Pointer;
                }
                else if (CppParser.Token(tokens, ref index, "&"))
                {
                    if (CppParser.Token(tokens, ref index, "&"))
                    {
                        decoration = Decoration.RightRef;
                    }
                    else
                    {
                        decoration = Decoration.LeftRef;
                    }
                }
                else if (CppParser.Token(tokens, ref index, "."))
                {
                    CppParser.EnsureToken(tokens, ref index, ".");
                    CppParser.EnsureToken(tokens, ref index, ".");

                    var vaDecl = new VariadicArgumentTypeDecl
                    {
                    };
                    if (decl == null)
                    {
                        continuation = x => vaDecl.Element = x;
                    }
                    else
                    {
                        vaDecl.Element = decl;
                    }
                    decl = vaDecl;
                }
                else
                {
                    TypeDecl classType = null;
                    if (ParseMiniType(tokens, ref index, out classType))
                    {
                        if (CppParser.Token(tokens, ref index, ":"))
                        {
                            CppParser.EnsureToken(tokens, ref index, ":");
                            var classMemberTypeDecl = new ClassMemberTypeDecl
                            {
                                ClassType = classType,
                            };
                            if (decl == null)
                            {
                                continuation = x => classMemberTypeDecl.Element = x;
                            }
                            else
                            {
                                classMemberTypeDecl.Element = decl;
                            }
                            decl = classMemberTypeDecl;
                        }
                        else
                        {
                            var subType = classType as SubTypeDecl;
                            var refType = classType as RefTypeDecl;
                            if (subType != null)
                            {
                                name = subType.Name;

                                var classMemberTypeDecl = new ClassMemberTypeDecl
                                {
                                    ClassType = subType.Parent,
                                };
                                if (decl == null)
                                {
                                    continuation = x => classMemberTypeDecl.Element = x;
                                }
                                else
                                {
                                    classMemberTypeDecl.Element = decl;
                                }
                                decl = classMemberTypeDecl;
                            }
                            else if (refType != null)
                            {
                                name = refType.Name;
                            }
                            else
                            {
                                throw new ArgumentException("Failed to parse.");
                            }

                            if (name == "operator")
                            {
                                if (tokens[index + 1] == "(")
                                {
                                    name += " " + tokens[index];
                                    index += 1;
                                }
                                else if (tokens[index + 2] == "(")
                                {
                                    name += " " + tokens[index] + tokens[index + 1];
                                    index += 2;
                                }
                                else
                                {
                                    throw new ArgumentException("Failed to parse.");
                                }
                            }
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                if (decoration != null)
                {
                    var decorateDecl = new DecorateTypeDecl
                    {
                        Decoration = decoration.Value,
                    };
                    if (decl == null)
                    {
                        continuation = x => decorateDecl.Element = x;
                    }
                    else
                    {
                        decorateDecl.Element = decl;
                    }
                    decl = decorateDecl;
                }
            }
        }

        internal static void ParseTypeContinueAfterName(string[] tokens, ref int index, ref CallingConvention callingConvention, out TypeDecl decl, out Action<TypeDecl> continuation)
        {
            decl = null;
            continuation = null;

            while (true)
            {
                if (CppParser.Token(tokens, ref index, "["))
                {
                    CppParser.SkipUntil(tokens, ref index, "]");
                    var arrayDecl = new ArrayTypeDecl();
                    if (decl == null)
                    {
                        continuation = x => arrayDecl.Element = x;
                    }
                    else
                    {
                        arrayDecl.Element = decl;
                    }
                    decl = arrayDecl;
                }
                else if (CppParser.Token(tokens, ref index, "("))
                {
                    var funcDecl = new FunctionTypeDecl
                    {
                        Const = false,
                        CallingConvention = callingConvention,
                        Parameters = new List<VarDecl>(),
                    };
                    callingConvention = CallingConvention.Default;

                    if (decl == null)
                    {
                        continuation = x => funcDecl.ReturnType = x;
                    }
                    else
                    {
                        funcDecl.ReturnType = decl;
                    }
                    decl = funcDecl;

                    bool skipParameters = false;
                    if (CppParser.Token(tokens, ref index, "void"))
                    {
                        if (CppParser.Token(tokens, ref index, ")"))
                        {
                            skipParameters = true;
                        }
                        else
                        {
                            index--;
                        }
                    }

                    if (!skipParameters && !CppParser.Token(tokens, ref index, ")"))
                    {
                        while (true)
                        {
                            string name = null;
                            var parameterType = EnsureType(tokens, ref index, out name);

                            funcDecl.Parameters.Add(new VarDecl
                            {
                                Static = false,
                                Name = name,
                                Type = parameterType,
                            });

                            if (CppParser.Token(tokens, ref index, "="))
                            {
                                CppParser.SkipUntilInTemplate(tokens, ref index, ",", ")", ";");
                                index--;
                            }

                            if (CppParser.Token(tokens, ref index, ")"))
                            {
                                break;
                            }
                            CppParser.EnsureToken(tokens, ref index, ",");
                        }
                    }

                    while (true)
                    {
                        if (CppParser.Token(tokens, ref index, "const"))
                        {
                            funcDecl.Const = true;
                        }
                        else if (CppParser.Token(tokens, ref index, "override"))
                        {

                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }

        static void ParseTypeContinue(string[] tokens, ref int index, out CallingConvention callingConvention, out TypeDecl decl, out Action<TypeDecl> continuation, out string name)
        {
            callingConvention = CallingConvention.Default;
            decl = null;
            continuation = null;
            name = null;

            if (CppParser.Token(tokens, ref index, "__cdecl"))
            {
                callingConvention = CallingConvention.CDecl;
            }
            else if (CppParser.Token(tokens, ref index, "__clrcall"))
            {
                callingConvention = CallingConvention.ClrCall;
            }
            else if (CppParser.Token(tokens, ref index, "__stdcall"))
            {
                callingConvention = CallingConvention.StdCall;
            }
            else if (CppParser.Token(tokens, ref index, "__fastcall"))
            {
                callingConvention = CallingConvention.FastCall;
            }
            else if (CppParser.Token(tokens, ref index, "__thiscall"))
            {
                callingConvention = CallingConvention.ThisCall;
            }
            else if (CppParser.Token(tokens, ref index, "__vectorcall"))
            {
                callingConvention = CallingConvention.VectorCall;
            }

            TypeDecl beforeDecl = null;
            Action<TypeDecl> beforeContinuation = null;
            ParseTypeContinueBeforeName(tokens, ref index, out beforeDecl, out beforeContinuation, out name);

            TypeDecl middleDecl = null;
            Action<TypeDecl> middleContinuation = null;
            if (name == null)
            {
                int middleIndex = index;
                bool recursive = false;
                if (CppParser.Token(tokens, ref middleIndex, "("))
                {
                    string token = null;
                    CppParser.SkipUntil(tokens, ref middleIndex, out token, ",", ")");
                    if (token == ")")
                    {
                        recursive = tokens[middleIndex] == "(" || tokens[middleIndex] == "[";
                    }
                }

                if (recursive)
                {
                    CppParser.EnsureToken(tokens, ref index, "(");
                    var middleCallingConvention = CallingConvention.Default;
                    ParseTypeContinue(tokens, ref index, out middleCallingConvention, out middleDecl, out middleContinuation, out name);
                    CppParser.EnsureToken(tokens, ref index, ")");

                    if (middleCallingConvention != CallingConvention.Default)
                    {
                        if (callingConvention == CallingConvention.Default)
                        {
                            callingConvention = middleCallingConvention;
                        }
                        else
                        {
                            throw new ArgumentException("Failed to parse.");
                        }
                    }
                }
            }

            TypeDecl afterDecl = null;
            Action<TypeDecl> afterContinuation = null;
            ParseTypeContinueAfterName(tokens, ref index, ref callingConvention, out afterDecl, out afterContinuation);

            decl = middleDecl;
            continuation = middleContinuation;

            if (afterDecl != null)
            {
                if (decl == null)
                {
                    decl = afterDecl;
                }
                else
                {
                    continuation(afterDecl);
                }
                continuation = afterContinuation;
            }

            if (beforeDecl != null)
            {
                if (decl == null)
                {
                    decl = beforeDecl;
                }
                else
                {
                    continuation(beforeDecl);
                }
                continuation = beforeContinuation;
            }
        }

        public static bool ParseType(string[] tokens, ref int index, out TypeDecl decl, out string name)
        {
            decl = null;
            name = null;
            TypeDecl miniType = null;
            if (ParseMiniType(tokens, ref index, out miniType))
            {
                CallingConvention callingConvention = CallingConvention.Default;
                TypeDecl continuationDecl = null;
                Action<TypeDecl> continuation = null;
                ParseTypeContinue(tokens, ref index, out callingConvention, out continuationDecl, out continuation, out name);
                if (callingConvention != CallingConvention.Default)
                {
                    throw new ArgumentException("Failed to parse.");
                }
                if (continuationDecl != null)
                {
                    continuation(miniType);
                    decl = continuationDecl;
                }
                else
                {
                    decl = miniType;
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        public static TypeDecl EnsureType(string[] tokens, ref int index, out string name)
        {
            TypeDecl decl = null;
            if (!ParseType(tokens, ref index, out decl, out name))
            {
                throw new ArgumentException("Failed to parse.");
            }
            return decl;
        }

        public static TypeDecl EnsureTypeWithoutName(string[] tokens, ref int index)
        {
            string name = null;
            var decl = EnsureType(tokens, ref index, out name);
            if (name != null)
            {
                throw new ArgumentException("Failed to parse.");
            }
            return decl;
        }

        public static TypeDecl EnsureTypeWithName(string[] tokens, ref int index, out string name)
        {
            var decl = EnsureType(tokens, ref index, out name);
            if (name == null)
            {
                throw new ArgumentException("Failed to parse.");
            }
            return decl;
        }
    }
}
