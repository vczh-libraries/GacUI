using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocSymbol
{
    public class TypeDecl
    {
    }

    public class RefTypeDecl : TypeDecl
    {
        public string Name { get; set; }

        public override string ToString()
        {
            return this.Name;
        }
    }

    public class SubTypeDecl : TypeDecl
    {
        public TypeDecl Parent { get; set; }
        public string Name { get; set; }

        public override string ToString()
        {
            return this.Parent.ToString() + "::" + this.Name;
        }
    }

    public enum Decoration
    {
        Const,
        Volatile,
        Pointer,
        LeftRef,
        RightRef,
        Signed,
        Unsigned,
    }

    public class DecorateTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public Decoration Decoration { get; set; }

        public override string ToString()
        {
            switch (this.Decoration)
            {
                case global::DocSymbol.Decoration.Const:
                    return "const " + this.Element.ToString();
                case global::DocSymbol.Decoration.Volatile:
                    return "volatile " + this.Element.ToString();
                case global::DocSymbol.Decoration.Pointer:
                    return "* " + this.Element.ToString();
                case global::DocSymbol.Decoration.LeftRef:
                    return "& " + this.Element.ToString();
                case global::DocSymbol.Decoration.RightRef:
                    return "&& " + this.Element.ToString();
                case global::DocSymbol.Decoration.Signed:
                    return "signed " + this.Element.ToString();
                case global::DocSymbol.Decoration.Unsigned:
                    return "unsigned " + this.Element.ToString();
                default:
                    throw new NotSupportedException();
            }
        }
    }

    public class ArrayTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }

        public override string ToString()
        {
            return "array " + this.Element.ToString();
        }
    }

    public enum CallingConvention
    {
        Default,
        CDecl,
        ClrCall,
        StdCall,
        FastCall,
        ThisCall,
        VectorCall,
    }
    public class FunctionTypeDecl : TypeDecl
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
                case global::DocSymbol.CallingConvention.CDecl:
                    result += "__cdecl ";
                    break;
                case global::DocSymbol.CallingConvention.ClrCall:
                    result += "__clrcall ";
                    break;
                case global::DocSymbol.CallingConvention.StdCall:
                    result += "__stdcall ";
                    break;
                case global::DocSymbol.CallingConvention.FastCall:
                    result += "__fastcall ";
                    break;
                case global::DocSymbol.CallingConvention.ThisCall:
                    result += "__thiscall ";
                    break;
                case global::DocSymbol.CallingConvention.VectorCall:
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

    public class ClassMemberTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public TypeDecl ClassType { get; set; }

        public override string ToString()
        {
            return this.ClassType.ToString() + "::(" + this.Element.ToString() + ")";
        }
    }

    public class GenericTypeDecl : TypeDecl
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

    public class DeclTypeDecl : TypeDecl
    {
        public string Expression { get; set; }

        public override string ToString()
        {
            return "decltype( " + this.Expression + " )";
        }
    }

    public class VariadicArgumentTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }

        public override string ToString()
        {
            return "... " + this.Element.ToString();
        }
    }

    public class ConstantTypeDecl : TypeDecl
    {
        public string Value { get; set; }

        public override string ToString()
        {
            return "<" + this.Value + ">";
        }
    }
}
