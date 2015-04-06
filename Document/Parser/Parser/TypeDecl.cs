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
                TypeDecl element = null;
                if (!ParseMiniType(tokens, ref index, out element))
                {
                    throw new ArgumentException("Failed to parse.");
                }
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Const,
                    Element = element,
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "volatile"))
            {
                TypeDecl element = null;
                if (!ParseMiniType(tokens, ref index, out element))
                {
                    throw new ArgumentException("Failed to parse.");
                }
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Volatile,
                    Element = element,
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "signed"))
            {
                TypeDecl element = null;
                if (!ParseMiniType(tokens, ref index, out element))
                {
                    throw new ArgumentException("Failed to parse.");
                }
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Signed,
                    Element = element,
                };
                return true;
            }
            else if (Parser.Token(tokens, ref index, "unsigned"))
            {
                TypeDecl element = null;
                if (!ParseMiniType(tokens, ref index, out element))
                {
                    throw new ArgumentException("Failed to parse.");
                }
                decl = new DecorateTypeDecl
                {
                    Decoration = Decoration.Unsigned,
                    Element = element,
                };
                return true;
            }
            else
            {
                string token = null;
                if (Parser.Id(tokens, ref index, out token))
                {
                    decl = new RefTypeDecl
                    {
                        Name = token,
                    };

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
                                    TypeDecl typeArgument = null;
                                    string name = null;
                                    if (!ParseType(tokens, ref index, out typeArgument, out name))
                                    {
                                        throw new ArgumentException("Failed to parse");
                                    }
                                    genericDecl.TypeArguments.Add(typeArgument);

                                    if (Parser.Token(tokens, ref index, ">"))
                                    {
                                        break;
                                    }
                                    else if (!Parser.Token(tokens, ref index, ","))
                                    {
                                        throw new ArgumentException("Failed to parse.");
                                    }
                                }
                            }
                        }
                        else if (Parser.Token(tokens, ref index, ":"))
                        {
                            if (!Parser.Token(tokens, ref index, ":"))
                            {
                                throw new ArgumentException("Failed to parse.");
                            }
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

                    return true;
                }
                else
                {
                    return false;
                }
            }
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
                else
                {
                    TypeDecl classType = null;
                    if (ParseMiniType(tokens, ref index, out classType))
                    {
                        if (Parser.Token(tokens, ref index, ":"))
                        {
                            if (Parser.Token(tokens, ref index, ":"))
                            {
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
                                throw new ArgumentException("Failed to parse.");
                            }
                        }
                        else
                        {
                            name = ((RefTypeDecl)classType).Name;
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

        static void ParseTypeContinueAfterName(string[] tokens, ref int index, CallingConvention callingConvention, out TypeDecl decl, out Action<TypeDecl> continuation)
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

                    if (!Parser.Token(tokens, ref index, ")"))
                    {
                        TypeDecl parameterType = null;
                        string name = null;
                        if (!ParseType(tokens, ref index, out parameterType, out name))
                        {
                            throw new ArgumentException("Failed to parse.");
                        }

                        funcDecl.Parameters.Add(new VarDecl
                        {
                            Name = name,
                            Type = parameterType,
                        });
                        if (Parser.Token(tokens, ref index, ")"))
                        {
                            break;
                        }
                        if (!Parser.Token(tokens, ref index, ","))
                        {
                            throw new ArgumentException("Failed to parse.");
                        }
                    }
                    funcDecl.Const = Parser.Token(tokens, ref index, "const");
                }
            }
        }

        static void ParseTypeContinue(string[] tokens, ref int index, out TypeDecl decl, out Action<TypeDecl> continuation, out string name)
        {
            decl = null;
            continuation = null;
            name = null;
            CallingConvention callingConvention = CallingConvention.Default;

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
                    if (!Parser.Token(tokens, ref index, "("))
                    {
                        throw new ArgumentException("Failed to parse.");
                    }
                    ParseTypeContinue(tokens, ref index, out middleDecl, out middleContinuation, out name);
                    if (!Parser.Token(tokens, ref index, ")"))
                    {
                        throw new ArgumentException("Failed to parse.");
                    }
                }
            }

            TypeDecl afterDecl = null;
            Action<TypeDecl> afterContinuation = null;
            ParseTypeContinueAfterName(tokens, ref index, callingConvention, out afterDecl, out afterContinuation);

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
                TypeDecl continuationDecl = null;
                Action<TypeDecl> continuation = null;
                ParseTypeContinue(tokens, ref index, out continuationDecl, out continuation, out name);
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
}
