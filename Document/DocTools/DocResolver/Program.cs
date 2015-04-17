using DocSymbol;
using System;
using System.Collections.Generic;
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
                var fileName = args[i * 2 + 1];
                Console.WriteLine("Reading " + fileName + " ...");
                var xml = XDocument.Load(fileName);
                var decl = (GlobalDecl)SymbolDecl.Deserialize(xml.Root);
                decl.BuildSymbolTree();
                input.Add(args[i * 2], decl);
            }

            Console.WriteLine("Resolving ...");
            var output = args[args.Length - 1];
        }
    }
}
