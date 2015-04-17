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

        protected static string GetKeyOfScopeParent(SymbolDecl decl)
        {
            if (decl != null)
            {
                decl = decl.GetScopeParent();
            }
            return decl == null ? "" : decl.OverloadKey;
        }

        internal virtual string GenerateNameKey()
        {
            return null;
        }

        internal virtual string GenerateOverloadKey()
        {
            return GenerateNameKey();
        }

        internal virtual SymbolDecl GetScopeParent()
        {
            return this;
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

        internal override string GenerateNameKey()
        {
            return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
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

        internal override SymbolDecl GetScopeParent()
        {
            return this.Parent == null ? null : this.Parent.GetScopeParent();
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

        internal override string GenerateNameKey()
        {
            var template = this.Parent as TemplateDecl;
            if (template == null)
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
            }
            else
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name + "`" + template.TypeParameters.Count.ToString();
            }
        }

        internal override string GenerateOverloadKey()
        {
            var template = this.Parent as TemplateDecl;
            if (template == null)
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
            }
            else
            {
                var postfix = "<" + template.Specialization.Aggregate("", (a, b) => a == "" ? b.ToString() : a + "," + b.ToString()) + ">";
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name + "`" + template.TypeParameters.Count.ToString() + (postfix == "<>" ? "" : postfix);
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

        internal override string GenerateNameKey()
        {
            return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
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

        internal override string GenerateNameKey()
        {
            var template = this.Parent as TemplateDecl;
            if (template == null)
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
            }
            else
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name + "`" + template.TypeParameters.Count.ToString();
            }
        }

        internal override string GenerateOverloadKey()
        {
            var func = (FunctionTypeDecl)this.Type;
            var postfix = "(" + func.Parameters.Aggregate("", (a, b) => a == "" ? b.Type.ToString() : a + "," + b.Type.ToString()) + ")" + (func.Const ? "const" : "");
            var template = this.Parent as TemplateDecl;
            if (template == null)
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name + postfix;
            }
            else
            {
                return GetKeyOfScopeParent(this.Parent) + "::" + this.Name + "`" + template.TypeParameters.Count.ToString() + postfix;
            }
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

        internal override SymbolDecl GetScopeParent()
        {
            return this.Parent == null ? null : this.Parent.GetScopeParent();
        }
    }

    public class EnumItemDecl : SymbolDecl
    {
        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        internal override string GenerateNameKey()
        {
            return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
        }
    }

    public class EnumDecl : SymbolDecl
    {
        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        internal override string GenerateNameKey()
        {
            return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
        }
    }

    public class TypedefDecl : SymbolDecl
    {
        public TypeDecl Type { get; set; }

        public override void Accept(SymbolDecl.IVisitor visitor)
        {
            visitor.Visit(this);
        }

        internal override string GenerateNameKey()
        {
            return GetKeyOfScopeParent(this.Parent) + "::" + this.Name;
        }
    }
}
