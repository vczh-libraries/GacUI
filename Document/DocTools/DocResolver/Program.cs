using DocSymbol;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocResolver
{
    class Program
    {
        static void Main(string[] args)
        {
            var input = new Dictionary<string, GlobalDecl>();
            for (int i = 0; i < args.Length / 2; i++)
            {
                var tag = args[i * 2];
                var fileName = args[i * 2 + 1];
                Console.WriteLine("Reading " + fileName + " ...");
                var xml = XDocument.Load(fileName);
                var decl = (GlobalDecl)SymbolDecl.Deserialize(xml.Root);
                decl.BuildSymbolTree(null, tag);
                input.Add(tag, decl);
            }

            Console.WriteLine("De-duplicating ...");
            var symbolGroup = new Dictionary<string, List<SymbolDecl>>();
            GroupSymbolsByOverloadKey(input.Values, symbolGroup);
            foreach (var pair in symbolGroup)
            {
                var groups = pair.Value
                    .GroupBy(x => x.ContentKey)
                    .ToArray();
                foreach (var group in groups)
                {
                    var decls = group.ToArray();
                    var tags = decls
                        .Select(x => x.Tags)
                        .OrderBy(x => x)
                        .Aggregate((a, b) => a + ";" + b);
                    foreach (var decl in decls)
                    {
                        decl.Tags = tags;
                    }
                }
            }

            Console.WriteLine("Resolving Symbols ...");
            var symbolResolving = symbolGroup
                .SelectMany(x => x.Value)
                .ToArray();
            var environment = new ResolveEnvironment(input.Values);
            foreach (var symbol in symbolResolving)
            {
                symbol.Resolve(environment);
            }

            var output = args[args.Length - 1];
            var xmlErrors = environment.Errors.Where(x => x.StartsWith("(Xml)")).ToArray();
            var warnings = environment.Errors.Where(x => x.StartsWith("(Warning)")).ToArray();
            var errors = environment.Errors.Where(x => x.StartsWith("(Error)")).ToArray();
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine("Xml Errors: " + xmlErrors.Length);
            Console.WriteLine("Warnings: " + warnings.Length);
            Console.WriteLine("Errors: " + errors.Length);
            Console.ResetColor();

            using (var writer = new StreamWriter(output + ".errors.txt", false, Encoding.UTF8))
            {
                writer.WriteLine("=======================XML ERROR=======================");
                foreach (var message in xmlErrors)
                {
                    writer.WriteLine(message);
                }
                writer.WriteLine();

                writer.WriteLine("========================WARNING========================");
                foreach (var message in warnings)
                {
                    writer.WriteLine(message);
                }

                writer.WriteLine();
                writer.WriteLine("=========================ERROR=========================");
                foreach (var message in errors)
                {
                    writer.WriteLine(message);
                }
            }

            Console.WriteLine("Saving ...");
            var symbolSaving = symbolGroup
                .ToDictionary(
                    x => x.Key,
                    x => x.Value
                        .GroupBy(y => y.Tags)
                        .Select(y => y.First())
                        .Select(y =>
                        {
                            var templateDecl = y.Parent as TemplateDecl;
                            return templateDecl == null ? y : templateDecl;
                        })
                        .Where(y => y.Parent is NamespaceDecl || y.Parent is GlobalDecl)
                        .ToArray()
                    )
                .Where(x => x.Value.Length > 0)
                .GroupBy(x => x.Value.First().Parent)
                .ToDictionary(
                    x => x.Key,
                    x => x.ToDictionary(
                        y => y.Key,
                        y => y.Value
                        )
                    )
                ;
            var outputXml = new XDocument(
                new XElement("CppXmlDocument",
                    symbolSaving.Select(x => new XElement("Namespace",
                        new XAttribute("Name", x.Key is GlobalDecl ? "" : (x.Key as NamespaceDecl).NameKey),
                        x.Value.Select(y => new XElement("Symbol",
                            new XAttribute("OverloadKey", y.Key),
                            y.Value
                                .Select(z => z.Parent is TemplateDecl ? z.Parent : z)
                                .Select(z => z.Serialize())
                            ))
                        ))
                    )
                );
            outputXml.Save(output);
        }

        static void GroupSymbolsByOverloadKey(IEnumerable<SymbolDecl> decls, Dictionary<string, List<SymbolDecl>> group)
        {
            foreach (var decl in decls)
            {
                if (!(decl is NamespaceDecl) && decl.OverloadKey != null)
                {
                    List<SymbolDecl> values = null;
                    if (!group.TryGetValue(decl.OverloadKey, out values))
                    {
                        values = new List<SymbolDecl>();
                        group.Add(decl.OverloadKey, values);
                    }
                    values.Add(decl);
                }
                if (decl.Children != null)
                {
                    GroupSymbolsByOverloadKey(decl.Children, group);
                }
            }
        }
    }
}
