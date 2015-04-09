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

        public static void EnsureToken(string[] tokens, ref int index, params string[] expectedTokens)
        {
            foreach (var token in expectedTokens)
            {
                if (Token(tokens, ref index, token))
                {
                    return;
                }
            }
            throw new ArgumentException("Failed to parse");
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

        public static string EnsureId(string[] tokens, ref int index)
        {
            string id = null;
            if (!Id(tokens, ref index, out id))
            {
                throw new ArgumentException("Failed to parse");
            }
            return id;
        }

        static void SkipUntilInternal(string[] tokens, ref int index, bool inTemplate, out string token, params string[] expectedTokens)
        {
            token = null;
            var set = new HashSet<string>(expectedTokens);
            int brackets = 0;
            int squares = 0;
            int braces = 0;
            int angles = 0;

            while (true)
            {
                if (set.Contains(tokens[index]))
                {
                    if (brackets == 0 && squares == 0 && braces == 0 && angles == 0)
                    {
                        token = tokens[index];
                        index++;
                        return;
                    }
                }

                if (tokens[index].StartsWith("///"))
                {
                    throw new ArgumentException("Failed to parse.");
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
                    case "-":
                        if (index + 1 < tokens.Length && tokens[index + 1] == ">")
                        {
                            index++;
                        }
                        break;
                    case "<":
                        if (inTemplate)
                        {
                            if (index + 1 < tokens.Length && tokens[index + 1] == "<")
                            {
                                index++;
                            }
                            else
                            {
                                angles++;
                            }
                        }
                        break;
                    case ">":
                        if (inTemplate)
                        {
                            if (angles == 0) throw new ArgumentException("Failed to parse.");
                            angles--;
                        }
                        break;
                }
                index++;
            }
        }

        public static void SkipUntil(string[] tokens, ref int index, out string token, params string[] expectedTokens)
        {
            SkipUntilInternal(tokens, ref index, false, out token, expectedTokens);
        }

        public static void SkipUntilInTemplate(string[] tokens, ref int index, out string token, params string[] expectedTokens)
        {
            SkipUntilInternal(tokens, ref index, true, out token, expectedTokens);
        }

        public static void SkipUntil(string[] tokens, ref int index, params string[] expectedTokens)
        {
            string token = null;
            SkipUntil(tokens, ref index, out token, expectedTokens);
        }

        public static void SkipUntilInTemplate(string[] tokens, ref int index, params string[] expectedTokens)
        {
            string token = null;
            SkipUntilInTemplate(tokens, ref index, out token, expectedTokens);
        }
    }
}
