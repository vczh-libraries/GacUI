using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DocSymbol
{
    public class ResolveEnvironment
    {
        public List<string> Errors { get; set; }
        public Dictionary<NamespaceDecl, List<string>> NamespaceReferences { get; set; }
        public Dictionary<string, List<SymbolDecl>> NamespaceContents { get; set; }

        private void FillNamespaces(SymbolDecl decl)
        {
            var ns = decl as NamespaceDecl;
            if (ns != null)
            {
                this.NamespaceReferences.Add(ns, new List<string> { ns.NameKey });
                if (decl.Children != null)
                {
                    foreach (var child in decl.Children)
                    {
                        FillNamespaces(child);
                    }
                }
            }
        }

        private void ResolveUsingNamespaces()
        {
            var allns = new HashSet<string>(this.NamespaceReferences.Values.SelectMany(x => x).Distinct());
            foreach (var nsp in this.NamespaceReferences)
            {
                var ns = nsp.Key;
                var nss = new List<string>();
                {
                    var current = ns;
                    while (current != null)
                    {
                        nss.Add(current.NameKey);
                        current = current.Parent as NamespaceDecl;
                    }
                }

                foreach (var uns in ns.Children.Where(x => x is UsingNamespaceDecl).Cast<UsingNamespaceDecl>())
                {
                    string path = uns.Path.Aggregate("", (a, b) => a + "::" + b);
                    var resolved = nss
                        .Select(x => x + path)
                        .Where(x => allns.Contains(x))
                        .ToArray();
                    if (resolved.Length != 1)
                    {
                        Errors.Add(string.Format("Failed to resolve {0} in {1}.", uns.ToString(), ns.ToString()));
                    }
                    nsp.Value.AddRange(resolved);
                }
                nsp.Value.AddRange(nss.Skip(1));
            }
        }

        public ResolveEnvironment(IEnumerable<SymbolDecl> globals)
        {
            this.Errors = new List<string>();
            this.NamespaceReferences = new Dictionary<NamespaceDecl, List<string>>();
            this.NamespaceContents = new Dictionary<string, List<SymbolDecl>>();

            foreach (var global in globals.SelectMany(x => x.Children))
            {
                FillNamespaces(global);
            }
            ResolveUsingNamespaces();
        }
    }

    class ResolveTypeDeclVisitor : TypeDecl.IVisitor
    {
        public SymbolDecl Symbol { get; set; }
        public ResolveEnvironment Environment { get; set; }
        public string Result { get; set; }

        public void Visit(RefTypeDecl decl)
        {
            switch (decl.Name)
            {
                case "__int8":
                case "__int16":
                case "__int32":
                case "__int64":
                case "char":
                case "wchar_t":
                case "bool":
                case "float":
                case "double":
                    return;
            }

            throw new NotImplementedException();
        }

        public void Visit(SubTypeDecl decl)
        {
            decl.Parent.Resolve(this.Symbol, this.Environment);
            throw new NotImplementedException();
        }

        public void Visit(DecorateTypeDecl decl)
        {
            decl.Element.Resolve(this.Symbol, this.Environment);
        }

        public void Visit(ArrayTypeDecl decl)
        {
            decl.Element.Resolve(this.Symbol, this.Environment);
        }

        public void Visit(FunctionTypeDecl decl)
        {
            decl.ReturnType.Resolve(this.Symbol, this.Environment);
            foreach (var type in decl.Parameters)
            {
                type.Resolve(this.Environment);
            }
        }

        public void Visit(ClassMemberTypeDecl decl)
        {
            decl.Element.Resolve(this.Symbol, this.Environment);
            decl.ClassType.Resolve(this.Symbol, this.Environment);
        }

        public void Visit(GenericTypeDecl decl)
        {
            decl.Element.Resolve(this.Symbol, this.Environment);
            foreach (var type in decl.TypeArguments)
            {
                type.Resolve(this.Symbol, this.Environment);
            }
        }

        public void Visit(DeclTypeDecl decl)
        {
        }

        public void Visit(VariadicArgumentTypeDecl decl)
        {
            decl.Element.Resolve(this.Symbol, this.Environment);
        }

        public void Visit(ConstantTypeDecl decl)
        {
        }
    }

    class ResolveSymbolDeclVisitor : SymbolDecl.IVisitor
    {
        public ResolveEnvironment Environment { get; set; }

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TemplateDecl decl)
        {
            if (decl.Specialization != null)
            {
                foreach (var type in decl.Specialization)
                {
                    type.Resolve(decl, this.Environment);
                }
            }
        }

        public void Visit(BaseTypeDecl decl)
        {
        }

        public void Visit(ClassDecl decl)
        {
            var templateDecl = decl.Parent as TemplateDecl;
            if (templateDecl != null)
            {
                Visit(templateDecl);
            }

            foreach (var baseType in decl.BaseTypes)
            {
                baseType.Type.Resolve(decl, this.Environment);
            }
        }

        public void Visit(VarDecl decl)
        {
            decl.Type.Resolve(decl, this.Environment);
        }

        public void Visit(FuncDecl decl)
        {
            var templateDecl = decl.Parent as TemplateDecl;
            if (templateDecl != null)
            {
                Visit(templateDecl);
            }

            decl.Type.Resolve(decl, this.Environment);
        }

        public void Visit(GroupedFieldDecl decl)
        {
        }

        public void Visit(EnumItemDecl decl)
        {
        }

        public void Visit(EnumDecl decl)
        {
        }

        public void Visit(TypedefDecl decl)
        {
            var templateDecl = decl.Parent as TemplateDecl;
            if (templateDecl != null)
            {
                Visit(templateDecl);
            }

            decl.Type.Resolve(decl, this.Environment);
        }
    }
}
