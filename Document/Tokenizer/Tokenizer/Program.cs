using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;

namespace Tokenizer
{
    class Program
    {
        const string Operators = "~!#%^&*()-+={[}]|:;<,>.?/";

        static void Main(string[] args)
        {
            var code = File.ReadAllText(args[0]);
            using (var writer = new StreamWriter(args[1], false, Encoding.UTF8))
            {
                int index = 0;
                while (index < code.Length)
                {
                    if (char.IsWhiteSpace(code[index]))
                    {
                        index++;
                    }
                    else if (code[index] == '/')
                    {
                        if (code[index + 1] == '*')
                        {
                            int end = index + 4;
                            while (true)
                            {
                                if (code[end - 2] == '*' && code[end - 1] == '/')
                                {
                                    break;
                                }
                                else
                                {
                                    end++;
                                }
                            }
                            index = end;
                        }
                        else if (code[index + 1] == '/')
                        {
                            int end = index + 1;
                            while (true)
                            {
                                if (code[end] == '\r' || code[end] == '\n')
                                {
                                    break;
                                }
                                else
                                {
                                    end++;
                                }
                            }
                            if (code[index + 2] == '/')
                            {
                                writer.WriteLine(code.Substring(index, end - index));
                            }
                            index = end;
                        }
                        else
                        {
                            writer.WriteLine(code[index]);
                            index++;
                        }
                    }
                    else if (Operators.Contains(code[index]))
                    {
                        writer.WriteLine(code[index]);
                        index++;
                    }
                    else if (code[index] == '\"' || code[index] == '\'')
                    {
                        int end = index + 1;
                        while (true)
                        {
                            if (code[end] == '\\')
                            {
                                end += 2;
                            }
                            else if (code[end] == code[index])
                            {
                                end++;
                                break;
                            }
                            else
                            {
                                end++;
                            }
                        }
                        writer.WriteLine(code.Substring(index, end - index));
                        index = end;
                    }
                    else if ('0' <= code[index] && code[index] <= '9')
                    {
                        int end = index + 1;
                        while (true)
                        {
                            if ('0' <= code[end] && code[end] <= '9' || code[end] == '.' || code[end] == 'U' || code[end] == 'L')
                            {
                                end++;
                            }
                            else
                            {
                                break;
                            }
                        }
                        writer.WriteLine(code.Substring(index, end - index));
                        index = end;
                    }
                    else if ('a' <= code[index] && code[index] <= 'z' || 'A' <= code[index] && code[index] <= 'Z' || code[index] == '_')
                    {
                        int end = index + 1;
                        while (true)
                        {
                            if ('a' <= code[end] && code[end] <= 'z' || 'A' <= code[end] && code[end] <= 'Z' || '0' <= code[end] && code[end] <= '9' || code[end] == '_')
                            {
                                end++;
                            }
                            else
                            {
                                break;
                            }
                        }
                        writer.WriteLine(code.Substring(index, end - index));
                        index = end;
                    }
                    else
                    {
                        throw new ArgumentException("Failed to tokenize.");
                    }
                }
            }
        }
    }
}
