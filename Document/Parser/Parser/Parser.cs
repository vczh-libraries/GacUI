using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Parser
{
    class Parser
    {
        public static bool Token(string[] tokens, ref int index, string expectedToken)
        {
            if (index >= tokens.Length) return false;
            if (tokens[index] != expectedToken) return false;

            index++;
            return true;
        }

        public static bool Id(string[] tokens, ref int index, out string token)
        {
            token = null;
            if (index >= tokens.Length) return false;
            var c = tokens[index][0];
            if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_')
            {
                token = tokens[index++];
                return true;
            }
            return false;
        }
    }
}
