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
            var errors = environment.Errors.Where(x => x.StartsWith("(Error)")).ToArray();
            var warnings = environment.Errors.Where(x => x.StartsWith("(Warning)")).ToArray();
            Console.WriteLine("Errors: " + errors.Length);
            Console.WriteLine("Warnings: " + warnings.Length);

            using (var writer = new StreamWriter(output + ".errors.txt", false, Encoding.UTF8))
            {
                writer.WriteLine("=========================ERROR=========================");
                foreach (var message in errors)
                {
                    writer.WriteLine(message);
                }

                writer.WriteLine();
                writer.WriteLine("========================WARNING========================");
                foreach (var message in warnings)
                {
                    writer.WriteLine(message);
                }
            }

            Console.WriteLine("Saving ...");
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
