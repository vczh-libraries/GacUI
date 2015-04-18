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
        public Dictionary<string, Dictionary<string, List<SymbolDecl>>> NamespaceContents { get; set; }
        public Dictionary<SymbolDecl, Dictionary<string, List<SymbolDecl>>> SymbolContents { get; set; }
        public Dictionary<TypeDecl, List<SymbolDecl>> ResolvedTypes { get; set; }

        private void FillNamespaceReferences(SymbolDecl decl)
        {
            var ns = decl as NamespaceDecl;
            if (ns != null)
            {
                this.NamespaceReferences.Add(ns, new List<string> { ns.NameKey });
                if (decl.Children != null)
                {
                    foreach (var child in decl.Children)
                    {
                        FillNamespaceReferences(child);
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

        private void FillNamespaceContents()
        {
            var nsg = this.NamespaceReferences.Keys
                .GroupBy(x => x.NameKey)
                .ToArray();
            foreach (var nss in nsg)
            {
                var children = nss
                    .Where(x => x.Children != null)
                    .SelectMany(x => x.Children)
                    .Select(x =>
                        {
                            var template = x as TemplateDecl;
                            return template == null ? x : template.Element;
                        })
                    .Where(x => x.Name != null)
                    .GroupBy(x => x.Name)
                    .ToDictionary(x => x.Key, x => x.GroupBy(y => y.OverloadKey + ";" + y.Tags).Select(y => y.First()).ToList())
                    ;
                this.NamespaceContents.Add(nss.Key, children);
            }
        }

        public ResolveEnvironment(IEnumerable<SymbolDecl> globals)
        {
            this.Errors = new List<string>();
            this.NamespaceReferences = new Dictionary<NamespaceDecl, List<string>>();
            this.NamespaceContents = new Dictionary<string, Dictionary<string, List<SymbolDecl>>>();
            this.SymbolContents = new Dictionary<SymbolDecl, Dictionary<string, List<SymbolDecl>>>();
            this.ResolvedTypes = new Dictionary<TypeDecl, List<SymbolDecl>>();

            foreach (var global in globals.SelectMany(x => x.Children))
            {
                FillNamespaceReferences(global);
            }
            ResolveUsingNamespaces();
            FillNamespaceContents();
        }

        public Dictionary<string, List<SymbolDecl>> GetSymbolContent(SymbolDecl symbol)
        {
            if (symbol is NamespaceDecl)
            {
                return this.NamespaceContents[symbol.NameKey];
            }
            else
            {
                Dictionary<string, List<SymbolDecl>> content = null;
                if (!this.SymbolContents.TryGetValue(symbol, out content))
                {
                    if (symbol is GlobalDecl)
                    {
                        content = this.NamespaceReferences.Keys
                            .Where(x => x.Parent is GlobalDecl)
                            .GroupBy(x => x.Name)
                            .ToDictionary(x => x.Key, x => x.Cast<SymbolDecl>().ToList())
                            ;
                    }
                    else
                    {
                        var visitor = new ResolveSymbolDeclContentVisitor
                        {
                            Environment = this,
                        };
                        symbol.Accept(visitor);
                        content = visitor.Content;
                    }
                    this.SymbolContents.Add(symbol, content);
                }
                return content;
            }
        }
    }

    class ResolveTypeDeclVisitor : TypeDecl.IVisitor
    {
        public SymbolDecl Symbol { get; set; }
        public ResolveEnvironment Environment { get; set; }

        private List<SymbolDecl> FindSymbolInContent(TypeDecl decl, string name, Dictionary<string, List<SymbolDecl>> content)
        {
            if (content == null)
            {
                return null;
            }

            List<SymbolDecl> decls = null;
            if (content.TryGetValue(name, out decls))
            {
                var keys = decls.Select(x => x.NameKey).Distinct().ToArray();
                if (keys.Length > 1)
                {
                    var printingKeys = decls.Select(x => x.OverloadKey).Distinct().Aggregate("", (a, b) => a + "\r\n" + b);
                    this.Environment.Errors.Add(string.Format("Found multiple symbols for {0} in {1}: {2}", name, this.Symbol.OverloadKey, printingKeys));
                    return null;
                }
                decl.ReferencingNameKey = keys[0];
                return decls;
            }
            return null;
        }

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
                case "void":
                    return;
            }

            var current = this.Symbol;
            while (current != null)
            {
                var ns = current as NamespaceDecl;
                if (ns == null)
                {
                    var content = this.Environment.GetSymbolContent(current);
                    var decls = FindSymbolInContent(decl, decl.Name, content);
                    if (decls != null)
                    {
                        this.Environment.ResolvedTypes.Add(decl, decls);
                        return;
                    }
                }
                else
                {
                    var references = this.Environment.NamespaceReferences[ns];
                    foreach (var reference in references)
                    {
                        var content = this.Environment.NamespaceContents[reference];
                        var decls = FindSymbolInContent(decl, decl.Name, content);
                        if (decls != null)
                        {
                            this.Environment.ResolvedTypes.Add(decl, decls);
                            return;
                        }
                    }
                }
                current = current.Parent;
            }
            this.Environment.Errors.Add(string.Format("Failed to resolve {0} in {1}.", decl.Name, this.Symbol.OverloadKey));
        }

        public void Visit(SubTypeDecl decl)
        {
            decl.Parent.Resolve(this.Symbol, this.Environment);

            var parent = decl.Parent;
            while (parent != null)
            {
                var generic = parent as GenericTypeDecl;
                if (generic != null)
                {
                    parent = generic.Element;
                    continue;
                }
                break;
            }

            if (parent.ReferencingNameKey != null)
            {
                Dictionary<string, List<SymbolDecl>> content = null;
                if (!this.Environment.NamespaceContents.TryGetValue(parent.ReferencingNameKey, out content))
                {
                    var parentDecls = this.Environment.ResolvedTypes[parent];
                    content = parentDecls
                        .Select(x => this.Environment.GetSymbolContent(x))
                        .Where(x => x != null)
                        .SelectMany(x => x)
                        .GroupBy(x => x.Key)
                        .ToDictionary(x => x.Key, x => x.SelectMany(y => y.Value).ToList())
                        ;
                }
                var decls = FindSymbolInContent(decl, decl.Name, content);
                if (decls != null)
                {
                    this.Environment.ResolvedTypes.Add(decl, decls);
                    return;
                }
            }
            this.Environment.Errors.Add(string.Format("Failed to resolve {0} in {1}.", decl.Name, this.Symbol.OverloadKey));
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
                if (type.Parent == null)
                {
                    type.Type.Resolve(this.Symbol, this.Environment);
                }
                else
                {
                    type.Resolve(this.Environment);
                }
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

        public void Visit(TypeParameterDecl decl)
        {
        }

        public void Visit(TemplateDecl decl)
        {
            foreach (var type in decl.Specialization)
            {
                type.Resolve(decl, this.Environment);
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
                baseType.Type.Resolve(decl.Parent, this.Environment);
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

    class ResolveSymbolDeclContentVisitor : SymbolDecl.IVisitor
    {
        public ResolveEnvironment Environment { get; set; }
        public Dictionary<string, List<SymbolDecl>> Content { get; set; }

        private void AddSymbol(string key, SymbolDecl symbol)
        {
            if (this.Content == null)
            {
                this.Content = new Dictionary<string, List<SymbolDecl>>();
            }

            List<SymbolDecl> decls = null;
            if (!this.Content.TryGetValue(key, out decls))
            {
                decls = new List<SymbolDecl>();
                this.Content.Add(key, decls);
            }
            if (!decls.Contains(symbol))
            {
                decls.Add(symbol);
            }
        }

        public void Visit(GlobalDecl decl)
        {
        }

        public void Visit(NamespaceDecl decl)
        {
        }

        public void Visit(UsingNamespaceDecl decl)
        {
        }

        public void Visit(TypeParameterDecl decl)
        {
        }

        public void Visit(TemplateDecl decl)
        {
            foreach (var item in decl.TypeParameters)
            {
                AddSymbol(item.Name, item);
            }
        }

        public void Visit(BaseTypeDecl decl)
        {
        }

        public void Visit(ClassDecl decl)
        {
            if (decl.Children != null)
            {
                foreach (var item in decl.Children)
                {
                    if (item.Name != null)
                    {
                        var template = item as TemplateDecl;
                        var func = (template == null ? template : item) as FuncDecl;
                        if (func == null || (func.Function != Function.Constructor && func.Function != Function.Destructor))
                        {
                            AddSymbol(item.Name, item);
                        }
                    }
                    else
                    {
                        item.Accept(this);
                    }
                }
            }

            var keys = this.Content == null ? null : new HashSet<string>(this.Content.Keys);
            foreach (var baseType in decl.BaseTypes)
            {
                baseType.Type.Resolve(decl.Parent, this.Environment);
                var parent = baseType.Type;
                while (parent != null)
                {
                    var generic = parent as GenericTypeDecl;
                    if (generic != null)
                    {
                        parent = generic.Element;
                        continue;
                    }
                    break;
                }

                if (parent.ReferencingNameKey != null)
                {
                    var symbols = this.Environment.ResolvedTypes[parent];
                    foreach (var symbol in symbols)
                    {
                        if (symbol == decl)
                        {
                            break;
                        }

                        var content = this.Environment.GetSymbolContent(symbol);
                        if (content != null)
                        {
                            foreach (var item in content.Where(p => keys == null || !keys.Contains(p.Key)).SelectMany(x => x.Value))
                            {
                                if (item.Access != Access.Private)
                                {
                                    var template = item as TemplateDecl;
                                    var func = (template == null ? template : item) as FuncDecl;
                                    if (func == null || (func.Function != Function.Constructor && func.Function != Function.Destructor))
                                    {
                                        AddSymbol(item.Name, item);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        public void Visit(VarDecl decl)
        {
        }

        public void Visit(FuncDecl decl)
        {
            if (decl.Children != null)
            {
                foreach (VarDecl item in decl.Children)
                {
                    if (item.Name != null)
                    {
                        AddSymbol(item.Name, item);
                    }
                }
            }
        }

        public void Visit(GroupedFieldDecl decl)
        {
            if (decl.Children != null)
            {
                foreach (var item in decl.Children)
                {
                    item.Accept(this);
                }
            }
        }

        public void Visit(EnumItemDecl decl)
        {
        }

        public void Visit(EnumDecl decl)
        {
            foreach (EnumItemDecl item in decl.Children)
            {
                AddSymbol(item.Name, item);
            }
        }

        public void Visit(TypedefDecl decl)
        {
        }
    }
}
