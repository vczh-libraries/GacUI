using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    class Program
    {
        static void Main(string[] args)
        {
            var tokens = File.ReadAllLines(args[0]);
            var global = new SymbolDecl();
            {
                int index = 0;
                SymbolDecl.ParseSymbols(tokens, ref index, global);
                if (index != tokens.Length)
                {
                    throw new ArgumentException("Failed to parse.");
                }
            }
        }
    }
}
