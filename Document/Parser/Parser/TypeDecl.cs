using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    public class TypeDecl
    {
        static bool ParseMiniType(string[] tokens, ref int index, out TypeDecl decl)
        {
            decl = null;
            if (Parser.Token(tokens, ref index, "const"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Const,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "volatile"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Volatile,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "signed"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Signed,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "unsigned"))
            {
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Unsigned,
                    Element = EnsureMiniType(tokens, ref index),
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "decltype"))
            {
                Parser.EnsureToken(tokens, ref index, "(");
                int oldIndex = index;
                Parser.SkipUntil(tokens, ref index, ")");

                decl = new DeclTypeDecl
                {
                    Expression = tokens
                        .Skip(oldIndex)
                        .Take(index - 1 - oldIndex)
                        .Aggregate((a, b) => a + " " + b),
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "false") || Parser.Token(tokens, ref index, "true"))
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
                Parser.Token(tokens, ref index, "typename");
                if (Parser.Id(tokens, ref index, out token))
                {
                    decl = new RefTypeDecl
                    {
                        Name = token,
                    };

                    if (token != "operator")
                    {
                        while (true)
                        {
                            if (Parser.Token(tokens, ref index, "<"))
                            {
                                var genericDecl = new GenericTypeDecl
                                {
                                    Element = decl,
                                    TypeArguments = new List<TypeDecl>(),
                                };
                                decl = genericDecl;

                                if (!Parser.Token(tokens, ref index, ">"))
                                {
                                    while (true)
                                    {
                                        genericDecl.TypeArguments.Add(EnsureTypeWithoutName(tokens, ref index));

                                        if (Parser.Token(tokens, ref index, ">"))
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            Parser.EnsureToken(tokens, ref index, ",");
                                        }
                                    }
                                }
                            }
                            else if (Parser.Token(tokens, ref index, ":"))
                            {
                                Parser.EnsureToken(tokens, ref index, ":");
                                Parser.Token(tokens, ref index, "template");
                                if (Parser.Id(tokens, ref index, out token))
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
                if (Parser.Token(tokens, ref index, "const"))
                {
                    decoration = Decoration.Const;
                }
                else if (Parser.Token(tokens, ref index, "volatile"))
                {
                    decoration = Decoration.Volatile;
                }
                else if (Parser.Token(tokens, ref index, "*"))
                {
                    decoration = Decoration.Pointer;
                }
                else if (Parser.Token(tokens, ref index, "&"))
                {
                    decoration = Decoration.LeftRef;
                }
                else if (Parser.Token(tokens, ref index, "&&"))
                {
                    decoration = Decoration.RightRef;
                }
                else if (Parser.Token(tokens, ref index, "."))
                {
                    Parser.EnsureToken(tokens, ref index, ".");
                    Parser.EnsureToken(tokens, ref index, ".");

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
                        if (Parser.Token(tokens, ref index, ":"))
                        {
                            Parser.EnsureToken(tokens, ref index, ":");
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
                if (Parser.Token(tokens, ref index, "["))
                {
                    Parser.SkipUntil(tokens, ref index, "]");
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
                else if (Parser.Token(tokens, ref index, "("))
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
                    if (Parser.Token(tokens, ref index, "void"))
                    {
                        if (Parser.Token(tokens, ref index, ")"))
                        {
                            skipParameters = true;
                        }
                        else
                        {
                            index--;
                        }
                    }

                    if (!skipParameters && !Parser.Token(tokens, ref index, ")"))
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

                            if (Parser.Token(tokens, ref index, "="))
                            {
                                Parser.SkipUntilInTemplate(tokens, ref index, ",", ")", ";");
                                index--;
                            }

                            if (Parser.Token(tokens, ref index, ")"))
                            {
                                break;
                            }
                            Parser.EnsureToken(tokens, ref index, ",");
                        }
                    }

                    while (true)
                    {
                        if (Parser.Token(tokens, ref index, "const"))
                        {
                            funcDecl.Const = true;
                        }
                        else if (Parser.Token(tokens, ref index, "override"))
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

            if (Parser.Token(tokens, ref index, "__cdecl"))
            {
                callingConvention = CallingConvention.CDecl;
            }
            else if (Parser.Token(tokens, ref index, "__clrcall"))
            {
                callingConvention = CallingConvention.ClrCall;
            }
            else if (Parser.Token(tokens, ref index, "__stdcall"))
            {
                callingConvention = CallingConvention.StdCall;
            }
            else if (Parser.Token(tokens, ref index, "__fastcall"))
            {
                callingConvention = CallingConvention.FastCall;
            }
            else if (Parser.Token(tokens, ref index, "__thiscall"))
            {
                callingConvention = CallingConvention.ThisCall;
            }
            else if (Parser.Token(tokens, ref index, "__vectorcall"))
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
                if (Parser.Token(tokens, ref middleIndex, "("))
                {
                    string token = null;
                    Parser.SkipUntil(tokens, ref middleIndex, out token, ",", ")");
                    if (token == ")")
                    {
                        recursive = tokens[middleIndex] == "(" || tokens[middleIndex] == "[";
                    }
                }

                if (recursive)
                {
                    Parser.EnsureToken(tokens, ref index, "(");
                    var middleCallingConvention = CallingConvention.Default;
                    ParseTypeContinue(tokens, ref index, out middleCallingConvention, out middleDecl, out middleContinuation, out name);
                    Parser.EnsureToken(tokens, ref index, ")");

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

    class RefTypeDecl : TypeDecl
    {
        public string Name { get; set; }

        public override string ToString()
        {
            return this.Name;
        }
    }

    class SubTypeDecl : TypeDecl
    {
        public TypeDecl Parent { get; set; }
        public string Name { get; set; }

        public override string ToString()
        {
            return this.Parent.ToString() + "::" + this.Name;
        }
    }

    enum Decoration
    {
        Const,
        Volatile,
        Pointer,
        LeftRef,
        RightRef,
        Signed,
        Unsigned,
    }

    class DecorateTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public Decoration Decoration { get; set; }

        public override string ToString()
        {
            switch (this.Decoration)
            {
                case global::Parser.Decoration.Const:
                    return "const " + this.Element.ToString();
                case global::Parser.Decoration.Volatile:
                    return "volatile " + this.Element.ToString();
                case global::Parser.Decoration.Pointer:
                    return "* " + this.Element.ToString();
                case global::Parser.Decoration.LeftRef:
                    return "& " + this.Element.ToString();
                case global::Parser.Decoration.RightRef:
                    return "&& " + this.Element.ToString();
                case global::Parser.Decoration.Signed:
                    return "signed " + this.Element.ToString();
                case global::Parser.Decoration.Unsigned:
                    return "unsigned " + this.Element.ToString();
                default:
                    throw new NotSupportedException();
            }
        }
    }

    class ArrayTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }

        public override string ToString()
        {
            return "array " + this.Element.ToString();
        }
    }

    enum CallingConvention
    {
        Default,
        CDecl,
        ClrCall,
        StdCall,
        FastCall,
        ThisCall,
        VectorCall,
    }
    class FunctionTypeDecl : TypeDecl
    {
        public CallingConvention CallingConvention { get; set; }
        public TypeDecl ReturnType { get; set; }
        public List<VarDecl> Parameters { get; set; }
        public bool Const { get; set; }

        public override string ToString()
        {
            string result = "function ";
            switch (this.CallingConvention)
            {
                case global::Parser.CallingConvention.CDecl:
                    result += "__cdecl ";
                    break;
                case global::Parser.CallingConvention.ClrCall:
                    result += "__clrcall ";
                    break;
                case global::Parser.CallingConvention.StdCall:
                    result += "__stdcall ";
                    break;
                case global::Parser.CallingConvention.FastCall:
                    result += "__fastcall ";
                    break;
                case global::Parser.CallingConvention.ThisCall:
                    result += "__thiscall ";
                    break;
                case global::Parser.CallingConvention.VectorCall:
                    result += "__vectorcall ";
                    break;
            }
            if (this.Const)
            {
                result += "const ";
            }

            result += "(";
            for (int i = 0; i < this.Parameters.Count; i++)
            {
                if (i > 0)
                {
                    result += ", ";
                }
                if (this.Parameters[i].Name != null)
                {
                    result += this.Parameters[i].Name + " : ";
                }
                result += this.Parameters[i].Type.ToString();
            }

            result += ") : " + this.ReturnType.ToString();
            return result;
        }
    }

    class ClassMemberTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public TypeDecl ClassType { get; set; }

        public override string ToString()
        {
            return this.ClassType.ToString() + "::(" + this.Element.ToString() + ")";
        }
    }

    class GenericTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public List<TypeDecl> TypeArguments { get; set; }

        public override string ToString()
        {
            string result = this.Element.ToString();
            result += "<";
            for (int i = 0; i < this.TypeArguments.Count; i++)
            {
                if (i > 0)
                {
                    result += ", ";
                }
                result += this.TypeArguments[i].ToString();
            }
            result += ">";
            return result;
        }
    }

    class DeclTypeDecl : TypeDecl
    {
        public string Expression { get; set; }

        public override string ToString()
        {
            return "decltype( " + this.Expression + " )";
        }
    }

    class VariadicArgumentTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }

        public override string ToString()
        {
            return "... " + this.Element.ToString();
        }
    }

    class ConstantTypeDecl : TypeDecl
    {
        public string Value { get; set; }

        public override string ToString()
        {
            return "<" + this.Value + ">";
        }
    }
}
