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
                                var xmlDecl = xmlSymbol.Elements().First();
                                var decl = SymbolDecl.Deserialize(xmlDecl);
                                decl.BuildSymbolTree(null, null);
                                return Tuple.Create(key, decl);
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
            var namespaceFileNames = symbols.Keys
                .ToDictionary(
                    ns => ns,
                    ns => "ns("
                        + ns
                            .Split(new[] { "::" }, StringSplitOptions.RemoveEmptyEntries)
                            .Aggregate("", (a, b) => a == "" ? b : a + "." + b)
                        + ").xml"
                );
            var outputNss = new XDocument(
                new XElement("Namespaces",
                    namespaceFileNames
                        .OrderBy(p => p.Key)
                        .Select(p =>
                            new XElement("Namespace",
                                new XAttribute("Key", p.Key),
                                new XAttribute("DisplayName", (p.Key == "" ? "::" : p.Key.Substring(2)) + " Namespace"),
                                new XAttribute("FileName", p.Value)
                            )
                        )
                    )
                );
            outputNss.Save(output + "nss.xml");

            Console.WriteLine("Writing ns(*).xml ...");
            var symbolFileNames = symbols
                .SelectMany(x => x.Value.Select(t => t.Item1))
                .ToDictionary(
                    x => x,
                    x => x
                        .Replace("*", "[ptr]")
                        .Replace("?", "[q]")
                        .Replace("<", "[lt]")
                        .Replace(">", "[gt]")
                        .Replace(":", "[colon]")
                );
            foreach (var nsp in symbols)
            {
                var outputNs = new XDocument(
                    new XElement("Namespace",
                        new XAttribute("DisplayName", nsp.Key),
                        nsp.Value
                            .GroupBy(t => GetDisplayName(t.Item2))
                            .OrderBy(g => g.Key)
                            .Select(g => new XElement("Overloads",
                                new XAttribute("DisplayName", g.Key),
                                g
                                    .Select(t => new XElement("Symbol",
                                        new XAttribute("Key", t.Item1),
                                        new XAttribute("FileName", symbolFileNames[t.Item1])
                                        )
                                    )
                                )
                            )
                        )
                    );
                outputNs.Save(output + namespaceFileNames[nsp.Key]);
            }
        }

        static string GetDisplayName(SymbolDecl decl)
        {
            return "";
        }
    }
}
