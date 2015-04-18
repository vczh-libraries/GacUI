using DocSymbol;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace DocParser
{
    class Program
    {
        static IEnumerable<SymbolDecl> ExpandChildren(SymbolDecl decl)
        {
            yield return decl;
            if (decl.Children != null)
            {
                foreach (var child in decl.Children.SelectMany(ExpandChildren))
                {
                    yield return child;
                }
            }
        }

        static void Main(string[] args)
        {
            var tokens = File.ReadAllLines(args[0]);
            var global = new GlobalDecl();
            {
                int index = 0;
                CppDeclParser.ParseSymbols(tokens, ref index, global);
                if (index != tokens.Length)
                {
                    throw new ArgumentException("Failed to parse.");
                }
            }

            var xml = new XDocument(global.Serialize());
            xml.Save(args[1]);

            global.BuildSymbolTree(null, null);
            var grouping = ExpandChildren(global)
                .Where(decl => decl.OverloadKey != null)
                .GroupBy(decl => decl.OverloadKey)
                .ToDictionary(g => g.Key, g => g.ToArray())
                ;
            foreach (var pair in grouping)
            {
                if (pair.Value.Length > 1 && pair.Value.Any(decl => !(decl is NamespaceDecl)))
                {
                    Console.WriteLine("Duplicate key founds: " + pair.Key);
                }
            }
        }
    }
}
