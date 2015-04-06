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

        public static void SkipUntil(string[] tokens, ref int index, out string token, params string[] expectedTokens)
        {
            token = null;
            var set = new HashSet<string>(expectedTokens);
            int brackets = 0;
            int squares = 0;
            int braces = 0;

            while (true)
            {
                if (set.Contains(tokens[index]))
                {
                    if (brackets == 0 && squares == 0 && braces == 0)
                    {
                        token = tokens[index];
                        index++;
                        return;
                    }
                }

                switch (tokens[index])
                {
                    case "(":
                        brackets++;
                        break;
                    case ")":
                        if (brackets == 0) throw new ArgumentException("Failed to parse.");
                        brackets--;
                        break;
                    case "[":
                        squares++;
                        break;
                    case "]":
                        if (squares == 0) throw new ArgumentException("Failed to parse.");
                        squares--;
                        break;
                    case "{":
                        braces++;
                        break;
                    case "}":
                        if (braces == 0) throw new ArgumentException("Failed to parse.");
                        braces--;
                        break;
                }
                index++;
            }
        }

        public static void SkipUntil(string[] tokens, ref int index, params string[] expectedTokens)
        {
            string token = null;
            SkipUntil(tokens, ref index, out token, expectedTokens);
        }
    }
}
