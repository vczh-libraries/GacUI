using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocSymbol
{
    class PrintTypeDeclVisitor : TypeDecl.IVisitor
    {
        public string Result { get; set; }

        public void Visit(RefTypeDecl decl)
        {
            this.Result = decl.Name;
        }

        public void Visit(SubTypeDecl decl)
        {
            this.Result = decl.Parent.ToString() + "::" + decl.Name;
        }

        public void Visit(DecorateTypeDecl decl)
        {
            switch (decl.Decoration)
            {
                case global::DocSymbol.Decoration.Const:
                    this.Result = "const " + decl.Element.ToString();
                    break;
                case global::DocSymbol.Decoration.Volatile:
                    this.Result = "volatile " + decl.Element.ToString();
                    break;
                case global::DocSymbol.Decoration.Pointer:
                    this.Result = "* " + decl.Element.ToString();
                    break;
                case global::DocSymbol.Decoration.LeftRef:
                    this.Result = "& " + decl.Element.ToString();
                    break;
                case global::DocSymbol.Decoration.RightRef:
                    this.Result = "&& " + decl.Element.ToString();
                    break;
                case global::DocSymbol.Decoration.Signed:
                    this.Result = "signed " + decl.Element.ToString();
                    break;
                case global::DocSymbol.Decoration.Unsigned:
                    this.Result = "unsigned " + decl.Element.ToString();
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        public void Visit(ArrayTypeDecl decl)
        {
            this.Result = "array " + decl.Element.ToString();
        }

        public void Visit(FunctionTypeDecl decl)
        {
            string result = "function ";
            switch (decl.CallingConvention)
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
            if (decl.Const)
            {
                result += "const ";
            }

            result += "(";
            for (int i = 0; i < decl.Parameters.Count; i++)
            {
                if (i > 0)
                {
                    result += ", ";
                }
                if (decl.Parameters[i].Name != null)
                {
                    result += decl.Parameters[i].Name + " : ";
                }
                result += decl.Parameters[i].Type.ToString();
            }

            result += ") : " + decl.ReturnType.ToString();
            this.Result = result;
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            this.Result = decl.ClassType.ToString() + "::(" + decl.Element.ToString() + ")";
        }

        public void Visit(GenericTypeDecl decl)
        {
            string result = decl.Element.ToString();
            result += "<";
            for (int i = 0; i < decl.TypeArguments.Count; i++)
            {
                if (i > 0)
                {
                    result += ", ";
                }
                result += decl.TypeArguments[i].ToString();
            }
            result += ">";
            this.Result = result;
        }

        public void Visit(DeclTypeDecl decl)
        {
            this.Result = "decltype( " + decl.Expression + " )";
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            this.Result = "... " + decl.Element.ToString();
        }

        public void Visit(ConstantTypeDecl decl)
        {
            this.Result = "<" + decl.Value + ">";
        }
    }
}
