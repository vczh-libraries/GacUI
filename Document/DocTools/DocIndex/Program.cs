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

            var symbols = new Dictionary<string, SymbolDecl[]>();
            foreach (var nss in xmlNss)
            {
                var ns = nss.Key;
                var symbolDecls = nss.Value
                    .Select(xmlSymbol =>
                    {
                        var xmlDecl = xmlSymbol.Elements().First();
                        var decl = SymbolDecl.Deserialize(xmlDecl);
                        decl.BuildSymbolTree(null, null);
                        return decl;
                    })
                    .ToArray();
                symbols.Add(ns, symbolDecls);
            }

            var output = Path.GetFullPath(args[1]);
            if (!output.EndsWith("\\"))
            {
                output += "\\";
            }

            Console.WriteLine("Writing nss.xml ...");
            var nssFileNames = xmlNss
                .Select(p => p.Key)
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
                    nssFileNames
                        .Select(p =>
                            new XElement("Namespace",
                                new XAttribute("FileName", p.Value)
                            )
                        )
                    )
                );
            outputNss.Save(output + "nss.xml");

            Console.WriteLine("Writing ns(*).xml ...");
        }
    }
}
