using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
        public Access Access { get; set; }
        public string Name { get; set; }
        public List<SymbolDecl> Children { get; set; }
        public SymbolDecl Parent { get; set; }
        public string NameKey { get; set; }
        public string OverloadKey { get; set; }
        public string Document { get; set; }

        public interface IVisitor
        {
            void Visit(GlobalDesc decl);
            void Visit(NamespaceDecl decl);
            void Visit(UsingNamespaceDecl decl);
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

        internal string KeyOfScopeParent
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

        public virtual void BuildSymbolTree(SymbolDecl parent = null)
        {
            this.Parent = parent;
            this.NameKey = GenerateNameKey();
            this.OverloadKey = GenerateOverloadKey();
            if (this.Children != null)
            {
                foreach (var decl in this.Children)
                {
                    decl.BuildSymbolTree(this);
                }
            }
        }
    }

    public class GlobalDesc : SymbolDecl
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

    public class TemplateDecl : SymbolDecl
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

        public override void BuildSymbolTree(SymbolDecl parent)
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
            base.BuildSymbolTree(parent);
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
