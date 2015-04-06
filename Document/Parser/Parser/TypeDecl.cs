using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    class TypeDecl
    {
        public static bool ParseMiniType(string[] tokens, ref int index, out TypeDecl decl)
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
                            while (true)
                            {
                                if (Parser.Token(tokens, ref index, ">"))
                                {
                                    break;
                                }
                                else if (Parser.Token(tokens, ref index, ","))
                                {
                                    TypeDecl typeArgument = null;
                                    string name = null;
                                    if (!ParseType(tokens, ref index, out typeArgument, out name))
                                    {
                                        throw new ArgumentException("Failed to parse");
                                    }
                                    genericDecl.TypeArguments.Add(typeArgument);
                                }
                                else
                                {
                                    throw new ArgumentException("Failed to parse.");
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

        public static void ParseTypeContinueBeforeName(string[] tokens, ref int index, out TypeDecl decl, out Action<TypeDecl> continuation, out string name)
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

        public static bool ParseTypeContinueAfterName(string[] tokens, ref int index, out TypeDecl decl, out Action<TypeDecl> continuation)
        {
            throw new NotImplementedException();
        }

        public static bool ParseType(string[] tokens, ref int index, out TypeDecl decl, out string name)
        {
            throw new NotImplementedException();
        }
    }

    class RefTypeDecl : TypeDecl
    {
        public string Name { get; set; }
    }

    class SubTypeDecl : TypeDecl
    {
        public TypeDecl Parent { get; set; }
        public string Name { get; set; }
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
    }

    class ArrayTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public int? Size { get; set; }
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
    }

    class ClassMemberTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public TypeDecl ClassType { get; set; }
    }

    class GenericTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public List<TypeDecl> TypeArguments { get; set; }
    }
}
