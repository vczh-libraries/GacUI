using DocSymbol;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocIndex
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Reading " + args[0] + " ...");
            var xml = XDocument.Load(args[0]);
            var xmlNss = xml.Root
                .Elements("Namespace")
                .GroupBy(xmlNs => xmlNs.Attribute("Name").Value)
                .ToDictionary(
                    g => g.Key,
                    g => g.SelectMany(xmlNs => xmlNs.Elements("Symbol")).ToArray()
                );

            var symbols = xml.Root
                .Elements("Namespace")
                .GroupBy(xmlNs => xmlNs.Attribute("Name").Value)
                .ToDictionary(
                    g => g.Key,
                    g =>
                    {
                        return g
                            .SelectMany(xmlNs => xmlNs.Elements("Symbol"))
                            .Select(xmlSymbol =>
                            {
                                var key = xmlSymbol.Attribute("OverloadKey").Value;
                                var symbolDecls = xmlSymbol
                                    .Elements()
                                    .Select(xmlDecl =>
                                    {
                                        var decl = SymbolDecl.Deserialize(xmlDecl);
                                        decl.BuildSymbolTree(null, null);
                                        return decl;
                                    })
                                    .ToArray();
                                return Tuple.Create(key, symbolDecls);
                            })
                            .ToArray();
                    }
                );

            var output = Path.GetFullPath(args[1]);
            if (!output.EndsWith("\\"))
            {
                output += "\\";
            }

            Console.WriteLine("Writing nss.xml ...");
            var namespaceNames = symbols.Keys
                .ToDictionary(
                    ns => ns,
                    ns => ns
                            .Split(new[] { "::" }, StringSplitOptions.RemoveEmptyEntries)
                            .Aggregate("", (a, b) => a == "" ? b : a + "." + b)
                );
            var outputNss = new XDocument(
                new XElement("Namespaces",
                    namespaceNames
                        .OrderBy(p => p.Key)
                        .Select(p =>
                            new XElement("Namespace",
                                new XAttribute("Key", p.Key),
                                new XAttribute("DisplayName", (p.Key == "" ? "::" : p.Key.Substring(2)) + " namespace"),
                                new XAttribute("UrlName", p.Value),
                                new XAttribute("Doc", symbols[p.Key].SelectMany(t => t.Item2.Select(s => s)).Any(ContainsDocument))
                            )
                        )
                    )
                );
            outputNss.Save(output + "nss.xml");

            Console.WriteLine("Writing ns(*).xml ...");
            var symbolNames = symbols
                .SelectMany(x => x.Value.Select(t => t.Item1))
                .ToDictionary(
                    x => x,
                    x => x
                        .Replace("::", ".")
                        .Replace(":", "#")
                        .Replace("|", "$")
                        .Replace("/", "%")
                        .Replace("*", "^")
                        .Replace("<", "{")
                        .Replace(">", "}")
                        .Substring(1)
                );
            foreach (var nsp in symbols)
            {
                var outputNs = new XDocument(
                    new XElement("Namespace",
                        new XAttribute("DisplayName", nsp.Key),
                        nsp.Value
                            .GroupBy(t => GetDisplayName(t.Item2[0]))
                            .OrderBy(g => g.Key)
                            .Select(g => new XElement("Overloads",
                                new XAttribute("DisplayName", g.Key),
                                g
                                    .Select(t => new XElement("Symbol",
                                        new XAttribute("UrlName", symbolNames[t.Item1]),
                                        new XAttribute("Doc", t.Item2.Any(ContainsDocument))
                                        )
                                    )
                                )
                            )
                        )
                    );
                outputNs.Save(output + "ns(" + namespaceNames[nsp.Key] + ").xml");
            }

            Console.WriteLine("Writing t(*).xml ...");
            var symbolFileMapping = new Dictionary<string, string>();
            foreach (var nsp in symbols)
            {
                Console.WriteLine("Processing namespace: " + nsp.Key);
                foreach (var st in nsp.Value)
                {
                    var urlName = symbolNames[st.Item1];
                    var outputSymbol = CreateSymbolTree(namespaceNames[nsp.Key], urlName, st.Item2, symbolFileMapping);
                    try
                    {
                        outputSymbol.Save(output + "t(" + urlName + ").xml");
                    }
                    catch (PathTooLongException)
                    {
                        Console.WriteLine("Error: File path is too long: \"" + urlName + "\".");
                    }
                }
            }

            Console.WriteLine("Writing s(*).xml ...");
            foreach (var nsp in symbols)
            {
                Console.WriteLine("Processing namespace: " + nsp.Key);
                foreach (var st in nsp.Value)
                {
                    var urlName = symbolNames[st.Item1];
                    foreach (var decl in st.Item2)
                    {
                        FixSymbolLinks(decl, symbolFileMapping);
                    }
                    var outputSymbol = new XDocument(
                        new XElement("Symbols",
                            st.Item2.Select(decl => decl.Serialize())
                            )
                        );
                    try
                    {
                        outputSymbol.Save(output + "s(" + urlName + ").xml");
                    }
                    catch (PathTooLongException)
                    {
                        Console.WriteLine("Error: File path is too long: \"" + urlName + "\".");
                    }
                }
            }
        }

        static bool ContainsDocument(SymbolDecl decl)
        {
            var docDecl = decl;
            if (decl.Parent is TemplateDecl)
            {
                docDecl = decl.Parent;
            }
            return docDecl.Document != null || (decl.Children != null && decl.Children.Any(ContainsDocument));
        }

        static string GetDisplayName(SymbolDecl decl)
        {
            {
                var templateDecl = decl as TemplateDecl;
                if (templateDecl != null)
                {
                    decl = templateDecl.Element;
                }
            }

            if (decl is ClassDecl)
            {
                return decl.Name + " class";
            }
            else if (decl is EnumDecl)
            {
                return decl.Name + " enum";
            }
            else if (decl is FuncDecl)
            {
                return decl.Name + " function";
            }
            else if (decl is VarDecl)
            {
                return decl.Name + " field";
            }
            else if (decl is TypedefDecl)
            {
                return decl.Name + " typedecl";
            }
            else
            {
                throw new ArgumentException();
            }
        }

        #region FixSymbolLinks

        static void FixSymbolLinks(SymbolDecl decl, Dictionary<string, string> symbolFileMapping)
        {
            FixSymbolDeclVisitor.Execute(decl, symbolFileMapping);
        }

        class FixTypeDeclVisitor : TypeDecl.IVisitor
        {
            public Dictionary<string, string> SymbolFileMapping { get; set; }

            private void Execute(TypeDecl decl)
            {
                decl.Accept(this);
            }

            private void Fix(TypeDecl decl)
            {
                if (decl.ReferencingOverloadKeys != null)
                {
                    for (int i = 0; i < decl.ReferencingOverloadKeys.Count; i++)
                    {
                        var key = decl.ReferencingOverloadKeys[i];
                        decl.ReferencingOverloadKeys[i] = key + "@" + this.SymbolFileMapping[key];
                    }
                }
            }

            void TypeDecl.IVisitor.Visit(RefTypeDecl decl)
            {
                Fix(decl);
            }

            void TypeDecl.IVisitor.Visit(SubTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.Parent);
            }

            void TypeDecl.IVisitor.Visit(DecorateTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.Element);
            }

            void TypeDecl.IVisitor.Visit(ArrayTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.Element);
            }

            void TypeDecl.IVisitor.Visit(FunctionTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.ReturnType);
                foreach (var paremter in decl.Parameters)
                {
                    Execute(paremter.Type);
                }
            }

            void TypeDecl.IVisitor.Visit(ClassMemberTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.Element);
                Execute(decl.Element);
            }

            void TypeDecl.IVisitor.Visit(GenericTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.Element);
                foreach (var paremter in decl.TypeArguments)
                {
                    Execute(paremter);
                }
            }

            void TypeDecl.IVisitor.Visit(DeclTypeDecl decl)
            {
                Fix(decl);
            }

            void TypeDecl.IVisitor.Visit(VariadicArgumentTypeDecl decl)
            {
                Fix(decl);
                Execute(decl.Element);
            }

            void TypeDecl.IVisitor.Visit(ConstantTypeDecl decl)
            {
                Fix(decl);
            }
        }

        class FixSymbolDeclVisitor : SymbolDecl.IVisitor
        {
            private FixTypeDeclVisitor fixTypeDeclVisitor;

            public Dictionary<string, string> SymbolFileMapping { get; set; }

            public static void Execute(SymbolDecl decl, Dictionary<string, string> symbolFileMapping)
            {
                var visitor = new FixSymbolDeclVisitor
                {
                    SymbolFileMapping = symbolFileMapping,
                    fixTypeDeclVisitor = new FixTypeDeclVisitor
                    {
                        SymbolFileMapping = symbolFileMapping,
                    },
                };
                decl.Accept(visitor);
            }

            private void Execute(TypeDecl decl)
            {
                decl.Accept(this.fixTypeDeclVisitor);
            }

            private void Fix(SymbolDecl decl, bool skipChildren = false)
            {
                if (!skipChildren && decl.Children != null)
                {
                    foreach (var subDecl in decl.Children)
                    {
                        subDecl.Accept(this);
                    }
                }
            }

            void SymbolDecl.IVisitor.Visit(GlobalDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(NamespaceDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(UsingNamespaceDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(TypeParameterDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(TemplateDecl decl)
            {
                Fix(decl);
                foreach (var spec in decl.Specialization)
                {
                    Execute(spec);
                }
            }

            void SymbolDecl.IVisitor.Visit(BaseTypeDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(ClassDecl decl)
            {
                Fix(decl);
                foreach (var baseType in decl.BaseTypes)
                {
                    Execute(baseType.Type);
                }
            }

            void SymbolDecl.IVisitor.Visit(VarDecl decl)
            {
                Fix(decl);
                Execute(decl.Type);
            }

            void SymbolDecl.IVisitor.Visit(FuncDecl decl)
            {
                Fix(decl, true);
                Execute(decl.Type);
            }

            void SymbolDecl.IVisitor.Visit(GroupedFieldDecl decl)
            {
                Fix(decl);
            }

            void SymbolDecl.IVisitor.Visit(EnumItemDecl decl)
            {
                Fix(decl);
            }

            void SymbolDecl.IVisitor.Visit(EnumDecl decl)
            {
                Fix(decl);
            }

            void SymbolDecl.IVisitor.Visit(TypedefDecl decl)
            {
                Fix(decl);
                Execute(decl.Type);
            }
        }

        #endregion

        #region CreateSymbolTree

        class SymbolTree
        {
            public string Tags { get; set; }
            public string Key { get; set; }
            public string DisplayName { get; set; }
            public bool Doc { get; set; }
            public List<SymbolTree> Children { get; set; }

            public XElement Serialize()
            {
                return new XElement("Symbol",
                    new XAttribute("Tags", this.Tags),
                    new XAttribute("Key", this.Key),
                    new XAttribute("DisplayName", this.DisplayName),
                    new XAttribute("Doc", this.Doc),
                    this.Children.Select(tree => tree.Serialize())
                    );
            }

            public static SymbolTree Merge(SymbolTree[] forest)
            {
                var result = new SymbolTree();
                result.Key = forest[0].Key;
                result.DisplayName = forest[0].DisplayName;
                result.Doc = forest.Any(tree => tree.Doc);

                result.Tags = forest
                    .SelectMany(tree => tree.Tags.Split(new[] { ";" }, StringSplitOptions.RemoveEmptyEntries))
                    .Distinct()
                    .OrderBy(t => t)
                    .Aggregate((a, b) => a + ";" + b);

                result.Children = forest
                    .Where(tree => tree.Children != null)
                    .SelectMany(tree => tree.Children)
                    .GroupBy(tree => tree.Key)
                    .Select(g => Merge(g.ToArray()))
                    .ToList();
                return result;
            }
        }

        class GenerateSymbolTreeVisitor : SymbolDecl.IVisitor
        {
            public SymbolTree Parent { get; set; }
            public Dictionary<string, string> SymbolParentMapping { get; set; }
            public SymbolTree Result { get; set; }

            private static SymbolTree ExecuteInternal(SymbolDecl decl, SymbolTree parent, Dictionary<string, string> symbolParentMapping)
            {
                var visitor = new GenerateSymbolTreeVisitor
                {
                    Parent = parent,
                    SymbolParentMapping = symbolParentMapping,
                };
                decl.Accept(visitor);
                return visitor.Result;
            }

            public static SymbolTree Execute(SymbolDecl decl, Dictionary<string, string> symbolParentMapping)
            {
                var parent = new SymbolTree();
                ExecuteInternal(decl, parent, symbolParentMapping);
                if (parent.Children == null || parent.Children.Count != 1)
                {
                    throw new ArgumentException();
                }
                return parent.Children[0];
            }
            private void GenerateChildren(SymbolDecl decl, SymbolTree parent)
            {
                if (decl.Children != null)
                {
                    foreach (var subDecl in decl.Children)
                    {
                        ExecuteInternal(subDecl, parent, this.SymbolParentMapping);
                    }
                }
            }

            private void MapKey(string child, string parent)
            {
                string mapping = null;
                if (this.SymbolParentMapping.TryGetValue(child, out mapping))
                {
                    if (mapping != parent)
                    {
                        throw new ArgumentException();
                    }
                }
                else
                {
                    this.SymbolParentMapping.Add(child, parent);
                }
            }

            private void EntryDecl(SymbolDecl decl)
            {
                if (decl.OverloadKey == null)
                {
                    throw new ArgumentException();
                }

                MapKey(decl.OverloadKey, decl.OverloadKey);

                this.Result = new SymbolTree
                {
                    Tags = decl.Tags,
                    Key = decl.OverloadKey,
                    DisplayName = GetDisplayName(decl),
                    Doc = ContainsDocument(decl),
                };
                if (this.Parent != null)
                {
                    if (this.Parent.Children == null)
                    {
                        this.Parent.Children = new List<SymbolTree>();
                    }
                    this.Parent.Children.Add(this.Result);
                }
                GenerateChildren(decl, this.Result);
            }

            private void NoEntryDecl(SymbolDecl decl)
            {
                if (decl.Document != null && !(decl is TemplateDecl))
                {
                    throw new ArgumentException();
                }
                if (decl.OverloadKey != null)
                {
                    MapKey(decl.OverloadKey, this.Parent.Key);
                }
                GenerateChildren(decl, this.Parent);
            }

            void SymbolDecl.IVisitor.Visit(GlobalDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(NamespaceDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(UsingNamespaceDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(TypeParameterDecl decl)
            {
                NoEntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(TemplateDecl decl)
            {
                NoEntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(BaseTypeDecl decl)
            {
                throw new ArgumentOutOfRangeException();
            }

            void SymbolDecl.IVisitor.Visit(ClassDecl decl)
            {
                EntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(VarDecl decl)
            {
                EntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(FuncDecl decl)
            {
                EntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(GroupedFieldDecl decl)
            {
                NoEntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(EnumItemDecl decl)
            {
                NoEntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(EnumDecl decl)
            {
                EntryDecl(decl);
            }

            void SymbolDecl.IVisitor.Visit(TypedefDecl decl)
            {
                EntryDecl(decl);
            }
        }

        static XDocument CreateSymbolTree(string nsUrlName, string urlName, SymbolDecl[] decls, Dictionary<string, string> symbolFileMapping)
        {
            var symbolParentMapping = new Dictionary<string, string>();
            var forest = decls
                .Select(decl => GenerateSymbolTreeVisitor.Execute(decl, symbolParentMapping))
                .ToArray();
            var root = SymbolTree.Merge(forest);

            foreach (var key in symbolParentMapping.Keys)
            {
                symbolFileMapping.Add(key, urlName);
            }

            return new XDocument(
                new XElement("SymbolTree",
                    new XAttribute("NamespaceUrlName", nsUrlName),
                    new XElement("SymbolParentMapping",
                        symbolParentMapping
                            .Where(p => p.Key != p.Value)
                            .Select(p =>
                                new XElement("Map",
                                    new XAttribute("From", p.Key),
                                    new XAttribute("To", p.Value)
                                    )
                                )
                        ),
                    root.Serialize()
                    )
                );
        }

        #endregion
    }
}
