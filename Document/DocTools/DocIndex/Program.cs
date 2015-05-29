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
                                new XAttribute("DisplayName", (p.Key == "" ? "::" : p.Key.Substring(2)) + " Namespace"),
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
        }

        static bool ContainsDocument(SymbolDecl decl)
        {
            return decl.Document != null || (decl.Children != null && decl.Children.Any(ContainsDocument));
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
    }
}
