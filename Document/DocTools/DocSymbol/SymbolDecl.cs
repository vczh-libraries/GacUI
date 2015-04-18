using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocSymbol
{
    public enum Access
    {
        Public,
        Protected,
        Private,
    }

    public abstract class SymbolDecl
    {
        public interface IVisitor
        {
            void Visit(GlobalDecl decl);
            void Visit(NamespaceDecl decl);
            void Visit(UsingNamespaceDecl decl);
            void Visit(TypeParameterDecl decl);
            void Visit(TemplateDecl decl);
            void Visit(BaseTypeDecl decl);
            void Visit(ClassDecl decl);
            void Visit(VarDecl decl);
            void Visit(FuncDecl decl);
            void Visit(GroupedFieldDecl decl);
            void Visit(EnumItemDecl decl);
            void Visit(EnumDecl decl);
            void Visit(TypedefDecl decl);
        }

        public Access Access { get; set; }
        public string Name { get; set; }
        public List<SymbolDecl> Children { get; set; }
        public string Document { get; set; }

        #region BuildSymbolTree generated fields

        public SymbolDecl Parent { get; set; }
        public string Tags { get; set; }
        public string NameKey { get; set; }
        public string OverloadKey { get; set; }
        public string ContentKey { get; set; }

        #endregion

        public abstract void Accept(IVisitor visitor);

        public SymbolDecl()
        {
            this.Access = global::DocSymbol.Access.Public;
        }

        public override string ToString()
        {
            var visitor = new PrintSymbolDeclVisitor();
            Accept(visitor);
            return visitor.Result;
        }

        public XElement Serialize()
        {
            var visitor = new SerializeSymbolDeclVisitor();
            visitor.Element = new XElement(GetType().Name);
            Accept(visitor);
            return visitor.Element;
        }

        public static SymbolDecl Deserialize(XElement element)
        {
            var typeName = "DocSymbol." + element.Name;
            var type = typeof(SymbolDecl).Assembly.GetType(typeName);
            var symbolDecl = (SymbolDecl)type.GetConstructor(Type.EmptyTypes).Invoke(null);
            var visitor = new DeserializeSymbolDeclVisitor();
            visitor.Element = element;
            symbolDecl.Accept(visitor);
            return symbolDecl;
        }

        public void Resolve(ResolveEnvironment environment)
        {
            var visitor = new ResolveSymbolDeclVisitor();
            visitor.Environment = environment;
            Accept(visitor);
        }

        #region BuildSymbolTree

        internal string NameKeyOfScopeParent
        {
            get
            {
                var decl = this.Parent;
                if (decl != null)
                {
                    decl = decl.ScopeParent;
                }
                return decl == null ? "" : decl.NameKey;
            }
        }

        internal string OverloadKeyOfScopeParent
        {
            get
            {
                var decl = this.Parent;
                if (decl != null)
                {
                    decl = decl.ScopeParent;
                }
                return decl == null ? "" : decl.OverloadKey;
            }
        }

        internal virtual SymbolDecl ScopeParent
        {
            get
            {
                return this;
            }
        }

        internal string GenerateNameKey()
        {
            var visitor = new GenerateSymbolDeclNameKeyVisitor();
            Accept(visitor);
            return visitor.Result;
        }

        internal string GenerateOverloadKey()
        {
            var visitor = new GenerateSymbolDeclOverrideKeyVisitor();
            Accept(visitor);
            return visitor.Result == null ? GenerateNameKey() : visitor.Result;
        }

        internal string GenerateContentKey()
        {
            var visitor = new GenerateSymbolDeclContentKeyVisitor();
            Accept(visitor);
            return visitor.Result;
        }

        internal string GenerateChildContentKey()
        {
            var visitor = new GenerateSymbolDeclChildContentKeyVisitor();
            Accept(visitor);
            return visitor.Result;
        }

        public virtual void BuildSymbolTree(SymbolDecl parent, string tag)
        {
            this.Parent = parent;
            if (tag != null)
            {
                this.Tags = tag;
            }
            if (this.NameKey == null)
            {
                this.NameKey = GenerateNameKey();
            }
            if (this.OverloadKey == null)
            {
                this.OverloadKey = GenerateOverloadKey();
            }
            this.ContentKey = (this.Document == null ? "" : this.Document) + GenerateContentKey();
            if (this.Children != null)
            {
                foreach (var decl in this.Children)
                {
                    decl.BuildSymbolTree(this, tag);
                }
            }
        }

        #endregion
    }

    public class GlobalDecl : SymbolDecl
    {
        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class NamespaceDecl : SymbolDecl
    {
        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class UsingNamespaceDecl : SymbolDecl
    {
        public List<string> Path { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class TypeParameterDecl : SymbolDecl
    {
        public override void Accept(IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class TemplateDecl : SymbolDecl
    {
        public List<TypeParameterDecl> TypeParameters { get; set; }
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

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        internal override SymbolDecl ScopeParent
        {
            get
            {
                return this.Parent == null ? null : this.Parent.ScopeParent;
            }
        }

        public override void BuildSymbolTree(SymbolDecl parent, string tag)
        {
            base.BuildSymbolTree(parent, tag);
            foreach (var type in this.TypeParameters)
            {
                type.Parent = this;
                type.BuildSymbolTree(this, tag);
            }
        }
    }

    public enum ClassType
    {
        Class,
        Struct,
        Union,
    }

    public class BaseTypeDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class ClassDecl : SymbolDecl
    {
        public ClassType ClassType { get; set; }
        public List<BaseTypeDecl> BaseTypes { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        public override void BuildSymbolTree(SymbolDecl parent, string tag)
        {
            base.BuildSymbolTree(parent, tag);
            foreach (var type in this.BaseTypes)
            {
                type.Parent = this;
                type.BuildSymbolTree(this, tag);
            }
        }
    }

    public class VarDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
        public bool Static { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public enum Virtual
    {
        Static,
        Normal,
        Virtual,
        Abstract,
    }

    public enum Function
    {
        Constructor,
        Destructor,
        Function,
    }

    public class FuncDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }
        public Virtual Virtual { get; set; }
        public Function Function { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        public override void BuildSymbolTree(SymbolDecl parent, string tag)
        {
            foreach (var decl in ((FunctionTypeDecl)this.Type).Parameters)
            {
                if (decl.Name != "")
                {
                    if (this.Children == null)
                    {
                        this.Children = new List<SymbolDecl>();
                    }
                    this.Children.Add(decl);
                }
            }
            base.BuildSymbolTree(parent, tag);
        }
    }

    public enum Grouping
    {
        Union,
        Struct,
    }

    public class GroupedFieldDecl : SymbolDecl
    {
        public Grouping Grouping { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        internal override SymbolDecl ScopeParent
        {
            get
            {
                return this.Parent == null ? null : this.Parent.ScopeParent;
            }
        }
    }

    public class EnumItemDecl : SymbolDecl
    {
        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class EnumDecl : SymbolDecl
    {
        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }

    public class TypedefDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }
    }
}
