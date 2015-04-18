using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocSymbol
{
    public abstract class TypeDecl
    {
        public interface IVisitor
        {
            void Visit(RefTypeDecl decl);
            void Visit(SubTypeDecl decl);
            void Visit(DecorateTypeDecl decl);
            void Visit(ArrayTypeDecl decl);
            void Visit(FunctionTypeDecl decl);
            void Visit(ClassMemberTypeDecl decl);
            void Visit(GenericTypeDecl decl);
            void Visit(DeclTypeDecl decl);
            void Visit(VariadicArgumentTypeDecl decl);
            void Visit(ConstantTypeDecl decl);
        }

        public string ReferencingNameKey { get; set; }
        public List<string> ReferencingOverloadKeys { get; set; }

        public abstract void Accept(IVisitor visitor);

        public override string ToString()
        {
            var visitor = new PrintTypeDeclVisitor();
            Accept(visitor);
            return visitor.Result;
        }

        public XElement Serialize()
        {
            var visitor = new SerializeTypeDeclVisitor();
            visitor.Element = new XElement(GetType().Name);
            Accept(visitor);
            return visitor.Element;
        }

        public static TypeDecl Deserialize(XElement element)
        {
            var typeName = "DocSymbol." + element.Name;
            var type = typeof(TypeDecl).Assembly.GetType(typeName);
            var typeDecl = (TypeDecl)type.GetConstructor(Type.EmptyTypes).Invoke(null);
            var visitor = new DeserializeTypeDeclVisitor();
            visitor.Element = element;
            typeDecl.Accept(visitor);
            return typeDecl;
        }

        public void Resolve(SymbolDecl symbol, ResolveEnvironment environment)
        {
            if (this.ReferencingNameKey == null)
            {
                var visitor = new ResolveTypeDeclVisitor();
                visitor.Symbol = symbol;
                visitor.Environment = environment;
                Accept(visitor);
            }
        }
    }

    public class RefTypeDecl : TypeDecl
    {
        public string Name { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class SubTypeDecl : TypeDecl
    {
        public TypeDecl Parent { get; set; }
        public string Name { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
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

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class ArrayTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public string Expression { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
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

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class ClassMemberTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public TypeDecl ClassType { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class GenericTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }
        public List<TypeDecl> TypeArguments { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class DeclTypeDecl : TypeDecl
    {
        public string Expression { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class VariadicArgumentTypeDecl : TypeDecl
    {
        public TypeDecl Element { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class ConstantTypeDecl : TypeDecl
    {
        public string Value { get; set; }

        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
